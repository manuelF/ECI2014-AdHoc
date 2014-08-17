/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */

/*
 * This example program allows one to run ns-3 olsr on static nodes
 *
 * By default, the simulation runs for 110 simulated seconds, of which
 * the first 50 are used for start-up time.  The number of nodes is 4.
 * Nodes are placed in specified locations and are static, so their speed
 * is set to 0 m/s and no pause time within a 300x1500 m region.  The WiFi is
 * in ad hoc mode with a 2 Mb/s rate (802.11b) and a Friis loss model.
 * The transmit power is set to 7.5 dBm.
 *
 * It is possible to change the mobility and density of the network by
 * directly modifying the speed, the mobility model, and the number of
 * nodes.  It is also
 * possible to change the characteristics of the network by changing
 * the transmit power (as power increases, the impact of mobility
 * decreases and the effective density increases).
 *
 * By default, there is 1 source/sink data pair sending UDP data
 * at an application rate of 2.048 Kb/s. This is typically done
 * at a rate of 4 64-byte packets per second.  Application data is
 * started at a random time between 50 and 51 seconds and continues
 * to the end of the simulation.
 *
 * The program outputs a few items:
 * - packet receptions are notified to stdout such as:
 *   <timestamp> <dest-address> received one packet from <src-address>
 * - each second, the data reception statistics are tabulated and output
 *   to a comma-separated value (csv) file
 * - some tracing of the mobility of the nodes is written in an output file
 */

#include <fstream>
#include <iostream>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/wifi-module.h"
#include "ns3/ipv4-routing-helper.h"
#include "ns3/aodv-module.h"
#include "ns3/olsr-module.h"
#include "ns3/applications-module.h"

#include <numeric>

using namespace ns3;
//using namespace dsr;

NS_LOG_COMPONENT_DEFINE ("manet-routing-compare");

class RoutingExperiment
{
public:
  RoutingExperiment ();
  void Run (int nSinks, double txp, std::string CSVfileName);
  std::string CommandSetup (int argc, char **argv);

private:
  Ptr<Socket> SetupPacketReceive (Ipv4Address addr, Ptr<Node> node);
  void SendPacket (Ptr<Socket> socket, uint32_t param);
  void ReceivePacket (Ptr<Socket> socket);
  void CheckThroughput ();
  void WriteStatisticsData ();

  uint32_t port;
  uint32_t bytesTotal;
  uint32_t packetsReceived;
  uint32_t m_startTxTime;

  std::string m_CSVfileName;
  std::string m_StatsFileName;
  int m_nSinks;
  std::string m_protocolName;
  double m_txp;
  bool m_traceMobility;
  int m_nodeSpeed;
  int m_nodePause;
  int m_boxSize;
  std::map<uint32_t, uint32_t> m_packetsReceivedByNode;
  std::map<uint32_t, std::set<double> > m_packetsReceivedTime;
};

RoutingExperiment::RoutingExperiment ()
  : port (9),
    bytesTotal (0),
    packetsReceived (0),
    m_startTxTime (50),
    m_CSVfileName ("manet-routing.output.csv"),
    m_StatsFileName("manet-routing.stats.csv"),
    m_traceMobility (false),
    m_nodeSpeed(20),
    m_nodePause(0),
    m_boxSize(800)
{
}

std::string
PrintReceivedPacket (Ptr<Socket> socket, Ptr<Packet> packet)
{
  SocketAddressTag tag;
  bool found;
  std::ostringstream oss;
  Ptr<Ipv4> ipv4 = socket->GetNode()->GetObject<Ipv4>();
  Ipv4InterfaceAddress iaddr = ipv4->GetAddress(1,0);
  Ipv4Address addri=iaddr.GetLocal ();
  oss << Simulator::Now ().GetSeconds ()<< " "<<socket->GetNode ()->GetId ()<<" " << " " << addri;

  found = packet->PeekPacketTag (tag);
  if (found) {
    InetSocketAddress addr = InetSocketAddress::ConvertFrom (tag.GetAddress ());
    oss << " received one packet from " << addr.GetIpv4 ();
  } else {
    oss << " received one packet!";
  }
  return oss.str ();
}

void
RoutingExperiment::SendPacket (Ptr<Socket> socket, uint32_t param)
{
  uint32_t node = socket->GetNode()->GetId();
  std::ostringstream oss;
  oss << node << "envio!" << std::endl;
//  exit(1);
}

void
RoutingExperiment::ReceivePacket (Ptr<Socket> socket)
{
  Ptr<Packet> packet;
  while ((packet = socket->Recv ())) {
    bytesTotal += packet->GetSize ();
    packetsReceived += 1;
    NS_LOG_UNCOND (PrintReceivedPacket (socket, packet));
    uint32_t node = socket->GetNode()->GetId();
    m_packetsReceivedByNode[node] += 1;
    m_packetsReceivedTime[node].insert(Simulator::Now().GetSeconds());
  }
}

void
RoutingExperiment::CheckThroughput ()
{
  double kbs = (bytesTotal * 8.0) / 1000;
  //double percentage = static_cast<double>(packetsReceived)/packetsSent;
  bytesTotal = 0;

  std::ofstream out (m_CSVfileName.c_str (), std::ios::app);

  out << (Simulator::Now ()).GetSeconds () << ","
      << kbs << ","
      << packetsReceived << ","
      << m_nSinks << ","
      << m_protocolName << ","
      << m_txp << ","
      << std::endl;
  // Escribir el porcentajes de paquetes recibidos por algun nodo.

  out.close ();
  packetsReceived = 0;
  Simulator::Schedule (Seconds (1.0), &RoutingExperiment::CheckThroughput, this);
}

Ptr<Socket>
RoutingExperiment::SetupPacketReceive (Ipv4Address addr, Ptr<Node> node)
{
  TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
  Ptr<Socket> sink = Socket::CreateSocket (node, tid);
  InetSocketAddress local = InetSocketAddress (addr, port);
  sink->Bind (local);
  sink->SetRecvCallback (MakeCallback (&RoutingExperiment::ReceivePacket, this));
  sink->SetSendCallback (MakeCallback (&RoutingExperiment::SendPacket, this));

  return sink;
}

std::string
RoutingExperiment::CommandSetup (int argc, char **argv)
{
  CommandLine cmd;
  cmd.AddValue ("CSVfileName", "The name of the CSV output file name", m_CSVfileName);
  cmd.AddValue ("traceMobility", "Enable mobility tracing", m_traceMobility);
  cmd.AddValue ("boundingBoxSide", "Sets the size of one of the sides of the bounding box", m_boxSize);
  cmd.AddValue ("nodeSpeed", "Sets the speed of the nodes using random waypoint", m_nodeSpeed);
  cmd.AddValue ("nodePause", "Sets the pause of the nodes using random waypoint", m_nodePause);
  cmd.Parse (argc, argv);
  return m_CSVfileName;
}

int
main (int argc, char *argv[])
{
  RoutingExperiment experiment;
  std::string CSVfileName = experiment.CommandSetup (argc,argv);

  //blank out the last output file and write the column headers
  std::ofstream out (CSVfileName.c_str ());
  out << "SimulationSecond," <<
      "ReceiveRate," <<
      "PacketsReceived," <<
      "NumberOfSinks," <<
      "RoutingProtocol," <<
      "TransmissionPower" <<
      std::endl;

  out.close ();

  int nSinks = 10;
  double txp = 7.5;

  experiment.Run (nSinks, txp, CSVfileName);
}

uint32_t
mapAccumulator(uint32_t res, std::pair<uint32_t,uint32_t> pair)
{
    return res + pair.second;
}

void
RoutingExperiment::WriteStatisticsData()
{
    std::ofstream out (m_StatsFileName.c_str());
    out << "NodeId,"
        << "MessagesReceived,"
        << "Percentage"
        << std::endl;
    uint32_t totalPacketsReceived =
        std::accumulate(
                m_packetsReceivedByNode.begin(),
                m_packetsReceivedByNode.end(),
                0, mapAccumulator);
    for (std::map<uint32_t,uint32_t>::iterator it = m_packetsReceivedByNode.begin();
            it != m_packetsReceivedByNode.end();
            ++it)
    {
        out << it->first
            << ", "
            << it->second
            << ", "
            << 100.0*static_cast<double>(it->second) /
                     static_cast<double>(totalPacketsReceived) << "%"
            << std::endl;
    }
    out.close();
}

void
RoutingExperiment::Run (int nSinks, double txp, std::string CSVfileName)
{
  Packet::EnablePrinting ();
  m_nSinks = nSinks;
  m_txp = txp;
  m_CSVfileName = CSVfileName;

  int nWifis = 50; // the number of devices forming the network

  double TotalTime = 110.0;
  double TransmittedBytes = (2048/8)/64 *(TotalTime - m_startTxTime);
  std::cerr << TransmittedBytes << std::endl;
  std::string rate ("2048bps");
  std::string phyMode ("DsssRate11Mbps");
  std::string tr_name ("manet-routing-compare");
  int nodeSpeed = m_nodeSpeed; //in m/s
  int nodePause = m_nodePause; //in s
  m_protocolName = "protocol";

  Config::SetDefault  ("ns3::OnOffApplication::PacketSize",StringValue ("64"));
  Config::SetDefault ("ns3::OnOffApplication::DataRate",  StringValue (rate));

  //Set Non-unicastMode rate to unicast mode
  Config::SetDefault ("ns3::WifiRemoteStationManager::NonUnicastMode",StringValue (phyMode));

  NodeContainer adhocNodes;
  adhocNodes.Create (nWifis);

  // setting up wifi phy and channel using helpers
  WifiHelper wifi;
  wifi.SetStandard (WIFI_PHY_STANDARD_80211b);

  YansWifiPhyHelper wifiPhy =  YansWifiPhyHelper::Default ();
  YansWifiChannelHelper wifiChannel;
  wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");

  // Cambiado el modelo de propagacion a LogDistance
  //wifiChannel.AddPropagationLoss ("ns3::FriisPropagationLossModel");
  wifiChannel.AddPropagationLoss ("ns3::LogDistancePropagationLossModel");
  wifiPhy.SetChannel (wifiChannel.Create ());

  // Add a non-QoS upper mac, and disable rate control
  NqosWifiMacHelper wifiMac = NqosWifiMacHelper::Default ();
  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                "DataMode",StringValue (phyMode),
                                "ControlMode",StringValue (phyMode));

  wifiPhy.Set ("TxPowerStart",DoubleValue (txp));
  wifiPhy.Set ("TxPowerEnd", DoubleValue (txp));

  wifiMac.SetType ("ns3::AdhocWifiMac");
  NetDeviceContainer adhocDevices = wifi.Install (wifiPhy, wifiMac, adhocNodes);

  std::stringstream ssBoxSize;
//  ssBoxSize << "ns3::ConstantRandomVariable[Constant=" << m_boxSize << "]";
  ssBoxSize << "ns3::UniformRandomVariable[Min=0.0|Max=" << m_boxSize << "]";
  ObjectFactory pos;
  pos.SetTypeId ("ns3::RandomRectanglePositionAllocator");
  pos.Set ("X", StringValue(ssBoxSize.str()));
  pos.Set ("Y", StringValue(ssBoxSize.str()));

  std::stringstream ssSpeed;
  std::stringstream ssPause;
  ssSpeed << "ns3::ConstantRandomVariable[Constant=" << nodeSpeed << "]";
  ssPause << "ns3::ConstantRandomVariable[Constant=" << nodePause << "]";

  MobilityHelper mobilityAdhoc;

  Ptr<PositionAllocator> taAlloc = pos.Create()-> GetObject<PositionAllocator> ();
  mobilityAdhoc.SetMobilityModel ("ns3::RandomWaypointMobilityModel",
                                  "Pause", StringValue (ssPause.str()),
                                  "Speed", StringValue (ssSpeed.str()),
                                  "PositionAllocator", PointerValue(taAlloc));
  mobilityAdhoc.SetPositionAllocator(taAlloc);
  mobilityAdhoc.Install (adhocNodes);
  // Print the location of the nodes in standard output
//   for (NodeContainer::Iterator i = adhocNodes.Begin (); i != adhocNodes.End (); i++)
//   {
//
//       Ptr <Node> node = *i;
//  Ptr <MobilityModel> mob = node->GetObject<MobilityModel> ();
//  Vector pos = mob->GetPosition ();
//  std::cout<<node->GetId ()<<" "<< pos.x<<" "<< pos.y<<" "<<std::endl;
//    }

  // Install olsr in the nodes
  OlsrHelper olsr;
  Ipv4ListRoutingHelper list;
  InternetStackHelper internet;

  list.Add (olsr, 100);
  m_protocolName = "OLSR";
  internet.SetRoutingHelper (list);
  internet.Install (adhocNodes);

  NS_LOG_INFO ("assigning ip address");

  // Assign IP addresses to the nodes
  Ipv4AddressHelper addressAdhoc;
  addressAdhoc.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer adhocInterfaces;
  adhocInterfaces = addressAdhoc.Assign (adhocDevices);

  // Set the data flow configuration
  OnOffHelper onoff1 ("ns3::UdpSocketFactory",Address ());
  onoff1.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1.0]"));
  onoff1.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0.0]"));

  for (int i = 0; i <= nSinks - 1; i++) {
    Ptr<Socket> sink = SetupPacketReceive (adhocInterfaces.GetAddress (i), adhocNodes.Get (i));

    AddressValue remoteAddress (InetSocketAddress (adhocInterfaces.GetAddress (i), port));
    onoff1.SetAttribute ("Remote", remoteAddress);

    Ptr<UniformRandomVariable> var = CreateObject<UniformRandomVariable> ();
    ApplicationContainer temp = onoff1.Install (adhocNodes.Get (i + nSinks));
    temp.Start (Seconds (var->GetValue (m_startTxTime,m_startTxTime+1)));
    temp.Stop (Seconds (TotalTime));
  }

  AsciiTraceHelper ascii;
  Ptr<OutputStreamWrapper> osw = ascii.CreateFileStream ( (tr_name + ".tr").c_str());
  MobilityHelper::EnableAsciiAll (osw);

  NS_LOG_INFO ("Run Simulation.");

  CheckThroughput ();
  Simulator::Stop (Seconds (TotalTime));
  Simulator::Run ();

  WriteStatisticsData();
  Simulator::Destroy ();
}

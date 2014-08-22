all:
	pandoc -o informe.pdf informe.md

clean:
	rm -f informe.pdf

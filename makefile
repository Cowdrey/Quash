all: quash

quash: main.c
	g++ main.c -g -o quash
	g++ testProg.c -g -o testProg

clean:
	rm -f quash testProg *~


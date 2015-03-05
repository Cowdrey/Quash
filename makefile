all: quash

quash: main.c
	g++ main.c -g -o quash

clean:
	rm -f quash *~


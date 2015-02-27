all: quash

quash: main.c
	g++ main.c -g -o quash

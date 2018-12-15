CFLAGS=-O

all: ncut.o
	cc -o ncut ncut.o

clean: 
	rm ncut ncut.o

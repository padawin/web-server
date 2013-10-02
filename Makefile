INCDIRS =       -I/usr/local/include
CC =            gcc
CFLAGS =        $(INCDIRS) -Wall -g -O2
LIBS =          -levent

all: server

server: server.o
	$(CC) $(CFLAGS) server.o -g -O2 -o $@ $(LIBS)

%.o : %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f *.o server

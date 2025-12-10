# Makefile

CC = gcc
CFLAGS = -Wall -Wextra -pedantic -std=c11

.PHONY: all clean

all: tcpclient tcpserver udpclient udpserver

tcpclient: tcpclient.c
	$(CC) $(CFLAGS) tcpclient.c -o tcpclient

tcpserver: tcpserver.c
	$(CC) $(CFLAGS) tcpserver.c -o tcpserver

udpclient: udpclient.c
	$(CC) $(CFLAGS) udpclient.c -o udpclient

udpserver: udpserver.c
	$(CC) $(CFLAGS) udpserver.c -o udpserver

clean:
	rm -f tcpclient tcpserver udpclient udpserver
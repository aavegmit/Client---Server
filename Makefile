# Explicit rule
all: server client

server: server.o
	g++ -g -Wall -o server server.o 

client: client.o
	g++ -g -Wall -o client client.o

clean:
	rm *.o client server

server.o: server.cc
client.o: client.cc

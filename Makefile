# Explicit rule
all: server client

server: server.o server_operations.o
	g++ -g -Wall -o server server.o server_operations.o

client: client.o client_operations.o
	g++ -g -Wall -o client client.o client_operations.o

clean:
	rm *.o client server

server.o: server.cc
client.o: client.cc
server_operations.o: server_operations.cc
client_operations.o: client_operations.cc

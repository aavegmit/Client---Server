# Explicit rule
all: server client

server: server.o server_operations.o shared.o
	g++ -g -Wall -o server server.o server_operations.o shared.o

client: client.o client_operations.o shared.o
	g++ -g -Wall -o client client.o client_operations.o shared.o -lcrypto

clean:
	rm *.o client server

server.o: server.cc
client.o: client.cc
server_operations.o: server_operations.cc server_operations.h
client_operations.o: client_operations.cc client_operations.h
shared.o: shared.cc shared.h

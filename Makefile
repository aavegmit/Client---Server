# Macros

CC = g++
CFLAGS = -g -Wall
OBJ = server_operations.o shared.o server.o 
OBJ1 = client_operations.o shared.o client.o
LIBS = -lcrypto
#LIBS = -L/home.scf-22/csci551b/openssl/lib -lcrypto -lnsl -lsocket -lresolv
#INC = -I/home/scf-22/csci551b/openssl/include
INC = 

# Explicit rule
all: server client

server: $(OBJ)
	$(CC) $(CFLAGS) -o server $(OBJ) $(INC) $(LIBS) 

client: $(OBJ1)
	$(CC) $(CFLAGS) -o client $(OBJ1) $(INC)  $(LIBS) 

clean:
	rm -rf *.o client server

server.o: server.cc
	$(CC) $(CFLAGS) -c server.cc $(INC)
client.o: client.cc
	$(CC) $(CFLAGS) -c client.cc $(INC)
shared.o: shared.cc shared.h
	$(CC) $(CFLAGS) -c shared.cc $(INC)
server_operations.o: server_operations.cc server_operations.h
	$(CC) $(CFLAGS) -c server_operations.cc $(INC)
client_operations.o: client_operations.cc client_operations.h
	$(CC) $(CFLAGS) -c client_operations.cc $(INC)

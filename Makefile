# Macros

CC = g++
CFLAGS = -g -Wall
SRC = main.cc hexdump.cc enc-base64.cc dec-base64.cc
OBJ = main.o hexdump.o enc-base64.o dec-base64.o

# Explicit rule

hw1: $(OBJ)
	$(CC) $(CFLAGS) -o hw1 $(OBJ) 

debug: $(OBJ)
	$(CC) $(CFLAGS) -DDEBUG=1 -o hw1 $(OBJ) 


hexdump.o: hexdump.cc hexdump.h
dec-base64.o: dec-base64.cc dec-base64.h
enc-base64.o: enc-base64.cc enc-base64.h
main.o: main.cc

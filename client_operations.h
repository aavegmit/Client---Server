#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define HEADER_SIZE 11

extern void fsz_request(int sockfd, char *fileName, uint8_t delay) ;
extern void response_handler(int nSocket, char *reqString, struct sockaddr_in serv_addr) ;


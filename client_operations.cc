#include <iostream>
#include <stdio.h>
#include <string.h>
#include "client_operations.h"

/*
   void fsz_request(int sockfd, char *fileName, uint8_t delay ){


   SendAcrossNetwork(sockfd,0xfe20 , fileName, delay, 0);

   response_handler(sockfd) ;

   }
 */


/* 
 * Receives response from the server 
 */
void response_handler(int nSocket ){

	unsigned char header[HEADER_SIZE];
	unsigned char *buffer ;
	int return_code = 0 ;

	memset(header, 0, HEADER_SIZE) ;

	for (int i=0; i < HEADER_SIZE; i++) {
		return_code=(int)read(nSocket, &header[i], 1);
		if (return_code == -1){
			printf("Socket Read error...\n") ;
			exit(0) ;
		}
		printf("Reading %02x\n", header[i], return_code) ;
	}

	uint16_t message_type=0;
	uint32_t offset=0;
	uint32_t data_length=0;
	uint8_t delay=0 ;

	memcpy(&message_type, header, 2);
	memcpy(&offset,       header+2, 4);
	memcpy(&delay,       header+6, 1);
	memcpy(&data_length,  header+7, 4);

	message_type = ntohs(message_type);
	offset       = ntohl(offset);
	data_length  = ntohl(data_length);

	printf("In client response handler...\n") ;	

	/* allocate buffer to read data_length number of bytes */
	buffer = (unsigned char *)malloc(data_length) ;
	memset(buffer, 0 ,data_length) ;

	// If malloc fails, the datalength could be very big!!
	if (buffer == NULL){
		printf("Malloc failed. Might be because of big filename.\n");
		exit(0) ;
	}
	for (int i=0; i < data_length; i++) {
		return_code=(int)read(nSocket, &buffer[i], 1);
		if (return_code == -1){
			printf("Socket Read error...\n") ;
			exit(0) ;
		}

	}
	switch (message_type) {
		case 0xfe22:
			printf("No such file found..\n") ;
			break ;
		case 0xfe21:
			printf("Message: %02x %04x %d %04x %s\n", message_type, offset,delay, data_length, buffer) ;
			break;
		case 0xfe11:
			printf("IP ADDRESS: %02x %04x %d %04x %s\n", message_type, offset,delay, data_length, buffer) ;
			break ;
		case 0xfe12:
			printf("No such host exists..\n") ;
			break ;
		
	}
}

#include <iostream>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <openssl/md5.h>
#include "client_operations.h"
#include "shared.h"

/*
   void fsz_request(int sockfd, char *fileName, uint8_t delay ){


   SendAcrossNetwork(sockfd,0xfe20 , fileName, delay, 0);

   response_handler(sockfd) ;

   }
 */


/* 
 * Receives response from the server 
 */
void response_handler(int nSocket, char *reqString, struct sockaddr_in serv_addr ){

	unsigned char header[HEADER_SIZE];
	unsigned char *buffer ;
	int return_code = 0 ;

	memset(header, 0, HEADER_SIZE) ;

	// Declare variables for timeout
	int readsocks = 0;
	fd_set socks ;
	struct timeval timeout ;
	timeout.tv_sec = 50 ;
	timeout.tv_usec = 0 ;

	// Clear out the fd
	FD_ZERO(&socks) ;
	// Add current socket descriptor in this list
	FD_SET(nSocket, &socks) ;

	uint16_t message_type=0;
	uint32_t offset=0;
	uint32_t data_length=0;
	uint8_t delay=0 ;

	for (int i=0; i < HEADER_SIZE; i++) {
		readsocks = select(nSocket + 1, &socks, (fd_set *)0, (fd_set *)0, &timeout) ;
		if (readsocks < 0){
			perror("select") ;
			return ;
		}
		else if(readsocks == 0){
			printf("Timeout expired..\n") ;
			display(message_type, offset, delay, data_length, inet_ntoa(serv_addr.sin_addr), i) ;
			return ;
		}
		else{
			return_code=(int)read(nSocket, &header[i], 1);
			if (return_code == 0){
				display(message_type, offset, delay, data_length, inet_ntoa(serv_addr.sin_addr), i) ;
				printf("Socket Read error...\n") ;
				return ;
			}
			if (i==1)
				memcpy(&message_type, header, 2);
			if (i==5)
				memcpy(&offset,       header+2, 4);
			if (i==6)
				memcpy(&delay,       header+6, 1);
			if (i==10)
				memcpy(&data_length,  header+7, 4);

		}	//		printf("Reading %02x, return code %d\n", header[i], return_code) ;

	}


	memcpy(&message_type, header, 2);
	memcpy(&offset,       header+2, 4);
	memcpy(&delay,       header+6, 1);
	memcpy(&data_length,  header+7, 4);

	message_type = ntohs(message_type);
	offset       = ntohl(offset);
	data_length  = ntohl(data_length);

	//	printf("In client response handler...\n") ;	
	display(message_type, offset, delay, data_length, inet_ntoa(serv_addr.sin_addr), HEADER_SIZE) ;

	switch (message_type) {
		// Case when FSZ request fails
		case 0xfe22:
			printf("\tFILESIZE request for '%s' failed.\n", reqString) ;
			break ;
		case 0xfe21:
			if (data_length == 0 || data_length > 512){
				printf("\tInvalid server response\n");
				char ch ;
				for (unsigned int i=0; i < data_length; i++) {
					return_code=(int)read(nSocket, &ch, 1);
					if (return_code == 0){
						printf("Socket Read error...\n") ;
						exit(0) ;
					}

				}
				return ;
			}
			/* allocate buffer to read data_length number of bytes */
			buffer = (unsigned char *)malloc(data_length + 1) ;
			memset(buffer, 0 ,data_length + 1) ;

			// If malloc fails, the datalength could be very big!!
			if (buffer == NULL){
				printf("Malloc failed. Might be because of big filename.\n");
				return ;
			}
			for (unsigned int i=0; i < data_length; i++) {
				return_code=(int)read(nSocket, &buffer[i], 1);
				if (return_code == 0){
					printf("Socket Read error...\n") ;
					return ;
				}

			}
			buffer[data_length] = '\0' ;
			printf("\tFILESIZE = %s\n",  buffer) ;
			free(buffer) ;
			break;
		case 0xfe11:
			if (data_length == 0 || data_length > 512){
				char ch ;
				for (unsigned int i=0; i < data_length; i++) {
					return_code=(int)read(nSocket, &ch, 1);
					if (return_code == 0){
						printf("Socket Read error...\n") ;
						return ;
					}

				}
				printf("\tInvalid server response\n");
				return ;
			}
			/* allocate buffer to read data_length number of bytes */
			buffer = (unsigned char *)malloc(data_length + 1) ;
			memset(buffer, 0 ,data_length + 1) ;

			// If malloc fails, the datalength could be very big!!
			if (buffer == NULL){
				printf("Malloc failed. Might be because of IP address information.\n");
				return ;
			}
			for (unsigned int i=0; i < data_length; i++) {
				return_code=(int)read(nSocket, &buffer[i], 1);
				if (return_code == 0){
					printf("Socket Read error...\n") ;
					return ;
				}

			}
			buffer[data_length] = '\0' ;
			printf("\tADDR = %s\n", buffer) ;
			free(buffer) ;
			break ;
		case 0xfe12:
			printf("\tADDR request for '%s' failed.\n", reqString) ;
			break ;
		case 0xfe32:
			printf("\tGET request for '%s' failed.\n", reqString) ;
			break ;
		case 0xfcfe:
			printf("ALL-FAILURE response received\n") ;
			break ;
		case 0xfe31:
			char *getBuf = (char *)malloc(512) ;
			memset(getBuf,0,512) ;

			MD5_CTX ctx ;
			unsigned char *md = (unsigned char *)malloc(16) ;
			memset(md,0,16) ;
			if (!MD5_Init(&ctx)){
				printf("MD5 Init failed\n") ;
				return ;
			}
			int cnt = 0 ;
			for (unsigned int i=0; i < data_length; i++) {
				return_code=(int)read(nSocket, &getBuf[cnt], 1);
				if (return_code == 0){
					printf("Socket Read error...\n") ;
					return ;
				}
				++cnt ;


				// Use getBuf to update the MD5
				if (cnt == 512 ){
					MD5_Update(&ctx, getBuf, 512) ;
					// Clear out the buf
					memset(getBuf,0,512) ;
					cnt = 0 ;
				}


			}
			if (!MD5_Update(&ctx, getBuf, cnt)) {
				printf("MD5 update failed\n") ;
				return ;
			}
			if (!MD5_Final(md, &ctx)){
				printf("MD_Final failed\n");
				return ;
			}
			printf("\tFILESIZE = %d",data_length) ;
			if (data_length > 0){
				printf(", MD5: ") ;
				for (int j=0 ;j < 16; j++)
					printf("%02x", md[j]) ;
			}
			printf("\n") ;

			free(getBuf) ;
			break ;

	}
}

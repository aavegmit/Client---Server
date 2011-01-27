#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include "server_operations.h"
#include "shared.h"

/* Handle FSZ REQUEST
 * Use stat() to find the file size
 * Create a response packet and reply back to the client
 */
void handle_fszReq(int nSocket, unsigned char *buffer){
	struct stat fileStatus ;
	int ret_code = stat((const char *)buffer, &fileStatus) ;

	char *responseStr ;
	if (ret_code == -1){
		SendAcrossNetwork(nSocket, 0xfe22, NULL ,0,0) ;
	}
	else {
		// To find the length of size
		int tempS = fileStatus.st_size ;
		int count = 0 ;
		while(tempS != 0){
			tempS = tempS / 10 ;
			++count ;
		}

		responseStr = (char *)malloc(count) ;
		sprintf(responseStr, "%d", (int)fileStatus.st_size) ;
		SendAcrossNetwork(nSocket, 0xfe21, responseStr, 0,0) ;
		//		printf("File size %s, len %d\n", responseStr, count) ;
	}
	//free(responseStr) ;

}

/* Handle ADDR REQUEST
 * Use gethostname() to find the IP address of the host
 * Create a response packet and reply back to the client
 */
void handle_addrReq(int nSocket, unsigned char *buffer){
	struct hostent *hostIP = gethostbyname((char *)buffer) ;

	if (hostIP == NULL){
		SendAcrossNetwork(nSocket,0xfe12, NULL, 0,0) ;
	}
	else{
		char *hostname = new char[16] ;
		memset(hostname, '\0', 16) ;
		sprintf(hostname, "%s", inet_ntoa(*(struct in_addr*)(hostIP->h_addr_list[0]))) ;
		printf("Hostip %s--\n", hostname) ;

		SendAcrossNetwork(nSocket,0xfe11, hostname, 0,0) ;



		delete [] hostname ;
	}

}


/* Handle GET REQUEST
 * Use gethostname() to find the IP address of the host
 * Create a response packet and reply back to the client
 */
void handle_getReq(int sockfd, unsigned char *buffer, uint32_t offset, uint8_t delay){
	//	printf("In get request handler\n") ;
	FILE *fp ;

	// Open the file
	if ( (fp = fopen((char *)buffer, "rb"))==NULL){
		printf("File could not be open\n");
		SendAcrossNetwork(sockfd,0xfe32, NULL, offset,delay) ;
		return ;
	}
	myFile.push_back(fp) ;

	struct stat fileStatus ;
	int ret_code = stat((char *)buffer, &fileStatus) ;

	if (ret_code == -1){
		printf("Stat() on file  failed.\n");
		SendAcrossNetwork(sockfd,0xfe32, NULL, offset,delay) ;
		return ;
	}

	// Checking if the string passed is a regular file.
	// i.e. no directory, device, pipes are accepted
	if (!S_ISREG(fileStatus.st_mode)){
		printf("Not a regular file\n");
		SendAcrossNetwork(sockfd,0xfe32, NULL, offset,delay) ;
		return ;
	}

	char *bufe = (char *)malloc(512) ;
	memset(bufe,0,512) ;

	myMem.push_back((unsigned char*)bufe) ;

	// find the data length
	if (fseek(fp, 0, SEEK_END)) {
		printf("Fseek failed\n");
		SendAcrossNetwork(sockfd,0xfe32, NULL, offset,delay) ;
		//free(bufe) ;
		return ;
		//	exit(0) ;
	}
	uint32_t dlength = 0 ;
	dlength = ftell(fp) - offset ;


	// Check if the offset passed is equal or more than the file size
	if (offset >= (unsigned int)ftell(fp)){
		printf("Ftell failed\n");
		SendAcrossNetwork(sockfd,0xfe32, NULL, offset,delay) ;
		//free(bufe) ;
		return ;
	}

	// move the file pointer at the offset
	if (fseek(fp, (int)offset, SEEK_SET)){
		printf("Fseek failed\n");
		SendAcrossNetwork(sockfd,0xfe32, NULL, offset,delay) ;
		//free(bufe) ;
		return ;
		//		exit(0) ;
	}
	int count = 0 ;

	//	int buf_sz = HEADER_SIZE + dlength ;

	unsigned char *buf=(unsigned char *)malloc(HEADER_SIZE);
	memset(buf, 0, HEADER_SIZE) ;

	myMem.push_back(buf) ;

	if (buf == NULL) { 
		printf("Malloc failed. Check your str\n") ;
		//free(bufe) ;
		//free(buf) ;
		return ;
	}

	/* Fill out the 11-byte common header */

	/* MessageType is 0xfe20 */
	uint16_t type = htons(0xfe31) ;
	memcpy(&buf[0], &type, 2) ;

	/* Offset is 0 */
	offset = htonl(offset) ;
	memcpy(&buf[2], &offset, 4) ;

	/* delay */
	memcpy(&buf[6], &delay, 1) ;

	/* DataLength is strlen("Makefile")=8 */
	dlength = htonl(dlength) ;
	memcpy(&buf[7], &dlength, 4) ;

	// send the header
	//	printf("-----------------------\n") ;
	for (int i=0; i < HEADER_SIZE ; i++) {
		if(shutDown){
			printf("@child: Time to move out...\n") ;
			//free(bufe) ;
			//free(buf) ;
			return ;
		}
		int return_code=(int)write(sockfd, &buf[i], 1);
		if (return_code == -1){
			printf("Socket write error..\n");
			//free(bufe) ;
			//free(buf) ;
			return;
		}
		//		printf("%02x ", buf[i]) ;

	}
	//	printf("\n------------*----------\n") ;

	int bytes_read = 0 ;
	while(!feof(fp)){
		bytes_read = fread(&bufe[count],1,1,fp ) ;

		if (bytes_read){
			++count ;
			// Time to clear out the buffer and write to the socket
			if (count == 512){
				for (int i=0; i < 512 ; i++) {
					if(shutDown){
						printf("@child: Time to move out...\n") ;
						//free(bufe) ;
						//free(buf) ;
						return ;
					}
					int return_code=(int)write(sockfd, &bufe[i], 1);
					if (return_code == -1){
						printf("Socket write error..\n");
						//free(bufe) ;
						//free(buf) ;
						return;
					}

				}

				memset(bufe,0,512) ;
				count = 0 ;
			}
		}
		else if (!feof(fp)){
			printf("error %d %d\n", errno, bytes_read) ;
			printf("Fread failed\n");
			//			SendAcrossNetwork(sockfd,0xfe32, NULL, offset,delay) ;
			//free(bufe) ;
			//free(buf) ;
			return ;
		}

	}
	// write the rest of the content
	for (int i=0; i < count ; i++) {
		if(shutDown){
			printf("@child: Time to move out...\n") ;
			//free(bufe) ;
			//free(buf) ;
			return ;
		}
		int return_code=(int)write(sockfd, &bufe[i], 1);
		if (return_code == -1){
			printf("Socket write error..\n");
			//free(bufe) ;
			//free(buf) ;
			return ;
		}
		//		printf("%02x ", bufe[i]) ;

	}

	//free(bufe) ;
	//free(buf) ;

}

void server_processing( int nSocket, struct sockaddr_in cli_addr ){


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
		//		printf("Reading %02x, return code %d\n", header[i], return_code) ;
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

	//	printf("In server handler...\n") ;	

	/* allocate buffer to read data_length number of bytes */
	buffer = (unsigned char *)malloc(data_length + 1) ;
	memset(buffer, 0 ,data_length + 1) ;

	myMem.push_back(buffer) ;

	// If malloc fails, the datalength could be very big!!
	if (buffer == NULL){
		printf("Malloc failed. Might be because of big filename.\n");
		exit(0) ;
	}
	for (unsigned int i=0; i < data_length; i++) {
		return_code=(int)read(nSocket, &buffer[i], 1);
		if (return_code == -1){
			printf("Socket Read error...\n") ;
			exit(0) ;
		}

	}
	buffer[data_length] = '\0' ;
	display(message_type, offset, delay, data_length, inet_ntoa(cli_addr.sin_addr)) ;

	sleep(delay) ;
	switch (message_type) {
		case 0xfe20:
			handle_fszReq(nSocket, buffer) ;
			break;
		case 0xfe10:
			handle_addrReq(nSocket, buffer) ;
			break ;
		case 0xfe30:
			handle_getReq(nSocket, buffer, offset, delay) ;
			break ;
		default:
			SendAcrossNetwork(nSocket, 0xfcfe, NULL ,0,0) ;
	}

	//free(buffer) ;

}

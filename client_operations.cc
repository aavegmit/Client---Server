#include <iostream>
#include <stdio.h>
#include <string.h>
#include "client_operations.h"

void fsz_request(int sockfd, char *fileName, uint8_t delay ){
	int buf_sz = HEADER_SIZE + strlen(fileName) ;

	unsigned char *buf=(unsigned char *)malloc(buf_sz);
	memset(buf, 0, buf_sz) ;

	if (buf == NULL) { 
		printf("Malloc failed. Check your fileName\n") ;
		exit(0) ;
	}

	/* Fill out the 11-byte common header */

	/* MessageType is 0xfe20 */
	buf[0] = 0xfe;
	buf[1] = 0x20;

	/* Offset is 0 */
	buf[2] = 0x00;
	buf[3] = 0x00;
	buf[4] = 0x00;
	buf[5] = 0x00;


	/* delay */
	printf("DELAY 8 bit %02x\n", delay) ;
//	delay = htons(delay) ;
	memcpy(&buf[6], &delay, 1) ;
//	buf[6] = 0x00;

	/* DataLength is strlen("Makefile")=8 */
	uint32_t dlength = strlen(fileName) ;
	dlength = htonl(dlength) ;
	memcpy(&buf[7], &dlength, 4) ;

	/* Fill out the data field */
	strncpy((char *)&buf[HEADER_SIZE], fileName , strlen(fileName));

	for (int i=0; i < buf_sz ; i++) {
		int return_code=(int)write(sockfd, &buf[i], 1);
		if (return_code == -1){
			printf("Socket write error..\n");
			exit(0);
		}
		printf("Writing %02x\n", buf[i]) ;

	}
	/*
	 * Now the client should read from the socket to get the reply.
	 * But for now, we will put the client in an infinite loop.
	 *     If you want to kill the client, you can press <Cntrl+C>.
	 */
	//	for (;;) {
	//		/* call sleep() so we don't busy-wait */
	//		sleep(1);
	//	}
}


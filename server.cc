#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "server_operations.h"

void usage(){
	printf("Usage:\t ./server [-t seconds] [-m] port\n") ;
	exit(0) ;
}

int main(int argc, char *argv[])
{
	struct sockaddr_in serv_addr;
	int nSocket=0, portGiven = 0 , portNum = 0;

	// Parsing the command line
	if (argc < 2){
		usage() ;
	}
	else {
		argv++ ;

		for (int i = 0 ; i < argc-1 ; i++, argv++) {
			if (*argv[0] == '-') {
				if (strcmp(*argv, "-m") == 0) {
					/* set a global flag */
					printf("option m selected\n") ;
				} else if (strcmp(*argv, "-t") == 0) {
					argc--, argv++; /* move past "-t"*/
					if (argc <= 0) {
						usage() ;
					}
					/* read offset from *argv */
					if (!atoi(*argv)){
						printf("Bad Timeout argument\n") ;
						exit(0) ;
					}
					printf("Seconds: %d\n", atoi(*argv)) ;
				}
				else
					usage() ;
			} 
			else {
				if (portGiven)
					usage() ;
				if (!atoi(*argv)){
					printf("Bad Port Number\n");
					exit(0);
				}
				printf("Port: %s\n", *argv) ;
				portNum = atoi(*argv) ;
				portGiven = 1 ;
			}
		}
	}

	if (!portGiven)
		usage() ;


	printf("Command line parsing done\n") ;

	//creating a socket
	nSocket = socket(AF_INET, SOCK_STREAM, 0) ;

	// Initialising the structure with 0
	memset(&serv_addr,0,sizeof(struct sockaddr_in)) ;

	// Filling up the specifics
	serv_addr.sin_family = AF_INET ;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY) ;
	serv_addr.sin_port = htons(portNum) ;

	// Bind the socket 
	bind(nSocket, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

	// listen on this socket
	listen(nSocket, 5) ;

	for(;;){
		int cli_len = 0, newsockfd = 0;
		struct sockaddr_in cli_addr ;

		// Wait for clients to connect
		newsockfd = accept(nSocket, (struct sockaddr *)&cli_addr, (socklen_t *)&cli_len ) ;

		if (newsockfd < 0){
			printf("Error in accept\n") ;
		} 
		else {
			int pid = fork() ;

			if (pid < 0){
				/* error */
			}
			else if (pid == 0) {
				close(nSocket) ;
				printf("Connection with client established\n") ;
				server_processing( newsockfd ) ;
				exit(0) ;
			}
			close(newsockfd) ;
		}

	}
	close(nSocket) ;
	exit(0) ;

}

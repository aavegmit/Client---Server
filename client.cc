#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "client_operations.h"
#include "shared.h"
#include <ctype.h>

int optionM = 0 ;

void usage(){
	printf("Usage:\t ./client {adr|fsz|get} [-d delay] [-o offset] [-m] hostname:port string\n") ;
	exit(0) ;
}

void server_terminated(int sig){
	printf("Server terminated\n");
}

int main(int argc, char *argv[]){
	struct sockaddr_in serv_addr ;
	struct hostent *host ;
	int nSocket = 0, status, choice = 0, fixedArg = 0; 
	uint8_t delay=0 ;
	uint32_t offset = 0 ;

	char *port ;
	char *hostname, *stringArg ;

	optionM = 0 ;
	
	// Setting the signals
	struct sigaction sact ;
	sigemptyset(&sact.sa_mask) ;
	sact.sa_flags = 0 ;
	sact.sa_handler = server_terminated ;
	sigaction(SIGPIPE, &sact, NULL) ;

	// Parse the command line 
	if (argc < 3){
		usage() ;
	}
	else {
		argv++ ;
		if (strcmp(*argv, "adr") == 0) {
			choice = 1 ;
		} else if (strcmp(*argv, "fsz") == 0) {
			choice = 2 ;
		} else if (strcmp(*argv, "get") == 0) {
			choice = 3 ;
		} else {
			usage() ;
		}

		argv++ ;

		for (int i = 0 ; i < argc-2 ; i++, argv++) {
			if (*argv[0] == '-') {
				if (strcmp(*argv, "-m") == 0) {
					/* set a global flag */
					optionM = 1 ;
	//				printf("option m selected\n") ;
				} 
				else if (strcmp(*argv, "-d") == 0) {
					argc--, argv++; /* move past "-d"*/
					if (argc <= 0) {
						usage() ;
					}
					/* read delay from *argv */
					if (!isdigit(*argv[0]) || atoi(*argv) < 0){
						printf("Bad delay argument\n") ;
						usage() ;
					}
					delay = atoi(*argv) ;
	//				printf("delay: %d\n", delay) ;
				}
				else if (strcmp(*argv, "-o") == 0) {
					argc--, argv++; /* move past "-o"*/
					if (argc <= 0) {
						usage() ;
					}
					/* read offset from *argv */
					if (!isdigit(*argv[0]) || atoi(*argv) < 0 ){
						printf("Bad offset argument\n") ;
						usage() ;
					}
					offset = atoi(*argv) ;
	//				printf("offset: %d\n", offset) ;
				}
				else
					usage() ;
			} 
			else {
				if (fixedArg == 0){
					++fixedArg ;
					hostname = strtok(*argv, ":") ;
					if (hostname ==  NULL){
						printf("Bad hostname and portnumber\n") ;
						usage() ;
					}
					port = strtok(NULL, ":") ;
					if (port ==  NULL){
						printf("Bad hostname and portnumber\n") ;
						usage() ;
					}
					
					printf("%s %s\n", hostname, port) ;
					if (   atoi(port) <= 0 ){
						printf("Bad port number\n");
						exit(0) ;
					}
				}
				else if (fixedArg == 1){
					++fixedArg ;
					stringArg = *argv ;
				}
				else
					usage() ;
			}
		}
	}

	if ( fixedArg != 2   )
		usage() ;

	host = gethostbyname(hostname) ;


//	printf("Command line parsing done\n") ;

	// Creating the new server
	nSocket = socket(AF_INET, SOCK_STREAM,0) ;

	// Initialising the structure with 0
	memset(&serv_addr,0, sizeof(struct sockaddr_in)) ;

	// Filling up the structure with specifics
	serv_addr.sin_family = AF_INET ;
//	serv_addr.sin_addr.s_addr = inet_addr(hostname) ;
	serv_addr.sin_addr = *((struct in_addr *)host->h_addr);
	serv_addr.sin_port = htons(atoi(port)) ;

	// Connect to the server
	status = connect(nSocket, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) ;

	if (status < 0){
		perror("Connect") ;
		exit(1) ;
	}
	else {
	//	printf("Client connected\n") ;
		switch (choice) {
			case 1:
//				printf("Sending ADDR Request...\n");
				SendAcrossNetwork(nSocket, 0xfe10, stringArg, delay, 0) ;
				break ;
			case 2:
//				printf("Sending FSZ Request...\n");
//				fsz_request(nSocket, stringArg, delay) ;
				SendAcrossNetwork(nSocket, 0xfe20, stringArg, delay, 0) ;
				break ;
			case 3:
//				printf("Sending GET Request...\n");
				SendAcrossNetwork(nSocket, 0xfe30, stringArg, delay, offset) ;
				break ;

		}
		response_handler(nSocket, stringArg, serv_addr) ;
	}
	close(nSocket) ;
	exit(0) ;



} // End of main function

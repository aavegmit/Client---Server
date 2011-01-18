#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

void usage(){
	printf("Usage:\t ./client {adr|fsz|get} [-d delay] [-o offset] [-m] hostname:port string\n") ;
	exit(0) ;
}


int main(int argc, char *argv[]){
	struct sockaddr_in serv_addr ;
	int nSocket = 0, status, choice = 0, fixedArg = 0 ;

	// Parse the command line 
	if (argc < 3){
		usage() ;
	}
	else {
		argv++ ;
		if (strcmp(*argv, "adr") == 0) {
			choice = 1 ;
		} else if (strcmp(*argv, "fsz") == 0) {
			printf ("fsz selected\n") ;
			choice = 2 ;
		} else if (strcmp(*argv, "get") == 0) {
			printf ("get selected\n") ;
			choice = 3 ;
		} else {
			usage() ;
		}

		for (int i = 0 ; i < argc-2 ; i++, argv++) {
			if (*argv[0] == '-') {
				if (strcmp(*argv, "-m") == 0) {
					/* set a global flag */
					printf("option m selected\n") ;
				} 
				else if (strcmp(*argv, "-d") == 0) {
					argc--, argv++; /* move past "-d"*/
					if (argc <= 0) {
						usage() ;
					}
					/* read delay from *argv */
					if (!atoi(*argv)){
						printf("Bad delay argument\n") ;
						exit(0) ;
					}
					printf("delay: %d\n", atoi(*argv)) ;
				}
				else if (strcmp(*argv, "-o") == 0) {
					argc--, argv++; /* move past "-o"*/
					if (argc <= 0) {
						usage() ;
					}
					/* read offset from *argv */
					if (!atoi(*argv)){
						printf("Bad offset argument\n") ;
						exit(0) ;
					}
					printf("offset: %d\n", atoi(*argv)) ;
				}
				else
					usage() ;
			} 
			else {
				if (fixedArg == 0){
					++fixedArg ;
				}
				else if (fixedArg == 1){
					++fixedArg ;
				}
				else
					usage() ;
			}
		}
	}

	if ( fixedArg != 2   )
		usage() ;


	printf("Command line parsing done\n") ;

	// Creating the new server
	nSocket = socket(AF_INET, SOCK_STREAM,0) ;

	// Initialising the structure with 0
	memset(&serv_addr,0, sizeof(struct sockaddr_in)) ;

	// Filling up the structure with specifics
	serv_addr.sin_family = AF_INET ;
	serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1") ;
	serv_addr.sin_port = htons(12345) ;

	// Connect to the server
	status = connect(nSocket, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) ;

	if (status < 0){
		// Some error
	}
	else {
		printf("Client connected\n") ;
	}
	close(nSocket) ;
	exit(0) ;



} // End of main function

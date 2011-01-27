#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include "server_operations.h"
#include "shared.h"
#include <list>
#include <unistd.h>
#include <ctype.h>

using namespace std ;

int optionM = 0 ;
int shutTime = 60 ;
int shut_alarm = 0 ;
list<int> childList ;
list<unsigned char *> myMem ;
list<FILE *> myFile ;

void usage(){
	printf("Usage:\t ./server [-t seconds] [-m] port\n") ;
	exit(0) ;
}

void shutdown(int sig){

	if (sig == SIGALRM){
		printf("Server shutting down. Better Cleanup %d\n", sig) ;
		shut_alarm = 1 ;
	}
	else if (sig == SIGINT){
		printf("CTRL+C raised\n") ;
		shutDown = 1 ;
	}
}

void child_terminated(int sig){
	pid_t pid ;
	pid = wait(NULL) ;
	childList.remove(pid) ;
	printf("Child terminated %d, list size %d\n", (int)pid, (int)childList.size()) ;
}

void client_terminated(int sig){
	printf("Client halted\n") ;
}

int main(int argc, char *argv[])
{
	struct addrinfo hints, *servinfo, *p;
//	struct sockaddr_in serv_addr;
	int nSocket=0, portGiven = 0 , portNum = 0;
	list<int>::iterator it ;
	list<unsigned char *>::iterator itc ;
	list<FILE *>::iterator itf ;

	optionM = 0 ;

	// Setting the signals
	struct sigaction sact ;
	sigemptyset(&sact.sa_mask) ;
	sact.sa_flags = 0 ;
	sact.sa_handler = shutdown ;
	sigaction(SIGALRM, &sact, NULL) ;
	sigaction(SIGINT, &sact, NULL) ;
	
	struct sigaction sact_pipe ;
	sact_pipe.sa_handler = child_terminated ;
	sigemptyset(&sact_pipe.sa_mask) ;
	sact_pipe.sa_flags = SA_RESTART ;
	if ( sigaction(SIGPIPE, &sact_pipe, NULL) == -1 ){
		perror("sigaction") ;
		exit(1) ;
	}


//	(void) signal(SIGCHLD, child_terminated) ;
	char portBuf[10] ;

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
					optionM = 1 ;
					printf("option m selected\n") ;
				} else if (strcmp(*argv, "-t") == 0) {
					++i, argv++; /* move past "-t"*/
					if (i >= (argc-1)) {
						usage() ;
					}
					/* read offset from *argv */
					if (!atoi(*argv) || atoi(*argv) < 0 ){
						printf("Bad Timeout argument\n") ;
						exit(0) ;
					}
					shutTime = atoi(*argv) ;

					printf("Seconds: %d\n", shutTime) ;
				}
				else
					usage() ;
			} 
			else {
				if (portGiven)
					usage() ;
				if (!isdigit(*argv[0])){
					printf("Bad Port Number\n");
					exit(0);
				}
				printf("Port: %s\n", *argv) ;
				strncpy(portBuf, *argv, sizeof portBuf) ;
				portNum = atoi(*argv) ;
				portGiven = 1 ;
			}
		}
	}

	if (!portGiven)
		usage() ;

	alarm(shutTime) ;

	printf("Command line parsing done\n") ;
	
	
	

 	// Get the IP address of this machine
	char hostname[15] ;
        struct hostent *hostIP = gethostbyname("nunki.usc.edu") ;
        sprintf(hostname, "%s", inet_ntoa(*(struct in_addr*)(hostIP->h_addr_list[0]))) ;
	int rv ;
//	sprintf(portBuf, "%d", portNum) ;
	memset(&hints, 0, sizeof hints) ;
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; // use my IP
	// Code until connection establishment has been taken from the Beej's guide	
	if ((rv = getaddrinfo(NULL, portBuf, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		exit(1) ;
	}
	// loop through all the results and make a socket
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((nSocket = socket(p->ai_family, p->ai_socktype,
						p->ai_protocol)) == -1) {
			perror("talker: socket");
			continue;
		}
		int yes = 1 ;
		if (setsockopt(nSocket, SOL_SOCKET, SO_REUSEADDR, &yes,
					sizeof(int)) == -1) {
			perror("setsockopt");
			exit(1);
		}
		if (bind(nSocket, p->ai_addr, p->ai_addrlen) == -1) {
			close(nSocket);
			perror("server: bind");
			continue;
		}
		break;
	}

	// Return if fail to bind
	if (p == NULL) {
		fprintf(stderr, "talker: failed to bind socket\n");
		exit(1) ;
	}
	freeaddrinfo(servinfo); // all done with this structure
	if (listen(nSocket, 5) == -1) {
		perror("listen");
		exit(1);
	}
	///////////////////////////////////////////////////////////////////////


/*
	//creating a socket
	if ( (nSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1 ) {
		perror("Socket") ;
		exit(1) ;
	}

	// code for reusing the port later
//	 if (setsockopt(nSocket,SOL_SOCKET,SO_REUSEADDR,NULL,sizeof(int)) == -1) {
//             perror("Setsockopt");
//             exit(1);
//         }


	// Initialising the structure with 0
	memset(&serv_addr,0,sizeof(struct sockaddr_in)) ;

	// Filling up the specifics
	serv_addr.sin_family = AF_INET ;
	serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1") ;
	serv_addr.sin_port = ntohs(portNum) ;
	memset(&(serv_addr.sin_zero), '\0', 8) ;
	printf("Port: %d-\n", serv_addr.sin_port) ;

	// Bind the socket 
	if (bind(nSocket, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1) {
	        perror("Unable to bind") ;
       		exit(1) ;
	}       

	// listen on this socket
	if (listen(nSocket, 5) == -1){
		perror("Listen") ;
		exit(1) ;
	}
*/

	for(;;){
		int cli_len = 0, newsockfd = 0;
		struct sockaddr_in cli_addr ;

		// Wait for clients to connect
		newsockfd = accept(nSocket, (struct sockaddr *)&cli_addr, (socklen_t *)&cli_len ) ;
		int erroac = errno ;


		if (newsockfd < 0){
			if (erroac == EINTR){
				if (shut_alarm){
					printf("Time to shut down gracefully\n") ;
					// Signal all the child to move out of read
					//	wait(NULL) ;
					
					for (it = childList.begin(); it != childList.end(); it++){
						kill(*it, SIGINT) ;
//						childList.remove(*it) ;
					}
				}
				else if (shutDown){
					printf("CTRL + C hit\n") ;
					for (it = childList.begin(); it != childList.end(); it++){
						kill(*it, SIGINT) ;
					}
				}
			}
//			perror("accept") ;
			break ;

		} 
		else {
		//	printf("\n I got a connection from (%s , %d)", inet_ntoa(cli_addr.sin_addr),ntohs(cli_addr.sin_port));

			int pid = fork() ;

			if (pid < 0){
				/* error */
			}
			else if (pid == 0) {
				close(nSocket) ;
				printf("Connection with client established\n") ;
				server_processing( newsockfd, cli_addr ) ;
				close(newsockfd) ;
				printf("Client saying bye %d\n", (int)getpid()) ;
				printf("List size %d\n", (int)myMem.size()) ;
					for (itc = myMem.begin(); itc != myMem.end(); itc++){
						free(*itc) ;
					}
					for (itf = myFile.begin(); itf != myFile.end(); itf++){
						fclose(*itf) ;
					}
				exit(0) ;
			}
			childList.push_back(pid) ;
			close(newsockfd) ;
		}

	}
//	for (it = childList.begin(); it != childList.end(); it++){
//		waitpid(*it, (int *)0, NULL) ;
//		//	waitpid(*it, (int *)0, WNOHANG) ;
//	}
		int waitV ;
	while(!(wait(&waitV) == -1) ) ;
//	while(!childList.empty())
//		sleep(5) ;
	printf("Server final good bye\n") ;
	close(nSocket) ;
	exit(0) ;

}

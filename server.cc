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
	printf("Child terminated %d, list size %d\n", pid, (int)childList.size()) ;
}

void client_terminated(int sig){
	printf("Client halted\n") ;
}

int main(int argc, char *argv[])
{
	struct sockaddr_in serv_addr;
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
	sigemptyset(&sact_pipe.sa_mask) ;
	sact_pipe.sa_flags = 0 ;
	sact_pipe.sa_handler = client_terminated ;
	sigaction(SIGPIPE, &sact_pipe, NULL) ;
	(void) signal(SIGCHLD, child_terminated) ;

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
					argc--, argv++; /* move past "-t"*/
					if (argc <= 0) {
						usage() ;
					}
					/* read offset from *argv */
					if (!atoi(*argv) || atoi(*argv) <= 0 ){
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

	alarm(shutTime) ;

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
			break ;

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
				close(newsockfd) ;
				printf("Client saying bye %d\n", getpid()) ;
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

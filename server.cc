#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main(int argc, char *argv[])
{
	struct sockaddr_in serv_addr;
	int nSocket=0 ;

	//creating a socket
	nSocket = socket(AF_INET, SOCK_STREAM, 0) ;

	// Initialising the structure with 0
	memset(&serv_addr,0,sizeof(struct sockaddr_in)) ;

	// Filling up the specifics
	serv_addr.sin_family = AF_INET ;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY) ;
	serv_addr.sin_port = htons(12345) ;

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
				// Close the parent socket
				close(nSocket) ;
				printf("Connection with client established\n") ;
				exit(0) ;
			}
			close(newsockfd) ;
		}

	}
	close(nSocket) ;
	exit(0) ;

}

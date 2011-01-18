#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main(int argc, char *argv[]){
	struct sockaddr_in serv_addr ;
	int nSocket = 0, status ;

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

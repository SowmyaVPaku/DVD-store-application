//Homework 2: Program 2: TCP client Program.
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
//Error Method
void error(const char *msg)
{
    perror(msg);
    exit(0);
}
//Main method, command line arguments portnumber and local host
//values store in argv and number of values will be stored in argc
int main(int argc, char *argv[])
{
    int iSock, iPort, n;
	//sockaddr_in is the structure that contains address family, port number and the IP address.
    struct sockaddr_in sockServeraddr;
	//hostent structure contains Inet_addr, gethostbyname and the host IPaddress in binary. 
    struct hostent *server;
//Buffer is used to store the data.
    char buffer[256];
	
    if (argc != 4 && argc != 6) {
       fprintf(stderr,"usage %s hostname port command\n", argv[0]);
       exit(0);
    }
		
	if ((strcmp(argv[3],"order") != 0) && strcmp(argv[3],"list") != 0)
	{
		fprintf(stderr,"Invalid request. The Only allowed request commands are 1. list 2. order\n");	
		exit(0);
	}
	
    iPort = atoi(argv[2]);
	//create a socket, protocol = 0, =>raw socket. 
    iSock = socket(AF_INET, SOCK_STREAM, 0);
    if (iSock < 0) 
        error("ERROR opening socket");
    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
	//flush the server buffer
    bzero((char *) &sockServeraddr, sizeof(sockServeraddr));
	//set server internet address family.
    sockServeraddr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
         (char *)&sockServeraddr.sin_addr.s_addr,
         server->h_length);
		 //set the server port.
    sockServeraddr.sin_port = htons(iPort);
	//connect.
    if (connect(iSock,(struct sockaddr *) &sockServeraddr,sizeof(sockServeraddr)) < 0) 
        error("ERROR connecting to the socket");
//flush the buffer.
    bzero(buffer,256);
	if(argc==4)
	strcpy(buffer, argv[3] );
	else if (argc==6)
	//store the arguments in the buffer.
	sprintf(buffer, "%s\t%s\t%s", argv[3],argv[4],argv[5] );
	//write the buffer to the client.
    n = write(iSock,buffer,strlen(buffer));
	//If no messages sent. error.
    if (n < 0) 
         error("ERROR writing to the socket");
    bzero(buffer,256);
	//Read form the buffer.
    n = read(iSock,buffer,255);
	//if no messages are received, error
    if (n < 0) 
         error("ERROR reading from the socket");
    printf("%s\n",buffer);
	
	strcpy(buffer,"Client received your response");
	//Write back the acknowledge to the server.
	n = write(iSock,buffer,strlen(buffer));
	//n<0 implies no message received. Print an error message.
    if (n < 0) 
         error("ERROR writing to the socket");
    close(iSock);
    return 0;
}

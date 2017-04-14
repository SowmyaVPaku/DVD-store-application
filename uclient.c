//Homework 2: program 2: UDP client program.
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>	//contains hostent structure.
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h> //contains socklen_t
 //Funtion to print the error.
void error(char *);
//Main method, command line arguments portnumber and local host
//values store in argv and number of values will be stored in argc
int main(int argc, char *argv[])
{
int sock, n;
//sockaddr_in is the structure that contains address family, port number and the IP address.
struct sockaddr_in server, client;
//hostent structure contains Inet_addr, gethostbyname and the host IPaddress in binary. 
struct hostent *hp;
socklen_t length;
char buffer[1024];


if(argc!=4 && argc != 6)
{
printf("Usage:Serrver port command\n");
exit(1);

}
if ((strcmp(argv[3],"order") != 0) && strcmp(argv[3],"list") != 0)
{
fprintf(stderr,"Invalid request. The Only allowed request commands are 1. list 2. order\n");	
exit(1);
}


//create a socket, protocol = 0, =>raw socket. 
sock=socket(AF_INET, SOCK_DGRAM, 0);
if(sock<0)
{

error("Unable to create socket\n");
}
//setting memory.
memset((char*)&server, 0, sizeof(server));
//set server internet address family.
server.sin_family = AF_INET;
hp = gethostbyname(argv[1]);
if(hp==0)
{
error("unknown host");

}
bcopy((char *)hp->h_addr, (char *)&server.sin_addr,hp->h_length);
server.sin_port = htons(atoi(argv[2]));
length=sizeof(server);

bzero(buffer,1024);
//store the arguments in the buffer.
sprintf(buffer, "%s\t%s\t%s", argv[3],argv[4],argv[5] );
//send the buffer to the client.
if(sendto(sock,buffer,strlen(buffer),0,(struct sockaddr *)&server,length) <0) error("Sendto");
//clear the buffer.
bzero(buffer,1024);
//Receive the buffer from the server.
if((recvfrom(sock, buffer,1023,0,(struct sockaddr *) &server, &length)) <0) error("recvfrom\n");
printf("%s\n",buffer);
//n<0 implies no message received. Print an error message.
if(n<0)
{
error("recvfrom\n");
}
}

void error(char *msg)
{

perror(msg);
exit(1);
}



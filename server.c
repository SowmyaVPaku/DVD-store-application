/*Home work 2: Program 2:  A Multiservice Concurrent Server Program handling both TCP and UDP.
   The port numbers are passed as arguments */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <netdb.h>
#include <arpa/inet.h>

//DVD Method.
//DVD Inventory List.
struct DVDStore
{
char Item_no[10];
char Title[200];
int Quantity;
} DVDInventory[3] = {{"1001","Star Wars",100},{"1002","Inside Out",80},{"1003","Harry Potter",50}};

//Structure to handle mutex and log statistics. 

struct {
	pthread_mutex_t st_mutex;		//Mutex.
	unsigned int st_tcp_concount;
	unsigned int st_udp_concount;
	unsigned int st_concount;		//Total number of client request addressed. 
	unsigned int st_order_count;	//Number of DVDs purchased since the server started.
} stats;

//DVD Mutex to handle DVD order calculations.
pthread_mutex_t DVDMutex = PTHREAD_MUTEX_INITIALIZER;

//Error Method. Funtion to print error message.
void error(const char *msg)
{
    perror(msg);
    exit(1);
}

//UDP client Arguments passed through the function to handle UDP threads.
struct UDPClientArgs
	 {
		struct sockaddr_in *Argscli_addrT;
		int ArgssockfdU;
		char Argsbuffer[1024];
	 };
// Function to handle TCP clients.
void *process_tcp(void *);
//Funtion to handle UDP clients.
void *process_udp(void *UDPClientArgs);
// Funtion to print the statistics.
void printstats();
//string DVDCalculations(char * bufferstr);

//Main method. Command line arguments are the port numbers of TCP and UDP.
//Number of arguments stored in argc and the values of each argument stored in argv.
int main(int argc, char *argv[])
{
	(void) pthread_mutex_init(&stats.st_mutex,0);	// Initializing mutex 
	
	//To acquire a lock on the mutex variable.
	//As soon as the server is started, the number of connection count and the number of DVDs are set to 0.
	(void) pthread_mutex_lock(&stats.st_mutex);		
	stats.st_concount=0;
	stats.st_tcp_concount=0;
	stats.st_udp_concount=0;
	stats.st_order_count=0;	
	(void) pthread_mutex_unlock(&stats.st_mutex);//Release lock.
	//Print the initial statistics.
	printstats();
	char ipAddress[INET_ADDRSTRLEN];
	
     int sockfdT,sockfdU; 				//TCP and UDP sockets.
	 socklen_t clilen_udp,clilen_tcp;	//length of the client addresses of TCP and UDP.
	 int  portno_udp, portno_tcp; 		//Port numbers of TCP and UDP.
	 
	 struct sockaddr_in server_tcp, server_udp; //Server.
	 struct sockaddr_in cli_addrT, cli_addrU; //client variables of TCP and UDP.
	 socklen_t servlen_tcp, servlen_udp; // length of the server addresses of TCP and UDP.
	  	 
	 int newsockfd; //Slave socket.
	 
	 //Declaring a structure to pass through the UDP handle function. 	 	 
	struct UDPClientArgs UCPClientArgs;
	char buffer[1024];
	int n;  
	
		 //If the arguments are not given.
     if (argc < 3) {
         fprintf(stderr,"Ports not provided ERROR.........\n");
         exit(1);
     }
	 
	 //Creating a TCP socket.
	 sockfdT = socket(AF_INET, SOCK_STREAM, 0); 
	 //Creating a UDP socket.
     sockfdU = socket(AF_INET, SOCK_DGRAM, 0); 	 
	 
	//To check if the socket is created properly.
	 if (sockfdT < 0) 
        error("unable to create socket.....\n");

	 if (sockfdU < 0) 
        error("Unable to Create Socket....\n");

	//Clear the server before the initial connection begins.
	 bzero((char *) &server_tcp, sizeof(server_tcp)); 
	 bzero((char *) &server_udp, sizeof(server_udp)); 
	 
	//Initializing TCP and UDP ports.	
     portno_tcp = atoi(argv[1]); 
	 portno_udp = atoi(argv[2]); 
	 	 
	 //Declaring the TCP server configurations.
     server_tcp.sin_family = AF_INET; 
     server_tcp.sin_addr.s_addr = INADDR_ANY;
     server_tcp.sin_port = htons(portno_tcp);
	 
	 //Declaring the UDP server configurations.
     server_udp.sin_family = AF_INET; 
     server_udp.sin_addr.s_addr = INADDR_ANY;
     server_udp.sin_port = htons(portno_udp);
	 
	//Server address lengths of TCP and UDP. 
	servlen_tcp = sizeof(server_tcp);
	servlen_udp = sizeof(server_udp);
     	 
	 
	 //Bind the socket.
     if (bind(sockfdT, (struct sockaddr *) &server_tcp, sizeof(server_tcp)) < 0) 
     error("Unable to bind the socket...\n");

	 //Bind the socket.
     if (bind(sockfdU, (struct sockaddr *) &server_udp, sizeof(server_udp)) < 0) 
     error("Unable to Bind the Socket....\n"); 
			  
	//Listen to the TCP connection. 5 clients can be in Queue.		  
     listen(sockfdT,5);
	 //Read file descriptor.
	fd_set rfds; 
//Active file descriptor.	
    fd_set afds;  
    int nfds = getdtablesize();
    FD_ZERO(&afds);
	
	// Start processing the client requests.
	while(1)
    {
        FD_SET(sockfdT, &afds);
        FD_SET(sockfdU, &afds);
		memcpy(&rfds, &afds, sizeof(rfds));
        
		//Select() call - Used for multiservice application.
        select(nfds, &rfds, (fd_set *)0, (fd_set *)0, (struct timeval *)0);
		
		//To ckeck if the client connection is TCP. If TCP client, process the TCP request.
		if(FD_ISSET(sockfdT, &rfds))
        {
			printf("Processing TCP client request...\n");
			clilen_tcp = sizeof(cli_addrT);
			//Accepting the TCP client request. Assigned to the slave socket.
		    newsockfd = accept(sockfdT, (struct sockaddr *) &cli_addrT, &clilen_tcp);	
			if (newsockfd < 0) 
			error("ERROR on accept");
		
			//To print the IPaddress and port number of the server.
			inet_ntop(AF_INET, &((&server_tcp)->sin_addr), ipAddress, INET_ADDRSTRLEN);
			printf("Server IP address is: %s\n",ipAddress);
			if (getsockname(sockfdT, (struct sockaddr *)&server_tcp, &servlen_tcp) == -1) 
			perror("getsockname");
			else
			printf("server port number %d\n", ntohs(server_tcp.sin_port));
		
			//Printing the client IP address and Port number on the server console if the client connection is accepted by server.			
            printf("Accepted the client request.\n");
            printf("IP address of the client is..%s\n", inet_ntoa(cli_addrT.sin_addr));
            printf("Port Number of client is.. %d\n", ntohs(cli_addrT.sin_port));	
				 
			//New thread to process the client request. 
            pthread_t id1;
			//Thread created to handle the client request. process_tcp() function is called with the argument &newsockfd.
            if( pthread_create(&id1, NULL, process_tcp, &newsockfd) < 0)
            error("Error while creating threads");
			 pthread_join(id1, NULL);
			 //Closing slave socket.
			close(newsockfd);
			printstats();
            
		}
		//To ckeck if the client connection is UDP. If UDP client, process the UDP request.
		else if(FD_ISSET(sockfdU, &rfds))
        {
			//Flushing the buffer.
			bzero(buffer,1024);
			clilen_udp = sizeof(cli_addrU);
			bzero(&cli_addrU, sizeof(cli_addrU));
			
			n = recvfrom(sockfdU, buffer, sizeof(buffer), 0 , (struct sockaddr *)&cli_addrU,(socklen_t *)&clilen_udp );
			if(n<0)
			{																								
			error("Error recv from 1\n");
			}		
			//To print the IPaddress and port number of the server.
			inet_ntop(AF_INET, &((&server_udp)->sin_addr), ipAddress, INET_ADDRSTRLEN);
			printf("Server IP address is: %s\n",ipAddress);
			if (getsockname(sockfdU, (struct sockaddr *)&server_udp, &servlen_udp) == -1) 
			perror("getsockname");
			else
			printf("server port number %d\n", ntohs(server_udp.sin_port));
			
			//Printing the client IP address and Port number.
			printf("Processing UDP request...\n");			
			printf("Client IP address...%s\n", inet_ntoa(cli_addrU.sin_addr));
			printf("Client Port Number...%d\n", ntohs(cli_addrU.sin_port));
			
			//Structure arguments. Passing the client IP address, Socket and buffer as arguments to the process_udp() function.
			UCPClientArgs.Argscli_addrT = &cli_addrU;
			UCPClientArgs.ArgssockfdU = sockfdU;
			strcpy(UCPClientArgs.Argsbuffer,buffer);		
		
           //New thread creation to access UDP client request.
            pthread_t id2;
            if(pthread_create(&id2, NULL, process_udp, &UCPClientArgs) < 0)
            {
                error("Error while creating threads");
            }
            //pthread_join(id2, NULL);
			
            
        }
	
	}//while ends.
	
return 0;
}//main 

// Function to handle TCP client operations. List and order functions.
void *process_tcp(void *sock_ptr)
{
	//Incrementing number of tcp client connections and total connections.
	(void) pthread_mutex_lock(&stats.st_mutex);
	stats.st_concount++;
	stats.st_tcp_concount++;
	(void) pthread_mutex_unlock(&stats.st_mutex);
   
     int *newsockfd = (int *)sock_ptr;
	
	 char buffer[1024];
	 char outBuffer[1024];
	 char *token;
	 const char s[2] = "\t";
	 char command[3][7];		//used for tokenizing the arguments.
     int n, i=0, found;
	 char ipAddress[INET_ADDRSTRLEN];
    
	bzero(buffer,1024);
    //Reading the client message 
    n = read(*newsockfd,buffer,1024);
    if(n<0)
        error("\nError in read");
	//Breaking the arguments into tokens.
	 token = strtok(buffer, s);
	 i=0;
	 while( token != NULL ) 
	   {
	      strcpy(command[i],token);
		  token = strtok(NULL, s);
		  i++;
	   }	 
	 //Breaking down the arguments into tokens end.
	
	//If the command is list.
		if (strcmp(command[0], "list")== 0)
		{
		printf("Client has requested the DVD list...\n");
		
		bzero(outBuffer,1024);
		sprintf(outBuffer, "\n Item Number \t Title \t\t Quantity \n");
		//Storing the structure in the buffer.
		for(i=0;i<3;i++)
		{
		sprintf(outBuffer, "%s%s \t\t %s \t\t %d \n", outBuffer,DVDInventory[i].Item_no,DVDInventory[i].Title,DVDInventory[i].Quantity);
		if (n < 0) error("ERROR writing to socket");
		}
		//Printing the buffer at the server logs.
		printf("The message sent to the client is....%s\n",outBuffer);
		//Writing the buffer to the client.
		n = write(*newsockfd,outBuffer,strlen(outBuffer));
		if (n < 0) error("ERROR writing to socket");
		}
	//List operation ends.
	 	
	//If the argument is order, The server compares with the item number in the list provided and check the quantity.
		//If the quantity is present, server will reply with the message 'OK' to the client.
	 if (strcmp(command[0], "order") == 0)
		{
		printf("Client has placed an order.....\n");
		i=0;
		found = 1; 
			while (i<3)
			{
				if((strcmp(DVDInventory[i].Item_no, command[1]) == 0) && (DVDInventory[i].Quantity - atoi(command[2])) >= 0 && atoi(command[2])>0)
				{
					//Locking the order operation. 
					pthread_mutex_lock(&DVDMutex);
					DVDInventory[i].Quantity = DVDInventory[i].Quantity - atoi(command[2]);
					pthread_mutex_unlock(&DVDMutex);
					//Counts the number of DVDs client wants to purchase.
					(void) pthread_mutex_lock(&stats.st_mutex);
					stats.st_order_count=stats.st_order_count+atoi(command[2]);	
					(void) pthread_mutex_unlock(&stats.st_mutex);
				//If the order is success, server writes to client "OK"
				printf("MESSAGE SENDING: OK..\n");
				n = write(*newsockfd,"OK",strlen("OK"));
				if (n < 0) error("ERROR writing to socket");
				found = 0;
				break;
				}
				//If the Requested quantity is not available.
				else if((strcmp(DVDInventory[i].Item_no, command[1]) == 0) && (DVDInventory[i].Quantity - atoi(command[2])) < 0)
				{
				printf("MESSAGE SENDING: Requested Quantity is not available..\n");
				n = write(*newsockfd,"Requested Quantity is not available",strlen("Requested Quantity is not available"));
				if (n < 0) error("ERROR writing to socket");
				found = 0;
				break;
				}
				else if ((strcmp(DVDInventory[i].Item_no, command[1]) == 0) && ((atoi(command[2]) < 0) ))
				{
					printf("MESSAGE SENDING: Invalid Order Quantity..\n");
					n = write(*newsockfd,"Invalid Order Quantity..",strlen("Invalid Order Quantity.."));
					if (n < 0) error("ERROR writing to socket");
					found = 0;
					break;
				}
					
			i++;	
			}
			//printf("\n found = %d \n",found);
			//If the requested item is not avaiable in the list.
			if (found == 1)
			{
				printf("MESSAGE SENDING: ITEM NOT FOUND..\n");
				n = write(*newsockfd,"Item not found",strlen("Item not found"));
				if (n < 0) error("ERROR writing to socket");
			}				
		}
	 //Order operation ends.
	 //closing the slave socket.
    close(*newsockfd);
	
	//Decrementing the number of threads.
	(void) pthread_mutex_lock(&stats.st_mutex);
	stats.st_tcp_concount--;
	(void) pthread_mutex_unlock(&stats.st_mutex);
   //Thread exit.
    pthread_exit(NULL);
}


// Function to handle UDP client operations. List and order functions.
void *process_udp(void *UDPClientArgs)
{
	//Incrementing number of udp client connections and total connections.
	(void) pthread_mutex_lock(&stats.st_mutex);
	stats.st_udp_concount++;
	stats.st_concount++;
	(void) pthread_mutex_unlock(&stats.st_mutex);
   
	
	//struct UDPClientArgs *UDPArguments;
//	UDPArguments = UDPArgs;

struct UDPClientArgs *UDPArguments = (struct UDPClientArgs *)UDPClientArgs;
 
  char buffer[1024];
  char outBuffer[1024];
  struct sockaddr_in cli_addrUf;
  socklen_t clilen_udp;
  int n, i,k=0, found;;
  char *token;     //*token is taken for parsing
  const char s[] = "\t";  //delimiter
  char command[3][7];	//Parsed arguments are stored in this array.
  char ipAddress[INET_ADDRSTRLEN];	//to store the internet address
 
 int sockfdU =  (UDPArguments->ArgssockfdU);
 strcpy(buffer,UDPArguments->Argsbuffer);
 cli_addrUf = *(UDPArguments->Argscli_addrT);
 clilen_udp = sizeof(cli_addrUf);
 

	 int portno = ntohs(cli_addrUf.sin_port); 
	
//breaking down the args//
token=NULL;
memset(command, 0, sizeof(command));
token = strtok(buffer, s);
	 i=0;
	 while( token != NULL ) 
	   {
	strcpy(command[i],token);
		  //printf( " %s\n", token );
		  token = strtok(NULL, s);
		  i++;
	   }
	   token=NULL;

 cli_addrUf.sin_family = AF_INET ;
 cli_addrUf.sin_port = htons(portno);
 
 
//If the first argument is list, the list will be printed
if (strcmp(command[0], "list") == 0)
		{ 
	
		printf("Received a command to send the list to the client\n");
		//flush the buffer.
		bzero(buffer,1024);
		strcpy(buffer,"\n Item Number \t\t Title \t\t Quantity\n");
		for(i=0;i<3;i++)
				{
				//To convert into string.The list of items are stored in buffer 'buff'.
				sprintf(buffer, "%s %s \t\t %s \t\t %d \t\t\n", buffer, DVDInventory[i].Item_no,DVDInventory[i].Title,DVDInventory[i].Quantity);			
				}

		clilen_udp = sizeof(cli_addrUf);
		//Send the list of items to the client 
		n = sendto(sockfdU, buffer, sizeof(buffer),0,(struct sockaddr *)&cli_addrUf,clilen_udp );
						if (n < 0) error("ERROR writing to socket");

				

		}
		//If the first argument is order, The server compares with the item number in the list provided and check the quantity.
		//If the quantity is present, server will reply with the message 'OK' to the client.
else if (strcmp(command[0], "order") == 0)
		{
		//logging the order at the server.
		printf("Command is to order Items\n");
		printf("The order has been requested for the item number %s:", command[1]);
		printf("and the quantity is %s\n",command[2]);
		
		i=0;
		found = 1; 
		k=0;
		bzero(buffer,1024);
			while (k<3)
			{
		
			if((strcmp(DVDInventory[k].Item_no, command[1]) == 0) && (DVDInventory[k].Quantity - atoi(command[2])) >= 0 && atoi(command[2])>0)
			{
			//If the quantity of the ordered item present, server will reply with the message 'OK' to the client updating the list.
				pthread_mutex_lock(&DVDMutex);
				DVDInventory[k].Quantity = DVDInventory[k].Quantity - atoi(command[2]);
				pthread_mutex_unlock(&DVDMutex);
				//Counts the number of DVDs purchased.	
				(void) pthread_mutex_lock(&stats.st_mutex);
				stats.st_order_count=stats.st_order_count+atoi(command[2]);	
				(void) pthread_mutex_unlock(&stats.st_mutex);
				
				
				strcpy(buffer, "OK");
				printf("Message sending: OK..\n");
				n = sendto(sockfdU, buffer, strlen(buffer),0,(struct sockaddr *)&cli_addrUf, sizeof(cli_addrUf));
				if(n < 0) error("ERROR writing to socket");
				found = 0;
				break;
				}
//If the requested quantity is less than the quantity present, server will reply with the message 'Quantity not available.'
			else if((strcmp(DVDInventory[k].Item_no, command[1]) == 0) && (DVDInventory[k].Quantity - atoi(command[2])) < 0)
				
				{
				printf("Message sending: Quantity Unavailable\n");
			n = sendto(sockfdU,"Requested Quantity Unavailable",strlen("Requested Quantity Unavailable"),0,(struct sockaddr *)&cli_addrUf,sizeof(cli_addrUf));
				if (n < 0) error("ERROR writing to socket");
				found = 0;
				break;
				}
			else if ((strcmp(DVDInventory[k].Item_no, command[1]) == 0) && ((atoi(command[2]) < 0) ))
				{
					printf("MESSAGE SENDING: Invalid Order Quantity..\n");
					n = sendto(sockfdU,"Invalid Order Quantity..",strlen("Invalid Order Quantity.."),0,(struct sockaddr *)&cli_addrUf,sizeof(cli_addrUf));
					if (n < 0) error("ERROR writing to socket");
					found = 0;
					break;
				}	
				
			k++;
			}
			
			if (found == 1)
			   {
				 printf("Message sending: ITEM NOT FOUND..\n "); 
				n = sendto(sockfdU,"Item not found",strlen("Item not found"),0,(struct sockaddr *)&cli_addrUf,sizeof(cli_addrUf));
				if (n < 0) error("ERROR writing to socket");
			   }
		}
  
   (void) pthread_mutex_lock(&stats.st_mutex);
	stats.st_udp_concount++;
  (void) pthread_mutex_unlock(&stats.st_mutex);
printstats();
  pthread_exit(NULL); 
  //close(sockfdU);

 
}

//Printing the statistics- number of connections and the number of DVDs purchased since the server has started. 
void printstats()
{
printf("\n Total # of connections since server started: %d \n",stats.st_concount);
printf("\n Total # of Orders placed till now: %d \n",stats.st_order_count);
printf("*****************************************************************\n");
}

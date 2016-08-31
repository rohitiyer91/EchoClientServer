/*
File Name 		: EchoClient.c
Project Name 	: EchoCientServer
Author 			: Rohit Iyer
Purpose			: This is a preliminary file for testing the Client part of a UDP client/server communication
*/

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include<errno.h>

#define SIZE 1000

//Defining the server port Id to be unique as hinted in the problem statement
#define RECIPIENT_PORT 	IPPORT_RESERVED+3459

//Predefined strcuture declaration
 typedef struct {
 unsigned int length;
 unsigned char data[SIZE];
 } Message;

 typedef enum {
 OK,  /* operation successful */ 
 BAD,  /* unrecoverable error */ 
 WRONGLENGTH  /* bad message length supplied */
 } Status;

 
typedef struct sockaddr_in SocketAddress ;

//Structures from UDPSock.c
struct hostent *gethostbyname() ;
void printSA(struct sockaddr_in sa) ;
void makeDestSA(struct sockaddr_in * sa, char *hostname, int port) ;
void makeLocalSA(struct sockaddr_in *sa) ;

//Function declaration prototype from question
Status DoOperation (Message *message, Message *reply, int sock, SocketAddress serverSA);
Status  UDPsend(int sock, Message *msg, SocketAddress dest);
Status  UDPreceive(int sock, Message *msg, SocketAddress *orig);

//This is the client module
//The main function will trigger the client process and expect only one argument - "server address"
//Usage from cmd line : ./EchoClient <hostname>
void main(int argc,char **argv)
{
	   int serverPort = RECIPIENT_PORT;
	   int clientSock;
	   char *serverName;
	   SocketAddress mySocketAddress, yourSocketAddress;
	   Message clientMsg,replyMsg;
	   Status status; 

	   //Step1 : Create socket.
	   if((clientSock = socket(AF_INET, SOCK_DGRAM, 0))<0) {
		printf("EchoClient :: socket creation failed");
		exit(1);
	   }

	   //Step2: Check for server info in argument list.
	   if(argc<=1){
		  printf("EchoClient :: server name not provided \n");
		  exit(1);
	   }
  
	   serverName = argv[1];

	   //Step3: Create a SocketAddress struct for the server machine.
	   makeDestSA(&yourSocketAddress,serverName, serverPort);
	   
	   //Keep taking user input to be sent to the user from the terminal
	   while(1){
		  printf("\nEnter Message: ");
		  gets(clientMsg.data);
		  
		  //DoOperation is called and if it returns other than OK, it exits from the while loop         
		  if((status=DoOperation(&clientMsg, &replyMsg, clientSock, yourSocketAddress)) != OK){
			 perror("\n EchoClient :: Failed to send message to server");
			 exit(1);           
		  }
		  //Important to clear the string as the next user input might have content from previous input
		  memset(replyMsg.data,0,SIZE); 		
	   }           
}

/*
Function Name 	: DoOperation
Inputs 			: Message * , Message * , int , SocketAddress
Outputs 		: Status
Purpose 		: This will call UDPSend to send a message to the server and block the socket till it received a response from the server.
*/
Status DoOperation (Message *message, Message *reply, int sock, SocketAddress serverSA){

     Status status;

     //Call UDPSend and pass the client socket, server address and the message to be sent.
	 //if the call succeeds, wait for reply, else send back error response
	 message->length = strlen(message->data)+1;
     if((status=UDPsend(sock, message, serverSA))!=OK){
            printf("DoOperation :: Failed to send message to server.");
            return status;
      }

     if((status = UDPreceive(sock, reply, &serverSA))!=OK){
            printf("DoOperation :: Failed to recieve data from server.");
            return status;
     }

     printf("Reponse from Server : %s \n",reply->data);
     return status;
}


/*
Function Name 	: UDPsend
Inputs 			: int , Message *  , SocketAddress
Outputs 		: Status
Purpose 		: This will call call sendto to send a message to the destination and send back response of the call
*/
Status  UDPsend(int sock, Message *msg, SocketAddress dest){

    Status status = OK; //Being optimistic
    int n;

	//First check for termination condition. If so, then send the message and then exit.
	//Else send the message and wait ask the client to wait for reply
    
	if( (n = sendto(sock, msg->data, msg->length, 0, (struct sockaddr *)&dest, sizeof(dest))) < 0){
		status=BAD;
		printf("\n UDPsend :: failed to send to destination");
	}
	
	if (strcmp(msg->data,"q")==0){
		exit(1);
	}
        
   return status;
}


/*
Function Name 	: UDPreceive
Inputs 			: int , Message * , SocketAddress *
Outputs 		: Status
Purpose 		: This will accept a message from the origin of the message
*/
Status  UDPreceive(int sock, Message *msg, SocketAddress *orig){

    Status status = OK; //being optimistic
    int n,origLen;
	
    origLen = sizeof(orig);
    
	//Call recvFrom to check for response from the origin.
	//If response comes back, send back OK ,else send back error
	if((n = recvfrom(sock, msg->data, SIZE, 0, (struct sockaddr *)orig, &origLen))<0){
		status = BAD;
		printf("\n UDPreceive :: Failed to recieve from origin");
		return status;
    }

    int len = strlen(msg->data)+1;    //adding 1 because string includes null character in the end.
        
    if(n != len){
        printf("UDPreceive :: wrong message length received");
        status=WRONGLENGTH;         
     }
    return status;
}

/*print a socket address */
void printSA(struct sockaddr_in sa)
{
	char mybuf[80];
	char *ptr=inet_ntop(AF_INET, &sa.sin_addr, mybuf, 80);
	printf("Server address details  %d, %s, %d\n", sa.sin_family, mybuf, ntohs(sa.sin_port));
}

/* make a socket address for a destination whose machine and port
	are given as arguments */
void makeDestSA(struct sockaddr_in * sa, char *hostname, int port)
{
	struct hostent *host;

	sa->sin_family = AF_INET;
	if((host = gethostbyname(hostname))== NULL){
		printf("Unknown host name\n");
		exit(-1);
	}
	sa-> sin_addr = *(struct in_addr *) (host->h_addr);
	sa->sin_port = htons(port);
}

/* make a socket address using any of the addressses of this computer
for a local socket on any port */
void makeLocalSA(struct sockaddr_in *sa)
{
	sa->sin_family = AF_INET;
	sa->sin_port = htons(0);
	sa-> sin_addr.s_addr = htonl(INADDR_ANY);
}

/* make a socket address using any of the addressses of this computer
for a local socket on given port */
void makeReceiverSA(struct sockaddr_in *sa, int port)
{
	sa->sin_family = AF_INET;
	sa->sin_port = htons(port);
	sa-> sin_addr.s_addr = htonl(INADDR_ANY);
}

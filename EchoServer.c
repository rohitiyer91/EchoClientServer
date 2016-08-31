/*
File Name 		: EchoServer.c
Project Name 	: COM S 554 Programming Project 1000
Author 			: Rohit Iyer(930363459)
Purpose			: This is a preliminary file for testing the Server part of a UDP client/server communication
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
void makeLocalSA(struct sockaddr_in *sa);
void makeReceiverSA(struct sockaddr_in *sa, int port);

//Function declaration prototype from question
Status GetRequest (Message *callMessage, int sock, SocketAddress *clientSA);
Status SendReply (Message *replyMessage, int sock, SocketAddress clientSA);
Status UDPsend(int sock, Message *msg, SocketAddress dest);
Status UDPreceive(int sock, Message *msg, SocketAddress *orig);

//This is the SERVER module
//The main function will trigger the SERVER process.
void main(int argc,char **argv)
{     
	int sock;
	SocketAddress mySocketAddress,yourSocketAddress;
	Message callMessage,replyMessage;
	Status status;
	
    int port = RECIPIENT_PORT;

	//Make this the local socket
    //makeLocalSA(&mySocketAddress);
	makeReceiverSA(&mySocketAddress , port);
 
	//Step1 : Create socket.
   if((sock = socket(AF_INET, SOCK_DGRAM, 0))<0) {
		printf("EchoServer :: socket creation failed");
		exit(1);
   }

	 //Step2: Bind the socket to the local socket address
	 if( bind(sock, (struct sockaddr *)&mySocketAddress, sizeof(struct sockaddr_in))!= 0){
		perror("EchoServer :: Bind failed\n");
		close(sock);
		exit(1);
	}
		 
	 printf("\nEchoServer :: Server Initiated\n");

	 //Wait for a message from the client process
	 while(1){
		//Get the message from the client.       
		 if((status=GetRequest (&callMessage,sock, &yourSocketAddress))!=OK){
			 perror("EchoServer :: Unable to receive message from client.");
			 exit(1);
		}
			
		 //Check if the termination condition is reached
		 if(strcmp(callMessage.data , "q")==0){
			 printf("\n EchoServer:: Termination condition Q. Exiting\n");
			 exit(1);
		 }

		 printf("EchoServer :: Message from Client: %s \n",callMessage.data);

		 //clearing the message buffer before sending back response.
		 memset(callMessage.data,0,SIZE);

		 // Creating a message which needs to be sent to client
		 //sprintf(replyMessage.data, "Message acknowledged");
		 char message1[]="ACK :: Message recieved.";
		 strcpy(replyMessage.data,message1);
		 replyMessage.length = strlen(replyMessage.data)+1;
		 
		 //SendReply send the message to client using UDPreceive
		 
		 if ((status = SendReply(&replyMessage, sock , yourSocketAddress))!=OK){
			 perror("\n EchoServer :: Failed to send reply to client.");
			 exit(1);
		  }
	  }    
}


/*
Function Name 	: GetRequest
Inputs 			: Message * , Message * , int , SocketAddress
Outputs 		: Status
Purpose 		: This will call UDPSend to send a message to the server and block the socket till it received a response from the server.
*/
Status GetRequest(Message *callMessage, int sock , SocketAddress *clientSA){
     Status status = UDPreceive(sock, callMessage,clientSA);
     return status;
}


/*
Function Name 	: UDPreceive
Inputs 			: int , Message * , SocketAddress *
Outputs 		: Status
Purpose 		: This will accept a message from the origin of the message
*/
Status SendReply (Message *replyMessage, int s, SocketAddress clientSA){
    Status status=UDPsend(s, replyMessage, clientSA);
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
	printf(" Message received from sa = %d, %s, %d\n", sa.sin_family, mybuf, ntohs(sa.sin_port));
}

/* make a socket* address using any of the addressses of this computer
for a local socket on any port */
void makeLocalSA(struct sockaddr_in *sa)
{
	sa->sin_family = AF_INET;
	sa->sin_port = htons(RECIPIENT_PORT);
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

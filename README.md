# EchoClientServer
This project provides a basic UDP client and server written in C.
The server portion is implemented by EchoServer.c, by intiailizing a UDP server at a port to be specified by the user at runtime.
The server will listed to all incoming connections on this port.

The client portion is implemented by EchoClient.c again talking to the server on a prespecified port. Since this is UDP, the client wil not wait for any response from the server.

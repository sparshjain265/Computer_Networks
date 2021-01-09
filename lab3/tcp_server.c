#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main() {
	const int port = 9000;

	char server_message[256] = "You have reached the server.";

	// create a socket
	int server_socket = socket(AF_INET, SOCK_STREAM, 0);

	// specify address
	struct sockaddr_in server_address;
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(port);
	server_address.sin_addr.s_addr = INADDR_ANY; 
	
	// bind the socket to our specified IP and port
	bind(server_socket, (struct sockaddr*) &server_address, sizeof(server_address));

	// wait for the connections
	listen(server_socket, 5);
	
	// accept a connection
	int client_socket = accept(server_socket, NULL, NULL);

	// send the message
	send(client_socket, server_message, sizeof(server_message), 0);

	// close the socket
	close(server_socket);

	return 0;
}

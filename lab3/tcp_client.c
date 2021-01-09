#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main(int argc, char *argv[]) {
	const int port = 9000;

	// create a socket
	int network_socket;
	
	// int socket(int domain, int type, int protocol);
	network_socket = socket(AF_INET, SOCK_STREAM, 0);
	
	// specify an address for the socket
	struct sockaddr_in server_address;
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(port);
	server_address.sin_addr.s_addr = inet_addr("192.168.1.1");	
	
	// connecting to another socket
	int connection_status = connect(network_socket, (struct sockaddr *) &server_address, sizeof(server_address));
	
	// check for error with connection
	if (connection_status == -1) {
		printf("Error making a connection to the remote host.\n");
	}
	
	// receive data from the server
	char server_response[256];
	recv(network_socket, &server_response, sizeof(server_response), 0);
	
	// print out the server's response
	printf("Server response: %s", server_response);

	// close the socket
	close(network_socket);

	return 0;
}

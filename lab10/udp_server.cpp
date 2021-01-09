/* 
 * udpserver.c - A simple UDP echo server 
 * usage: udpserver <port>
 */

#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

using namespace std;

#define BUFSIZE 1024
#define UDPPORT 9999
#define TCPPORT 8888

/*
 * error - wrapper for perror
 */
void error(const char *msg) {
  perror(msg);
  exit(1);
}


int main(int argc, char **argv) {
	int sockfd; /* socket */
	int portno; /* port to listen on */
	socklen_t clientlen; /* byte size of client's address */
	struct sockaddr_in serveraddr; /* server's addr */
	struct sockaddr_in clientaddr; /* client addr */
	struct hostent *hostp; /* client host info */
	char buf[BUFSIZE]; /* message buf */
	char join_str[BUFSIZE];
	char *hostaddrp; /* dotted decimal host addr string */
	int optval; /* flag value for setsockopt */
	int n; /* message byte size */

  /* 
   * check command line arguments 
   */
  portno = UDPPORT;

  /* 
   * socket: create the parent socket 
   */
  sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if (sockfd < 0) 
    error("ERROR opening socket");

  /* setsockopt: Handy debugging trick that lets 
   * us rerun the server immediately after we kill it; 
   * otherwise we have to wait about 20 secs. 
   * Eliminates "ERROR on binding: Address already in use" error. 
   */
  optval = 1;
  setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval , sizeof(int));

  /*
   * build the server's Internet address
   */
  bzero((char *) &serveraddr, sizeof(serveraddr));
  serveraddr.sin_family = AF_INET;
  serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
  serveraddr.sin_port = htons((unsigned short)portno);

  /* 
   * bind: associate the parent socket with a port 
   */
  if (bind(sockfd, (struct sockaddr *) &serveraddr, 
	   sizeof(serveraddr)) < 0) 
    error("ERROR on binding");

  /* 
   * main loop: wait for a datagram, then echo it
   */
  clientlen = sizeof(clientaddr);

    /*
     * recvfrom: receive a UDP datagram from a client
     */

	int found_start = 0;
	int result;

	while (true) {
	    // n = recvfrom(sockfd, buf, BUFSIZE, 0,(struct sockaddr *) &clientaddr, &clientlen);
	    
      n = recvfrom(sockfd, &result, sizeof(int), 0,(struct sockaddr *) &clientaddr, &clientlen);
	    // n = read(sockfd, &result, sizeof(int));
	    if (n < 0) error("ERROR while reading result");

      n = recvfrom(sockfd, buf, BUFSIZE, 0,(struct sockaddr *) &clientaddr, &clientlen);
	    // n = read(sockfd, buf, BUFSIZE);
	    if (n < 0) error("ERROR while reading the IP of h1");
	    buf[n] = '\0';

	    cout << "H1 IP: " << buf << "\n";

	    /******************************** SENDING RESULTS BACK TO H1 ********************************/
	    int tcpfd;
    	struct sockaddr_in tcp_serveraddr;
    	
    	tcpfd = socket(AF_INET, SOCK_STREAM, 0);
    	if (tcpfd < 0) error("ERROR opening tcp socket");

    	/* build the server's Internet address */
	    bzero((char *) &tcp_serveraddr, sizeof(tcp_serveraddr));
	    tcp_serveraddr.sin_family = AF_INET;
	    tcp_serveraddr.sin_addr.s_addr = inet_addr(buf);
	    tcp_serveraddr.sin_port = htons(TCPPORT);

	    /* connect: create a connection with the server */
	    if (connect(tcpfd, (const sockaddr*)&tcp_serveraddr, sizeof(tcp_serveraddr)) < 0) 
	    	error("ERROR connecting to TCP server");

	    /* send the message line to the server */
      result *= result;
    	n = write(tcpfd, &result, sizeof(int));
    	if (n < 0) error("ERROR writing to TCP socket");

    	close(tcpfd);
   	}
}


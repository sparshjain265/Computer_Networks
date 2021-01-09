/* 
* tcp_add_server.cpp : takes two number as input and perform add
* usage : tcp_add_server
* PORT : 12500
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

#define PORT 12502
#define UDP_PORT 9999
#define QUEUE_SIZE 5

void error(const char *msg) {
    perror(msg);
    exit(1);
}

struct packet {
    int a;
    int b;
};

int main(int argc, char *argv[]) {
    int parentfd;   /* parent socket */
    int childfd;    /* child socket */
    int udpfd;      /* udp socket */
    int portno;     /* port to listen on */
    socklen_t clientlen;  /* byte size of client's address */

    struct sockaddr_in serveraddr;  /* server's addr */
    struct sockaddr_in clientaddr;  /* client addr */
    struct hostent *hostp;          /* client host info */
    
    char buf[BUFSIZE];  /* message buffer */
    char *hostaddrp;    /* dotted decimal host addr string */
    int optval;         /* flag value for setsockopt */
    int n;              /* message byte size */

    portno = PORT;

    parentfd = socket(AF_INET, SOCK_STREAM, 0);
    if (parentfd < 0) error("ERROR opening socket");

    /* setsockopt: Handy debugging trick that lets 
    * us rerun the server immediately after we kill it; 
    * otherwise we have to wait about 20 secs. 
    * Eliminates "ERROR on binding: Address already in use" error. 
    */
    optval = 1;
    setsockopt(parentfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval , sizeof(int));

    /*
    * build the server's Internet address
    */
    bzero((char *) &serveraddr, sizeof(serveraddr));

    /* this is an Internet address */
    serveraddr.sin_family = AF_INET;

    /* let the system figure out our IP address */
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);

    /* this is the port we will listen on */
    serveraddr.sin_port = htons((unsigned short)portno);

    /* 
    * bind: associate the parent socket with a port 
    */
    if (bind(parentfd, (struct sockaddr *) &serveraddr, sizeof(serveraddr)) < 0){
        error("ERROR on binding");
    }

    
    /* 
    * listen: make this socket ready to accept connection requests 
    */
    if (listen(parentfd, QUEUE_SIZE) < 0) /* allow QUEUE_SIZE requests to queue up */ 
        error("ERROR on listen");

    /* 
    * main loop: wait for a connection request, echo input line, 
    * then close connection.
    */
    clientlen = sizeof(clientaddr);
    while (true) {
        /* 
        * accept: wait for a connection request 
        */
        childfd = accept(parentfd, (struct sockaddr *)&clientaddr, &clientlen);
        if (childfd < 0) error("ERROR on accept");
    

        packet pkt;

        /* 
         * read: read input data from the client
         */
        // bzero(&pkt, BUFSIZE);
        n = read(childfd, &pkt.a, sizeof(int));
        // n = recvfrom(childfd, &pkt, sizeof(packet), 0, (struct sockaddr *)&clientaddr, &clientlen);
        if (n < 0) error("ERROR reading from socket");

        n = read(childfd, &pkt.b, sizeof(int));
        if (n < 0) error("ERROR reading from socket");

        cout << "Server received : " << pkt.a << " & " << pkt.b << "\n";

        /* 
         * write: echo the input string back to the client 
         */
        //  n = write(childfd, buf, strlen(buf));

        int result = pkt.a * pkt.b;

        cout << "result : " << result << endl;

        // sending to host h3
        // n = sendto(childfd, &result, sizeof(int), 0, (struct sockaddr *)&udpaddr, udplen);
        // if (n < 0) error("ERROR writing to socket");

        close(childfd);

        /**************************** Sending to the UDP on h3 ****************************/

        string ip_addr_of_h3 = "192.168.1.4";

        struct sockaddr_in udp_serveraddr;  /* udp server's addr */
        bzero((char *) &udp_serveraddr, sizeof(udp_serveraddr));

        udp_serveraddr.sin_family = AF_INET;
        udp_serveraddr.sin_addr.s_addr = inet_addr(ip_addr_of_h3.c_str());
        udp_serveraddr.sin_port = htons(UDP_PORT);

        udpfd = socket(AF_INET, SOCK_DGRAM, 0);
        if (udpfd < 0) error("ERROR opening socket for UDP");

        /* send the message to the h3 */
        int serverlen = sizeof(udp_serveraddr);
        n = sendto(udpfd, &result, sizeof(result), 0, (struct sockaddr *)&udp_serveraddr, serverlen);

        // n = write(udpfd, &result, sizeof(result));
        if (n < 0) error("ERROR in writing results to UDP");


        strcpy(buf, inet_ntoa(clientaddr.sin_addr));   /* client IP */

        n = sendto(udpfd, buf, strlen(buf), 0, (struct sockaddr *)&udp_serveraddr, serverlen);
        cout << "Sending IP of h1 : " << buf << endl;
        // n = write(udpfd, &result, strlen(buf));
        if (n < 0) error("ERROR in writing client to UDP");

        close(udpfd);
    }

    return 0;
}
/* 
* tcp_h1_server.cpp : takes two number as input and an operation
* usage : tcp_add_server <a> <b> <c>
* h2_PORT : depending on operation
* h1_PORT : 8888
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

#define H1_TCP_PORT 8888
#define QUEUE_SIZE 5

void error(const char *msg) {
    perror(msg);
    exit(1);
}

struct packet {
    int a;
    int b;
    packet(int _a, int _b) : a{_a}, b{_b} {}
};


int op_to_port(string op) {
    if (op == "ADD") {
        return 12500;
    } else if (op == "SUB") {
        return 12501;
    } else if (op == "MUL") {
        return 12502;
    } else if (op == "IDIV") {
        return 12503;       
    } else return -1;
}

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

    if (argc != 4) {
        error("Args: <a> <b> <op>\n");
    }

    portno = op_to_port(argv[3]);
    if (portno == -1) {
        error("Unsupported operation.\n");
    }

    int a = atoi(argv[1]);
    int b = atoi(argv[2]);

    /****************** sending to h1 *******************/
    int tcpfd;
    struct sockaddr_in tcp_serveraddr;
    
    tcpfd = socket(AF_INET, SOCK_STREAM, 0);
    if (tcpfd < 0) error("ERROR opening tcp socket");

    /* build the server's Internet address */
    bzero((char *) &tcp_serveraddr, sizeof(tcp_serveraddr));
    tcp_serveraddr.sin_family = AF_INET;
    tcp_serveraddr.sin_addr.s_addr = inet_addr("192.168.1.3");
    tcp_serveraddr.sin_port = htons(portno);

    /* connect: create a connection with the server */
    if (connect(tcpfd, (const sockaddr*)&tcp_serveraddr, sizeof(tcp_serveraddr)) < 0) 
        error("ERROR connecting to TCP server");

    /* send the message line to the server */
    n = write(tcpfd, &a, sizeof(int));
    if (n < 0) error("ERROR writing to TCP socket");

    n = write(tcpfd, &b, sizeof(int));
    if (n < 0) error("ERROR writing to TCP socket");

    close(tcpfd);

    /****************** listening from h3 ****************/

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
    serveraddr.sin_port = htons(H1_TCP_PORT);

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

    int t = 1;

    while (1) {
        /* 
        * accept: wait for a connection request 
        */
        childfd = accept(parentfd, (struct sockaddr *)&clientaddr, &clientlen);
        if (childfd < 0) error("ERROR on accept");
    

        int result;

        /* 
         * read: read input data from the client
         */
        n = read(childfd, &result, sizeof(int));

        if (n < 0) error("ERROR reading from socket");
        cout << "Result of computation : " << result << "\n";


        close(childfd);
    }

    return 0;
}
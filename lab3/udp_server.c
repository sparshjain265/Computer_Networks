/* 
 * udpserver.c - A simple UDP echo server 
 * usage: udpserver <port>
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUFSIZE 2048

const char flag[] = "01111110";

char* correct_data_code(char *str) {
	int len = strlen(str);
	char *data = (char*) malloc (sizeof(char) * 5);

	int flag = 0;

	printf("received string: %s\n" , str);
	flag |= (((str[0] == '1') ^ (str[2] == '1') ^ (str[4] == '1') ^ (str[6] == '1')) ? 1 : 0);
	flag |= (((str[0] == '1') ^ (str[1] == '1') ^ (str[4] == '1') ^ (str[5] == '1')) ? 1 : 0) << 1;
	flag |= (((str[0] == '1') ^ (str[1] == '1') ^ (str[2] == '1') ^ (str[3] == '1')) ? 1 : 0) << 2;
	
	printf("error in bit : %d\n", 7-flag);

	str[7-flag] = (str[7-flag] == '1')?'0':'1';
	
	data[0] = str[0];
	data[1] = str[1];
	data[2] = str[2];
	data[3] = str[4];
	data[4] = '\0';

	printf("data: %s\n\n", data);

	return data;
}

void print_original_data(char *str){
	int len = strlen(str);
	
	char ch = 0;
	
	printf("original data: ");

	for (int i=0; i < len; i+=8) {
		ch = 0;
		char *bp = &str[i];
		for (int j = 0; j < 8; j++) {
			ch |= (bp[j] == '1') << (7 - j);
		}

		printf("%c", ch);
	}

	printf("\n\n");
}

void reverse_bitstuffing(char *str) {
	int len = strlen(str);

	int count = 0;	
	for (int i = 0; i < len; i++) {
		count = 0;
		// count for five ones
		for (int j = i; j < (i+5); j++) {
			if (str[j] == '1') {
				++count;				
			}
		}

		// remove stuffed bits
		if (count == 5) {
			for (int k = i+5; k < len; k++) {
				str[k] = str[k+1];		
			}
			i = i + 4;
		}
	}
}

int check_end(char *str) {
	int len = strlen(str);
	char *end = strstr(str, flag);
	if (strstr(str, flag)){
		int pos = end - str;
		str[pos] = '\0';
		printf("bitstr with stuffing : %s\n", str); // recieved string
		reverse_bitstuffing(str);		
		printf("== original bits => %s\n== length => %d\n", str, strlen(str)); // recieved string
		print_original_data(str);
		return 1;
	}
	return 0;
}

/*
 * error - wrapper for perror
 */
void error(char *msg) {
  perror(msg);
  exit(1);
}

int main(int argc, char **argv) {
  int sockfd; /* socket */
  int portno; /* port to listen on */
  int clientlen; /* byte size of client's address */
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
  if (argc != 2) {
    fprintf(stderr, "usage: %s <port>\n", argv[0]);
    exit(1);
  }
  portno = atoi(argv[1]);

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
  setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, 
	     (const void *)&optval , sizeof(int));

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
	while (1) {
		if(check_end(join_str)) {
			join_str[0] = '\0';
			printf("\nclearing buffer starting for new data...\n\n");
		}

	    bzero(buf, BUFSIZE);
	    n = recvfrom(sockfd, buf, BUFSIZE, 0,
			 (struct sockaddr *) &clientaddr, &clientlen);
	    if (n < 0)
	      error("ERROR in recvfrom");

		 buf[n] = '\0';
		char *data = correct_data_code(buf);
		strcat(join_str, data);
			
		printf("joined data: %s\n", join_str);

		
		free(data);
   	}
}


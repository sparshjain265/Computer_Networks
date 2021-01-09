/* 
 * udpclient.c - A simple UDP client
 * usage: udpclient <host> <port>
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

#define BUFSIZE 2048

#define DATA_LEN (4)
#define WIDTH (8)
#define HAM_WIDTH (7)

const char flag[] = "01111110";

void get_bits(char c, char *str) {
	int n = c, i=0;
	int mask = 1 << 7;

	while (mask) {
		*str = (n & mask)?'1':'0';
		++str;
		mask = mask>>1;
	}
}


char** get_hammed(char *str, int *num_frames) {
	int len = strlen(str);
	char **ham = (char**) malloc (sizeof(char*) * (len/4));
	int j = 0;
	for (int i = 0; i < len; i += DATA_LEN, j++) {
		ham[j] = (char*) malloc(sizeof(char) * (HAM_WIDTH+1));

		strcpy(ham[j], "0000000");

		ham[j][0] = str[i + 0];
		ham[j][1] = str[i + 1];
		ham[j][2] = str[i + 2];
		ham[j][4] = str[i + 3];
		
		printf("data : %c %c %c %c \n",str[i + 0], str[i + 1], str[i + 2], str[i + 3]);

		ham[j][6] = ((ham[j][0] == '1') ^ (ham[j][2] == '1') ^ (ham[j][4] == '1')) ? '1' : '0';
		ham[j][5] = ((ham[j][0] == '1') ^ (ham[j][1] == '1') ^ (ham[j][4] == '1')) ? '1' : '0';
		ham[j][3] = ((ham[j][0] == '1') ^ (ham[j][1] == '1') ^ (ham[j][2] == '1')) ? '1' : '0';
		
		ham[j][7] = '\0';
		printf("%s\n", ham[j]);
	}
	*num_frames = j;
	return ham;
}



char* bit_stuff(char *str){
	char *stuffed = (char*) malloc(sizeof(char) * strlen(str) * WIDTH * 2);

	//strcpy(stuffed, flag);

	int count, nl;
	for (int i = 0; i < strlen(str); i++) {
		count=0;
		
		//the following code search the five ones in given string
		for (int j = i; j < (i+5); j++) {
			if(str[j] == '1') {
				count++;
			}
		}

		//if there is five ones then folling code execute to bit stuffing after five ones
		if(count == 5) {
			nl = strlen(str) + 1;
			for ( ; nl > (i+5); nl--) {
				str[nl]=str[nl-1];
			}
			str[i+5]='0';
			i=i+5;
		}
	}
	

	strcat(stuffed, str);
	strcat(stuffed, flag);


	printf("bit stuffed str: %s\n\n", str);

	nl = strlen(stuffed);
	int rem = (nl % 4 != 0) * 4 - (nl % 4);

	while (rem--) {
		strcat(stuffed, "0");
	}
		nl = strlen(stuffed);
	
	printf("bit stuffed with padding with zeros: %s\n\n, strlen : %d \n", stuffed, nl);

	free(str);

	return stuffed;
}


/* 
 * error - wrapper for perror
 */
void error(char *msg) {
    perror(msg);
    exit(0);
}

int main(int argc, char **argv) {
    int sockfd, portno, n;
    int serverlen;
    struct sockaddr_in serveraddr;
    char *server;
    char *hostname;
    char buf[BUFSIZE];

    /* check command line arguments */
    if (argc != 3) {
       fprintf(stderr,"usage: %s <hostname> <port>\n", argv[0]);
       exit(0);
    }
    // hostname = argv[1];
    server = argv[1];
    portno = atoi(argv[2]);

    /* socket: create the socket */
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");

    /* gethostbyname: get the server's DNS entry */
    /*server = gethostbyname(hostname);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host as %s\n", hostname);
        exit(0);
    }*/

    /* build the server's Internet address */
    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    
    serveraddr.sin_addr.s_addr = inet_addr(server);

    serveraddr.sin_port = htons(portno);

	while (1) {

					    /* get a message from the user */
		    bzero(buf, BUFSIZE);

		    printf("Please enter msg: ");
		    fgets(buf, BUFSIZE, stdin);

			
			char len = strlen(buf);

			int bit_str_len = 0;
			char *bit_str = (char*) malloc(sizeof(char) * len * WIDTH * 5);

			for (int i = 0; i < len; i++) {
				get_bits(buf[i], &bit_str[i*WIDTH]);
				bit_str[(i+1)*WIDTH] = '\0';
			}

			bit_str_len = strlen(bit_str);
			printf("bitstr: \n%s\n%d\n", bit_str, bit_str_len);
			bit_str = bit_stuff(bit_str);
			printf("bit str updated : %s\n\n", bit_str);

			int num_frames = 0;
			char** ham = get_hammed(bit_str, &num_frames);


			for (int i=0; i < num_frames; i++) {
				
				/* send the message to the server */
			    serverlen = sizeof(serveraddr);
			    n = sendto(sockfd, ham[i], strlen(ham[i]), 0, (struct sockaddr *)&serveraddr, serverlen);
			    if (n < 0) 
			      error("ERROR in sendto");


			}

			
			for (int i=0; i< num_frames; i++) {
				free(ham[i]);
			}

			free(ham);

			free(bit_str);

	}



    
    /* print the server's reply */
//    n = recvfrom(sockfd, buf, strlen(buf), 0, (struct sockaddr *)&serveraddr, &serverlen);
/*    if (n < 0) 
      error("ERROR in recvfrom");
    printf("Echo from server: %s", buf);*/





    return 0;
}


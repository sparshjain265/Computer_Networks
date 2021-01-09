#include <iostream>
#include<stdio.h>
#include <unistd.h>
#include<malloc.h>
#include<string.h>
#include<signal.h>
#include<stdbool.h>
#include<sys/socket.h>
#include<sys/types.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include<linux/if.h>
#include<linux/if_packet.h>
#include<netinet/in.h>		 
#include<netinet/if_ether.h>      // for ethernet header
#include<netinet/ip.h>		      // for ip header
#include<netinet/udp.h>		      // for udp header
#include<netinet/tcp.h>
#include<arpa/inet.h>             // to avoid warning at inet_ntoa

#define BUFF_MAX_SIZE 65536

using namespace std;

void error(const char *msg) {
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[]) {
	int raw_socket;
	int n;
	struct sockaddr saddr;
	int saddr_len = sizeof(saddr);
	unsigned char *buff = (unsigned char *) malloc(BUFF_MAX_SIZE);

	// htons(ETH_P_ALL)

	raw_socket = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_IP));
	if (raw_socket < 0) error("Failed to create socket\n");

	int t = 0;


	sockaddr_ll sll;
    ifreq ifr; 

    bzero(&sll , sizeof(sll));
    bzero(&ifr , sizeof(ifr));
     
    strncpy((char *)ifr.ifr_name ,"eth1" , IFNAMSIZ); 
    
    //copy device name to ifr 
    if((ioctl(raw_socket , SIOCGIFINDEX , &ifr)) == -1) error("Unable to find interface index");
    sll.sll_family = AF_PACKET; 
    sll.sll_ifindex = ifr.ifr_ifindex; 
    sll.sll_protocol = htons(ETH_P_IP);

    if((bind(raw_socket, (struct sockaddr *)&sll , sizeof(sll))) == -1) error("bind: ");
        
	while (true) {
		
		n = recvfrom(raw_socket, buff, BUFF_MAX_SIZE, 0, NULL, NULL);
		if (n < 0) error("Failed to get packets\n");

		/*extracting ethernet header*/
		/*
			struct ethhdr {
				unsigned char	h_dest[ETH_ALEN];
				unsigned char	h_source[ETH_ALEN];
				__be16		h_proto;
			} __attribute__((packed));
		*/


		ethhdr *eth = (ethhdr *) buff;

		/* for IP packets proto is 0x800 ETH_P_IP */

		/*
			struct iphdr {
			#if defined(__LITTLE_ENDIAN_BITFIELD)
				__u8	ihl:4,
					version:4;
			#elif defined (__BIG_ENDIAN_BITFIELD)
				__u8	version:4,
			  		ihl:4;
			#else
			#error	"Please fix <asm/byteorder.h>"
			#endif
				__u8	tos;
				__u16	tot_len;
				__u16	id;
				__u16	frag_off;
				__u8	ttl;
				__u8	protocol;
				__u16	check;
				__u32	saddr;
				__u32	daddr;
			};
		*/

		/* extracting IP header */
		unsigned short iphdrlen;
		
		iphdr *ip = (struct iphdr*)(buff + sizeof(struct ethhdr));
		
		sockaddr_in source;
    	
    	sockaddr_in dest;
		
		memset(&source, 0, sizeof(source));
		source.sin_addr.s_addr = ip->saddr;
		
		memset(&dest, 0, sizeof(dest));
		dest.sin_addr.s_addr = ip->daddr;

		// printf("\t|-Version : %d\n",(unsigned int)ip->version);
		// printf("\t|-Internet Header Length : %d DWORDS or %d Bytes\n",(unsigned int)ip->ihl,((unsigned int)(ip->ihl))*4);
		// printf("\t|-Type Of Service : %d\n",(unsigned int)ip->tos);
		// printf("\t|-Total Length : %d Bytes\n",ntohs(ip->tot_len));
		// printf("\t|-Identification : %d\n",ntohs(ip->id));
		// printf("\t|-Time To Live : %d\n",(unsigned int)ip->ttl);
		// printf("\t|-Protocol : %d\n",(unsigned int)ip->protocol);
		// printf("\t|-Header Checksum : %d\n",ntohs(ip->check));

		/* printing details */

		string temp(inet_ntoa(source.sin_addr));
		// cout << "temp[1] : " << temp[1] << " temp[2] : " << temp[2] << endl;

		// if (temp[1] != '0' and temp[2] == '.'){
			printf("t : %d\n", ++t);
			printf("\nEthernet Header\n");
			printf("\t|-Source Address : %.2X-%.2X-%.2X-%.2X-%.2X-%.2X\n",eth->h_source[0], eth->h_source[1], eth->h_source[2], eth->h_source[3], eth->h_source[4], eth->h_source[5]);
			printf("\t|-Destination Address : %.2X-%.2X-%.2X-%.2X-%.2X-%.2X\n",eth->h_dest[0], eth->h_dest[1], eth->h_dest[2], eth->h_dest[3], eth->h_dest[4], eth->h_dest[5]);
			printf("\t|-Protocol : %d\n",eth->h_proto);
			printf("\t|-Source IP : %s\n", inet_ntoa(source.sin_addr));
			printf("\t|-Destination IP : %s\n",inet_ntoa(dest.sin_addr));
		// }
	}

	close(raw_socket);

	return 0;
}
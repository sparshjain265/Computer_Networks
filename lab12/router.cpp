#include <bits/stdc++.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <linux/if_packet.h>
#include <netinet/in.h>
#include <netinet/if_ether.h> // for ethernet header
#include <netinet/ip.h>       // for ip header
#include <netinet/udp.h>      // for udp header
#include <netinet/tcp.h>
#include <arpa/inet.h>

#define R1_ETH1_IP "192.168.20.1"
#define R1_ETH2_IP "192.168.30.1"
#define SSH_IP_1 "10.0.2.2"
#define SSH_IP_2 "10.0.2.15"

using namespace std;

uint16_t CURRENT_PORT = 10000;

struct sockaddr_in source_ip, dest_ip;
struct sockaddr_in modified_source_ip, modified_dest_ip;
uint16_t source_port, dest_port;
uint16_t modified_source_port, modified_dest_port;

unsigned int ip_header_length;
unsigned int transport_header_length;
unsigned int total_length;
bool packet_from_home = false;
int total_packet = 0;

map<pair<string, uint16_t>, uint16_t> NAT;
map<uint16_t, pair<string, uint16_t>> NAT_reverse;

const char *stringToCstr(string str)
{
    char *cstr = new char[str.length() + 1];
    for (int i = 0; i < str.length(); i++)
    {
        cstr[i] = str[i];
    }
    cstr[str.length()] = '\0';
    const char *ans = cstr;
    return ans;
}

void UDPChecksum(struct iphdr *pIph, unsigned short *ipPayload)
{
    register unsigned long sum = 0;
    struct udphdr *udphdrp = (struct udphdr *)(ipPayload);
    unsigned short udpLen = htons(udphdrp->len);
    sum += (pIph->saddr >> 16) & 0xFFFF;
    sum += (pIph->saddr) & 0xFFFF;
    //the dest ip
    sum += (pIph->daddr >> 16) & 0xFFFF;
    sum += (pIph->daddr) & 0xFFFF;
    //protocol and reserved: 17
    sum += htons(IPPROTO_UDP);
    //the length
    sum += udphdrp->len;

    udphdrp->check = 0;
    while (udpLen > 1)
    {
        sum += *ipPayload++;
        udpLen -= 2;
    }
    if (udpLen > 0)
    {
        sum += ((*ipPayload) & htons(0xFF00));
    }
    while (sum >> 16)
    {
        sum = (sum & 0xffff) + (sum >> 16);
    }
    sum = ~sum;

    udphdrp->check = ((unsigned short)sum == 0x0000) ? 0xFFFF : (unsigned short)sum;
}

void getAddress(unsigned char *buffer, int buflen)
{

    //find the ip address
    struct iphdr *ip_header = (struct iphdr *)(buffer);
    memset(&source_ip, 0, sizeof(source_ip));
    source_ip.sin_addr.s_addr = ip_header->saddr;
    memset(&dest_ip, 0, sizeof(dest_ip));
    dest_ip.sin_addr.s_addr = ip_header->daddr;

    //find the header length
    ip_header_length = ip_header->ihl * 4;

    struct udphdr *udp = (struct udphdr *)(buffer + ip_header_length);
    source_port = htons(udp->source);
    dest_port = htons(udp->dest);

    if (strcmp(R1_ETH1_IP, inet_ntoa(dest_ip.sin_addr)) != 0 && strcmp(R1_ETH2_IP, inet_ntoa(dest_ip.sin_addr)) != 0)
    {
        packet_from_home = true;
    }
    //packet is coming from the outside world
    else if (strcmp(R1_ETH2_IP, inet_ntoa(dest_ip.sin_addr)) == 0)
    {
        packet_from_home = false;
    }
}

void changeAddress()
{
    if (packet_from_home)
    {
        modified_source_ip.sin_addr.s_addr = inet_addr(R1_ETH2_IP);
        modified_dest_ip.sin_addr.s_addr = dest_ip.sin_addr.s_addr;

        string source_add = inet_ntoa(source_ip.sin_addr);
        if (NAT.find(make_pair(source_add, source_port)) == NAT.end())
        {
            NAT[make_pair(source_add, source_port)] = CURRENT_PORT;
            NAT_reverse[CURRENT_PORT] = make_pair(source_add, source_port);
            CURRENT_PORT++;
        }

        modified_source_port = NAT[make_pair(source_add, source_port)];
        modified_dest_port = dest_port;
    }
    else
    {
        pair<string, uint16_t> p = NAT_reverse[dest_port];

        modified_source_ip.sin_addr.s_addr = source_ip.sin_addr.s_addr;
        modified_dest_ip.sin_addr.s_addr = inet_addr(stringToCstr(p.first));

        modified_source_port = source_port;
        modified_dest_port = p.second;
    }
}

unsigned short checksum(unsigned short *buff, int _16bitword)
{
    unsigned long sum;
    for (sum = 0; _16bitword > 0; _16bitword--)
        sum += (*(buff)++);
    do
    {
        sum = ((sum >> 16) + (sum & 0xFFFF));
    } while (sum & 0xFFFF0000);

    return (~sum);
}

int ip_header(unsigned char *buffer, unsigned char *sending_buffer, int buflen)
{
    struct iphdr *ip = (struct iphdr *)(buffer);
    struct iphdr *send_ip = (struct iphdr *)(sending_buffer);

    ip_header_length = ip->ihl * 4;

    if (strcmp(SSH_IP_1, inet_ntoa(source_ip.sin_addr)) == 0 || strcmp(SSH_IP_2, inet_ntoa(source_ip.sin_addr)) == 0)
        return -1;

    printf("******************************\n");
    printf("Total packets %d\n", total_packet);
    if (packet_from_home)
    {
        printf("Packet is from home\n");
    }
    else
    {
        printf("Packet is not from home\n");
    }
    printf("Source address %s\n", inet_ntoa(source_ip.sin_addr));
    printf("Destination Address %s\n", inet_ntoa(dest_ip.sin_addr));
    printf("Modified Source address %s\n", inet_ntoa(modified_source_ip.sin_addr));
    printf("Modified Destination Address %s\n", inet_ntoa(modified_dest_ip.sin_addr));

    printf("Source port %d\n", source_port);
    printf("destination port %d\n", dest_port);
    printf("Modified Source port %d\n", modified_source_port);
    printf("Modified destination port %d\n", modified_dest_port);
    printf("******************************\n");
    send_ip->ihl = ip->ihl;
    send_ip->version = ip->version;
    send_ip->tos = ip->tos;
    send_ip->tot_len = ip->tot_len;
    send_ip->id = ip->id;
    send_ip->ttl = ip->ttl;
    send_ip->protocol = ip->protocol;
    send_ip->saddr = inet_addr(inet_ntoa(modified_source_ip.sin_addr));
    send_ip->daddr = inet_addr(inet_ntoa(modified_dest_ip.sin_addr));
    send_ip->check = 0;

    //copy the options field
    unsigned char *send_option = (unsigned char *)(sending_buffer + 20);
    unsigned char *option = (unsigned char *)(buffer + 20);
    memcpy(send_option, option, ip_header_length - 20);

    total_length = ntohs(ip->tot_len);
    send_ip->check = (checksum((unsigned short *)(sending_buffer), ip_header_length / 2));
    return 0;
}

int udp_header(unsigned char *buffer, unsigned char *sending_buffer, int buflen)
{
    if (ip_header(buffer, sending_buffer, buflen) == -1)
        return -1;

    struct udphdr *udp = (struct udphdr *)(buffer + ip_header_length);
    struct udphdr *send_udp = (struct udphdr *)(sending_buffer + ip_header_length);

    transport_header_length = 8;
    send_udp->source = htons(modified_source_port);
    send_udp->dest = htons(modified_dest_port);
    send_udp->len = udp->len;

    cout << "Source Port : " << htons(modified_source_port) << endl;
    cout << "Destination Port : " << htons(modified_dest_port) << endl;
    // payload(buffer, sending_buffer, buflen);

    // copy data
    memcpy(sending_buffer + ip_header_length + transport_header_length,
           buffer + ip_header_length + transport_header_length,
           total_length - ip_header_length - transport_header_length);

    struct iphdr *send_ip = (struct iphdr *)(sending_buffer);
    unsigned short *ip_pay_load = (unsigned short *)(sending_buffer + ip_header_length);
    UDPChecksum(send_ip, ip_pay_load);

    return 0;
}

int processData(unsigned char *buffer, unsigned char *sending_buffer, int buflen)
{
    struct iphdr *ip = (struct iphdr *)(buffer);
    getAddress(buffer, buflen);
    changeAddress();
    total_packet++;
    switch (ip->protocol)
    {
    case 17:
        return udp_header(buffer, sending_buffer, buflen);
        break;
    default:
        printf("Error: Protocol Not Supported!\n");
        break;
    }
    return -1;
}

int main()
{
    int sock_r, saddr_len, buflen;
    unsigned char *buffer = (unsigned char *)malloc(65536);
    unsigned char *sending_buffer = (unsigned char *)malloc(65536);
    memset(buffer, 0, 65536);
    printf("starting .... \n");

    sock_r = socket(AF_PACKET, SOCK_DGRAM, htons(ETH_P_IP));
    if (sock_r < 0)
    {
        printf("error in socket: %d\n", sock_r);
        return -1;
    }

    //for sending the packet use this
    int sock_send = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
    if (sock_send < 0)
    {
        printf("Error in opening the sending socket\n");
        return 0;
    }

    while (1)
    {
        sockaddr saddr;
        saddr_len = sizeof saddr;
        buflen = recvfrom(sock_r, buffer, 65536, 0, &saddr, (socklen_t *)&saddr_len);

        if (buflen < 0)
        {
            printf("error in reading recvfrom function\n");
            return -1;
        }
        if (processData(buffer, sending_buffer, buflen) == 0)
        {
            printf("Packet received\n");
            modified_dest_ip.sin_port = htons(modified_dest_port);
            modified_dest_ip.sin_family = AF_INET;
            if (sendto(sock_send, sending_buffer, total_length, 0,
                       (struct sockaddr *)&modified_dest_ip, sizeof(modified_dest_ip)) < 0)
            {
                perror("Error in sending");
                return 0;
            }
            printf("Packet sent\n");
        }
    }

    close(sock_r);
    close(sock_send);
    printf("DONE!!!!\n");
}

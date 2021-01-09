// udp client driver program
#include <stdio.h>
#include <strings.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <iostream>
#include <math.h>
#include <bitset>
#include <time.h>

#define PORT 8567

using namespace std;

// function to convert a char to 8 bit binary
string charToBin(char c)
{
	string s;
	for (int i = 7; i >= 0; --i)
	{
		s.push_back((c & (1 << i)) ? '1' : '0');
	}
	return s;
}

// function to convert a string to 8 bit binary
string strToBin(string msg)
{
	string s;
	for (int i = 0; i < msg.size(); i++)
	{
		// s += charToBin(msg[i]);
		s += bitset<8>(msg[i]).to_string();
	}
	return s;
}

// Driver code
int main(int argc, char *argv[])
{
	if (argc < 3)
	{
		printf("\n Error : Not sufficient arguments!");
		exit(0);
	}

	//original msg
	string msg = argv[1];
	//message bits allowed
	int m = atoi(argv[2]);
	//message bits we will send
	int k = m - 1;
	//number of parity bits
	int r = 0;
	while (pow(2, r) < (m + r + 1))
	{
		r++;
	}
	//message converted to binary
	string message = strToBin(msg);
	// cout << msg << endl;
	// cout << message << endl;
	//length of binary message
	int n = message.size();
	//we extract 'k' bits, add a flag bit to indicate whether more
	for (int i = 0; i < n; i += k)
	{
		// extract k bits
		string temp = message.substr(i, k);
		// if less than k bits, append with 0
		while (temp.size() < k)
		{
			temp += "0";
		}
		// append flag bit
		if (i + k < n)
		{
			temp += "1";
		}
		else
		{
			temp += "0";
		}
		// temp is now of length 'm'
		// message as int bits for xor computation
		int temp_int[m];
		for (int j = 0; j < m; j++)
		{
			if (temp[j] == '0')
				temp_int[j] = 0;
			else
				temp_int[j] = 1;
		}
		// encoded message as int bits after xor computation
		int hamming[m + r];
		// place message bits in appropriate positions
		// currently parity bits are set to 0
		int p = 0, q = 0;
		for (int j = 0; j < m + r; j++)
		{
			if (j + 1 == pow(2, p))
			{
				hamming[j] = 0;
				p++;
			}
			else
			{
				hamming[j] = temp_int[q];
				q++;
			}
		}
		// for (int j = 0; j < m + r; j++)
		// 	cout << hamming[j];
		// cout << endl;
		// calculate parity bits
		p = 0;
		for (int j = 1; j <= m + r; j = pow(2, p))
		{
			int parity = 0;
			for (q = 0; q < m + r; q++)
			{
				// if pth bit at qth position is set, update parity
				string pos = bitset<11>(q + 1).to_string();
				if (pos[11 - (p + 1)] == '1')
				{
					parity = parity ^ hamming[q];
				}
			}
			hamming[j - 1] = parity;
			p++;
		}
		// convert into string
		string hammingMsg;
		for (int j = 0; j < m + r; j++)
		{
			if (hamming[j] == 0)
			{
				hammingMsg += "0";
			}
			else
			{
				hammingMsg += "1";
			}
		}
		// cout << temp << endl;
		// cout << hammingMsg << endl;

		// int MAXLINE = m + r;
		int sockfd, n;
		struct sockaddr_in servaddr;

		// clear servaddr
		bzero(&servaddr, sizeof(servaddr));
		servaddr.sin_addr.s_addr = inet_addr("192.168.1.2");
		servaddr.sin_port = htons(PORT);
		servaddr.sin_family = AF_INET;

		// create datagram socket
		sockfd = socket(AF_INET, SOCK_DGRAM, 0);

		// connect to server
		if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
		{
			printf("\n Error : Connect Failed \n");
			exit(0);
		}

		// request to send datagram
		// no need to specify server address in sendto
		// connect stores the peers IP and port
		sendto(sockfd, hammingMsg.c_str(), hammingMsg.size(), 0, (struct sockaddr *)NULL, sizeof(servaddr));

		close(sockfd);

		// sleep for 100 ms
		usleep(100 * 1000);
	}
	return 0;
}

// server program for udp connection
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
#define PORT 9567

using namespace std;

// Driver code
int main()
{
    string binMsg;
    while (true)
    {
        char buffer[1024];
        int listenfd, len;
        struct sockaddr_in servaddr, cliaddr;
        bzero(&servaddr, sizeof(servaddr));

        // Create a UDP Socket
        listenfd = socket(AF_INET, SOCK_DGRAM, 0);
        servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
        servaddr.sin_port = htons(PORT);
        servaddr.sin_family = AF_INET;

        // bind server address to socket descriptor
        bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr));

        //receive the datagram
        len = sizeof(cliaddr);
        int n = recvfrom(listenfd, buffer, sizeof(buffer),
                         0, (struct sockaddr *)&cliaddr, (socklen_t *)&len); //receive message from server
        buffer[n] = '\0';
        // puts(buffer);

        //find number of parity bits
        int r = 0;
        while (pow(2, r) < n + 1)
        {
            r++;
        }
        // find parity
        int parity[r];
        for (int i = 0; i < r; i++)
        {
            int p = 0;
            for (int q = 0; q < n; q++)
            {
                string pos = bitset<11>(q + 1).to_string();
                if (pos[11 - (i + 1)] == '1')
                {
                    if (buffer[q] == '1')
                    {
                        p = p ^ 1;
                    }
                    else
                    {
                        p = p ^ 0;
                    }
                }
            }
            parity[i] = p;
        }
        // find the position of incorrect bit
        int incorrectBit = 0;
        for (int i = 0; i < r; i++)
        {
            incorrectBit += parity[i] * pow(2, i);
        }
        // correct the incorrect bit
        if (incorrectBit > 0)
        {
            if (buffer[incorrectBit - 1] == '1')
            {
                buffer[incorrectBit - 1] = '0';
            }
            else
            {
                buffer[incorrectBit - 1] = '1';
            }
        }
        // extract msg
        string message;
        int p = 0;
        for (int i = 0; i < n - 1; i++)
        {
            if (i + 1 == pow(2, p))
            {
                p++;
            }
            else
            {
                if (buffer[i] == '0')
                {
                    message += "0";
                }
                else
                {
                    message += "1";
                }
            }
        }
        binMsg += message;
        // break if end
        if (buffer[n - 1] == '0')
        {
            break;
        }
    }
    // we have message in binary form
    // find out length
    int len = binMsg.size() / 8;
    // decode msg
    char message[len + 1];
    for (int i = 0; i < len; i += 8)
    {
        string temp = binMsg.substr(i, 8);
        message[i] = stoi(temp, 0, 2);
    }
    message[len] = '\0';
    string msg = message;
    cout << msg << endl;
}

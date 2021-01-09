#include <bits/stdc++.h>
#include <string>
#include <iostream>
#include <dirent.h>
#include <sys/types.h>
#include <fstream>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <linux/ip.h>
#include <linux/tcp.h>

using namespace std;

bool computeChecksum(const char *filename, int len)
{
    FILE *f;
    f = fopen(filename, "rb");
    unsigned char buffer[len];
    fread(buffer, sizeof(char), len, f);
    fclose(f);

    vector<uint16_t> header_bytes;
    for (int i = 0; i < len; i += 2)
    {
        uint16_t temp = ((buffer[i] << 8) | buffer[i + 1]);
        header_bytes.push_back(temp);
    }

    unsigned long sum = 0;
    for (int i = 0; i < header_bytes.size(); i++)
    {
        sum += header_bytes[i];
        if (sum >> 16)
        {
            sum = (sum >> 16) + (sum & 0xFFFF);
        }
    }
    while (sum >> 16)
    {
        sum = (sum >> 16) + (sum & 0xFFFF);
    }
    if (sum == 0xFFFF)
    {
        return true;
    }
    else
        return false;
}

int main()
{
    struct dirent *entry;
    DIR *dir = opendir("./ipfrags/");
    if (dir == NULL)
    {
        return -1;
    }

    unsigned char buffer[4];
    map<int, string> data;
    int count = 0;
    vector<string> validFile;
    set<int> validFragment;

    while ((entry = readdir(dir)) != NULL)
    {
        string filename = entry->d_name;
        filename = "./ipfrags/" + filename;
        const char *fname = filename.c_str();
        FILE *f = fopen(fname, "rb");

        int headerLength;
        unsigned char a;
        fseek(f, 0, SEEK_SET);
        fread(&a, 1, 1, f);
        headerLength = (a & 0x0F) * 4;

        // int packetLength;

        // fseek(f, 2, SEEK_SET);
        // fread(buffer, 1, 2, f);
        // packetLength = (buffer[1] << 8) || buffer[0];

        // int source[4];
        // fseek(f, 12, SEEK_SET);
        // fread(buffer, sizeof(char), 4, f);
        // for (int i = 0; i < 4; i++)
        //     source[i] = buffer[i];

        // int destination[4];
        // fseek(f, 16, SEEK_SET);
        // fread(buffer, sizeof(char), 4, f);
        // for (int i = 0; i < 4; i++)
        //     destination[i] = buffer[i];

        bool checksum = computeChecksum(fname, headerLength);

        if (checksum)
        {
            validFile.push_back(fname);

            int fragmentID;
            fseek(f, 4, SEEK_SET);
            fread(buffer, sizeof(unsigned char), 2, f);
            fragmentID = (buffer[1] << 8) || buffer[0];
            validFragment.insert(fragmentID);
            count++;
        }

        fclose(f);
    }

    cout << "#Correct Packets: " << count << endl;
    cout << "Valid Fragment IDs: " << endl;
    for (auto &x : validFragment)
        cout << x << endl;

    closedir(dir);
}

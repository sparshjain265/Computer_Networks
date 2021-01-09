#include <bits/stdc++.h>
#include <dirent.h>
#include <stdlib.h>
#include <sys/stat.h>
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

int getBufferLength(FILE *ptr)
{
    unsigned char a;
    fseek(ptr, 0, SEEK_SET);
    fread(&a, sizeof(char), 1, ptr);
    return (a & (0x0F)) * 4;
}

int getPacketLength(FILE *ptr)
{
    unsigned char buffer[2];
    fseek(ptr, 2, SEEK_SET);
    fread(buffer, sizeof(char), 2, ptr);
    int len = (buffer[1] << 8) | buffer[0];
    return len;
}

vector<int> getSource(FILE *ptr)
{
    unsigned char buffer[4];
    fseek(ptr, 12, SEEK_SET);
    fread(buffer, sizeof(char), 4, ptr);
    vector<int> ans(4);
    for (int i = 0; i < 4; i++)
    {
        ans[i] = buffer[i];
    }
    return ans;
}

vector<int> getDestination(FILE *ptr)
{
    unsigned char buffer[4];
    fseek(ptr, 16, SEEK_SET);
    fread(buffer, sizeof(char), 4, ptr);
    vector<int> ans(4);
    for (int i = 0; i < 4; i++)
    {
        ans[i] = buffer[i];
    }
    return ans;
}

int getFragmentID(FILE *ptr)
{
    unsigned char buffer[2];
    fseek(ptr, 4, SEEK_SET);
    fread(buffer, sizeof(unsigned char), 2, ptr);
    int fragment_id = (buffer[1] << 8) | buffer[0];
    return fragment_id;
}

bool computeChecksum(FILE *ptr, int bufferLength)
{
    unsigned char buffer[bufferLength];
    fseek(ptr, 0, SEEK_SET);
    fread(buffer, sizeof(char), bufferLength, ptr);

    vector<uint16_t> header_bytes;
    for (int i = 0; i < bufferLength; i += 2)
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

map<int, string> data;

//given fragment id combines the data
void create_data(map<int, string> data, int id, char *filename)
{
    FILE *f = fopen(filename, "rb");
    int fragment = getFragmentID(f);
    if (fragment == id)
    {
        int bufferLength = getBufferLength(f);
        int total_len = getPacketLength(f);
        unsigned char buffer[total_len];

        fseek(f, 0L, SEEK_SET);
        fread(buffer, 1 * total_len, 1, f);

        unsigned short fragment = buffer[6] | (buffer[7] << 8);
        int offset = (fragment & 0x1FFF);
        int flag = (fragment & 0xE000) >> 13;
        string packet_data((char *)(&buffer[bufferLength]));

        packet_data.resize(total_len - bufferLength);

        if (total_len - bufferLength != 0)
        {
            data[offset] = packet_data;
        }
    }
    fclose(f);
}

int main()
{

    DIR *dir;
    struct dirent *ent;
    int counter = 0;
    map<int, string> data;
    vector<string> valid_ips;
    set<int> valid_fragment_ids;
    if ((dir = opendir(".")) != NULL)
    {
        while ((ent = readdir(dir)) != NULL)
        {
            // printf("%s \n", ent->d_name);

            int len = getBufferLength(ent->d_name);
            // cout << "Length of header :" << len * 4 << endl;
            bool check = computeChecksum(ent->d_name, len);

            if (check)
            {
                string s(ent->d_name);
                valid_ips.push_back(s);
                valid_fragment_ids.insert(getFragmentID(ent->d_name));
                counter++;
            }
        }
        cout << "Total Correct packets : " << counter << endl;
        cout << "Valid fragment ID are : " << endl;
        for (auto &x : valid_fragment_ids)
        {
            cout << x << endl;
        }
        map<int, vector<string>> filename_fragment_id;
        for (int i = 0; i < counter; i++)
        {
            char *cstr = new char[valid_ips[i].length() + 1];
            strcpy(cstr, valid_ips[i].c_str());
            filename_fragment_id[getFragmentID(cstr)].push_back(valid_ips[i]);
        }

        for (auto &x : filename_fragment_id)
        {
            data.clear();
            for (int i = 0; i < x.second.size(); i++)
            {
                char *cstr = new char[x.second[i].length() + 1];
                strcpy(cstr, x.second[i].c_str());
                create_data(x.first, cstr);
            }

            string msg = "";
            for (auto &y : data)
            {
                msg = msg + y.second;
            }
            cout << "Message data with fragment id " << x.first << " : " << endl;
            cout << msg << endl;
        }

        // for (int i= 0 ; i < counter ; i++){
        //     cout <<i <<". " << valid_ips[i] << endl;
        //     char *cstr = new char[valid_ips[i].length() + 1];

        //     /**Source IP**/
        //     strcpy(cstr, valid_ips[i].c_str());
        //     vector<int> source = getSource(cstr);
        //     // cout << "Source IP : " << source[0];
        //     // for (int i = 1; i < 4; i++){
        //         // cout << "." << source[i];
        //     // }
        //     // cout << endl;

        //     /**Destination IP**/
        //     vector<int> dest = getDestination(cstr);
        //     // cout << "Destination IP : " << dest[0];
        //     // for (int i = 1; i < 4; i++)
        //     // {
        //         // cout << "." << dest[i];
        //     // }
        //     // cout << endl;

        //     cout << "Fragment ID : " << getFragmentID(cstr) << endl;
        //     // cout << "Packet Length :" << getPacketLength(cstr)  << endl;
        //     // cout << "File size : " << file_size(cstr) << endl;

        //     create_data(38906 , cstr);
        // }
        closedir(dir);
    }
    else
    {
        perror("");
        return EXIT_FAILURE;
    }
    return 0;
}
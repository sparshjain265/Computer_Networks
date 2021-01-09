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

int file_size(char *filename)
{
    FILE *ptr;
    ptr = fopen(filename, "rb");
    fseek(ptr, 0L, SEEK_END);
    int sz = ftell(ptr);
    fclose(ptr);
    return sz;
}

int compute_len(char *filename)
{
    FILE *ptr;
    unsigned char a;
    ptr = fopen(filename, "rb"); // r for read, b for binary
    fread(&a, 1, 1, ptr);
    fclose(ptr);
    return (a & (0x0F));
}
int packet_len(char *filename)
{
    FILE *ptr;
    unsigned char buffer[2];
    ptr = fopen(filename, "rb"); // r for read, b for binary
    fseek(ptr, 2, SEEK_SET);
    fread(buffer, 1, 2, ptr);
    fclose(ptr);
    int len = (buffer[1] << 8) | buffer[0];
    return len;
}

vector<int> find_source(char *filename)
{
    FILE *ptr;
    ptr = fopen(filename, "rb");
    fseek(ptr, 12, SEEK_SET);
    unsigned char buffer[4];
    fread(buffer, sizeof(char), 4, ptr);
    vector<int> ans(4);
    for (int i = 0; i < 4; i++)
    {
        ans[i] = buffer[i];
    }
    fclose(ptr);
    return ans;
}

vector<int> find_destination(char *filename)
{
    FILE *ptr;
    ptr = fopen(filename, "rb");
    fseek(ptr, 16, SEEK_SET);
    unsigned char buffer[4];
    fread(buffer, sizeof(char), 4, ptr);
    vector<int> ans(4);
    for (int i = 0; i < 4; i++)
    {
        // cout << (buffer[i] | 0)<< endl;
        ans[i] = buffer[i];
    }
    fclose(ptr);
    return ans;
}

int find_fragemnt_id(char *filename)
{
    FILE *ptr;
    unsigned char buffer[2];
    ptr = fopen(filename, "rb"); // r for read, b for binary
    fseek(ptr, 4, SEEK_SET);
    fread(buffer, sizeof(unsigned char), 2, ptr);
    fclose(ptr);
    int fragment_id = (buffer[1] << 8) | buffer[0];
    return fragment_id;
}

bool compute_checksum(const char *filename, int len)
{
    FILE *ptr;
    ptr = fopen(filename, "rb");
    unsigned char buffer[len * 4];
    fread(buffer, sizeof(char), len * 4, ptr);
    fclose(ptr);

    vector<uint16_t> header_bytes;
    for (int i = 0; i < len * 4; i += 2)
    {
        uint16_t temp = ((buffer[i] << 8) | buffer[i + 1]);
        header_bytes.push_back(temp);
    }

    unsigned long sum = 0;
    for (int i = 0; i < header_bytes.size(); i++)
    {
        // cout << std::hex << header_bytes[i] << endl;
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
void create_data(int id, char *filename)
{
    int fragment = find_fragemnt_id(filename);
    if (fragment == id)
    {
        int len = compute_len(filename);
        int total_len = packet_len(filename);
        unsigned char buffer[total_len];

        //reading the file
        FILE *ptr;
        ptr = fopen(filename, "rb");
        fread(buffer, 1 * total_len, 1, ptr);
        fclose(ptr);

        unsigned short fragment = buffer[6] | (buffer[7] << 8);
        int offset = (fragment & 0x1FFF);
        int flag = (fragment & 0xE000) >> 13;
        string packet_data((char *)(&buffer[len * 4]));

        // cout << int(buffer[6]) << " " << int(buffer[7]) << endl;
        // cout << "Offset : "<< offset;
        // cout << " Flag : " << flag;
        // cout << "  DF : " << ((flag & 0x2) >> 1);
        // cout << "  MF : " << (flag & 0x1) ;
        // cout << " Data Length : "<< total_len - len * 4<< endl;
        packet_data.resize(total_len - len * 4);

        // cout << " Data : " << packet_data << " data size : " << packet_data.size() << endl;
        if (total_len - len * 4 != 0)
        {
            data[offset] = packet_data;
        }
    }
}

int main()
{

    DIR *dir;
    struct dirent *ent;
    int counter = 0;
    vector<string> valid_ips;
    set<int> valid_fragment_ids;
    if ((dir = opendir(".")) != NULL)
    {
        while ((ent = readdir(dir)) != NULL)
        {
            // printf("%s \n", ent->d_name);

            int len = compute_len(ent->d_name);
            // cout << "Length of header :" << len * 4 << endl;
            bool check = compute_checksum(ent->d_name, len);

            if (check)
            {
                string s(ent->d_name);
                valid_ips.push_back(s);
                valid_fragment_ids.insert(find_fragemnt_id(ent->d_name));
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
            filename_fragment_id[find_fragemnt_id(cstr)].push_back(valid_ips[i]);
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
        //     vector<int> source = find_source(cstr);
        //     // cout << "Source IP : " << source[0];
        //     // for (int i = 1; i < 4; i++){
        //         // cout << "." << source[i];
        //     // }
        //     // cout << endl;

        //     /**Destination IP**/
        //     vector<int> dest = find_destination(cstr);
        //     // cout << "Destination IP : " << dest[0];
        //     // for (int i = 1; i < 4; i++)
        //     // {
        //         // cout << "." << dest[i];
        //     // }
        //     // cout << endl;

        //     cout << "Fragment ID : " << find_fragemnt_id(cstr) << endl;
        //     // cout << "Packet Length :" << packet_len(cstr)  << endl;
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
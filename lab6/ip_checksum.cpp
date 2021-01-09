/**
 * Author: Kaushal Kishore 
 * Computer Netoworks Lab 6
 */

#include <iostream>
#include <sys/types.h>
#include <dirent.h>
#include <fstream>
#include <vector>
#include <string>

#define MAX_BUFF 1024

using namespace std;

/**
 * Lists all files and sub-directories at given path.
 */
void list_files(const char *path, vector<string>& v) {
    struct dirent *dp;
    DIR *dir = opendir(path);

    // Unable to open directory stream
    if (!dir) return; 

    while ((dp = readdir(dir)) != NULL) {
        //printf("%s\n", dp->d_name);
        if (string(dp->d_name) != "." && string(dp->d_name) != "..")
			v.push_back(string(dp->d_name));
    }

    // Close directory stream
    closedir(dir);
}

bool check_integrity(const char *filename){
    ifstream input(filename, ios::binary);
    vector<unsigned char> buffer(istreambuf_iterator<char>(input), {});

    int hlen = (buffer[0] & 0xF); // number of 32-bit words

    vector <uint16_t> header_bytes;
    for (int i= 0; i < 4 * hlen ; i+=2){
        uint16_t temp = ((buffer[i] << 8) | buffer[i+1]);
        header_bytes.push_back(temp);
    }

    unsigned long sum = 0;
    for (int i= 0 ; i < header_bytes.size() ; i++){
        sum += header_bytes[i];
        if (sum >> 16){
            sum = (sum >> 16) + (sum & 0xFFFF);
        }
    }
    while (sum >> 16){
        sum = (sum >> 16) + (sum & 0xFFFF);
    }
    if (sum == 0xFFFF){
        return true;
    }
    else return false;
}


int main(int argc, char *argv[]) {
    vector<string> ip_frags;
    vector<string> good_ip_frags;

    string path(argv[1]);

    list_files(path.c_str(), ip_frags);
    
	for(auto &s : ip_frags) {
		cout << "Reading file : " << s << "........";

        if (check_integrity(string(path + "/" + s).c_str())) {
            good_ip_frags.push_back(s);
            cout << "good" << "\n";
        } else {
            cout << "bad" << "\n";
        }
    }

    cout << "\n" << "Number of files good: " << good_ip_frags.size() << "\n";
    return 0;
}
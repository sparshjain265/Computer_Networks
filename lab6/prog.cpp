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

using namespace std;

vector<string> ipFrags;

void listFiles(const char *path);
bool checkIntegrity(vector<int>&);


int main(int argc, char *argv[]) {

    // Directory path to list files
   	
    string path(argv[1]);

    // Input path from user
    // cout<< "Enter path to list files: ";
    // cin >> path;
    // cout << endl;

    listFiles(path.c_str());
    
    int count = 0;

	/* OPERATING ON BINARY FILE */    
    for(auto &s : ipFrags) {
		cout << "Reading file : " << s << endl;
		
		
    	ifstream input(path + "/" + s, ios::binary);
    	
    	// chunks of 32-bit data
    	vector<unsigned char> buf(istreambuf_iterator<char>(input), {});
		vector<int> chunk;
		
		for (auto &itr : buf) {
			chunk.push_back(int(itr));
		}


		if(checkIntegrity(chunk)) {
			cout << "\tfile ................ good" << endl;
			++count;
		} else {
			cout << "\tfile ................ bad" << endl;
		}

		// source addr
//		cout <<"\tSource: " << int(buf[13]) << "." << int(buf[14])<< "." << int(buf[15]) << "." << int(buf[16]) << endl;
		
		// destination addr
	//	cout <<"\tDest: " << int(buf[17]) << "." << int(buf[18])<< "." << int(buf[19]) << "." << int(buf[20]) << endl;
    	
//    	exit(0);

		cout << endl;
    	
    }

    cout << "Num Packets good : " << count << endl;
    
    
//    vector <int> customChunk {0x45, 0x00, 0x00, 0x73,  0x00, 0x00,  0x40, 0x00,  0x40, 0x11, 0xb8, 0x61,  0xc0, 0xa8,  0x00, 0x01,  0xc0, 0xa8, 0x00, 0xc7};
	vector <int> customChunk {0x45, 0x00, 0x00, 0x3c,  0x1c, 0x46,  0x40, 0x00,  0x40, 0x06, 0xb1, 0xe6,  0xac, 0x10,  0x0a, 0x63, 0xac, 0x10, 0x0a, 0x0c};
    checkIntegrity(customChunk);

    return 0;
}



bool checkIntegrity(vector<int>& chunk){

	const unsigned int mask = ~(0xFFFF);
	
	cout << std::hex << mask << endl;
	
	unsigned int check_sum = 0;
	unsigned int curr_num = 0;
	
	
	for (int i = 0; i <= 20; i++) {
		if ((i % 2) == 0){
			check_sum += curr_num;
			curr_num = chunk[i]<<8;
			cout<< "i: "<< i <<" chunk: "<< chunk[i] <<" curr_num : "  << curr_num << " check_sum : "<< check_sum << endl;
		} else {
			curr_num = curr_num | chunk[i];
			cout<< "i: "<< i <<" chunk: "<< chunk[i] <<" curr_num : "  << curr_num << " check_sum : "<< check_sum << endl;
		}
	}
	

	
	if((mask & check_sum) != 0) {
		unsigned int d = (mask & check_sum);
		cout << (check_sum & 0xffff) << " d : " << (d>>16) << endl;
		check_sum = (check_sum & 0xFFFF) + (d>>16);
	}
	
	cout << check_sum <<endl;
	
	if (check_sum == 0xFFFF) {
		cout << "good file" << endl;
		return true;
	}
	
	return false;
}


/**
 * Lists all files and sub-directories at given path.
 */
void listFiles(const char *path) {
    struct dirent *dp;
    DIR *dir = opendir(path);

    // Unable to open directory stream
    if (!dir) return; 

    while ((dp = readdir(dir)) != NULL) {
        //printf("%s\n", dp->d_name);
        if (string(dp->d_name) != "." && string(dp->d_name) != "..")
			ipFrags.push_back(string(dp->d_name));
    }

    // Close directory stream
    closedir(dir);
}

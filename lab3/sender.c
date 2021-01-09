#include <stdio.h>
#include <string.h>
#include <stdlib.h>

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

		ham[j][0] = str[i + 0];
		ham[j][1] = str[i + 1];
		ham[j][2] = str[i + 2];
		ham[j][4] = str[i + 3];
			
		ham[j][6] = ((str[i + 0] == '1') ^ (str[i + 2] == '1') ^ (str[i + 4] == '1')) ? '1' : '0';
		ham[j][5] = ((str[i + 0] == '1') ^ (str[i + 1] == '1') ^ (str[i + 4] == '1')) ? '1' : '0';
		ham[j][3] = ((str[i + 0] == '1') ^ (str[i + 1] == '1') ^ (str[i + 2] == '1')) ? '1' : '0';
		
		ham[j][7] = '\0';
		printf("%s\n", ham[j]);
	}
	*num_frames = j;
	return ham;
}


char* bit_stuff(char *str){
	char *stuffed = (char*) malloc(sizeof(char) * strlen(str) * WIDTH * 2);

	
	int count, nl;
	for (int i = 0; i < strlen(str); i++) {
		count=0;
		
		//the following code search the six ones in given string
		for (int j = i; j <= (i+5); j++) {
			if(str[j] == '1') {
				count++;
			}
		}

		//if there is six ones then folling code execute to bit stuffing after five ones
		if(count == 6) {
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
	int rem = 4 - (nl % 4);
	while (rem--) {
		strcat(stuffed, "0");
	}
		nl = strlen(stuffed);
	
	printf("bit stuffed with padding with zeros: %s\n\n, strlen : %d \n", stuffed, nl);

	free(str);

	return stuffed;
}

int main(){
	char message[] = {255, 255, '\0'};
	char len = strlen(message);

	int bit_str_len = 0;
	char *bit_str = (char*) malloc(sizeof(char) * len * WIDTH * 5);

	for (int i = 0; i < len; i++) {
		get_bits(message[i], &bit_str[i*WIDTH]);
		bit_str[(i+1)*WIDTH] = '\0';
	}

	bit_str_len = strlen(bit_str);
	printf("bitstr: \n%s\n%d\n", bit_str, bit_str_len);
	bit_str = bit_stuff(bit_str);
	printf("bit str updated : %s\n\n", bit_str);

	int num_frames = 0;
	char** ham = get_hammed(bit_str, &num_frames);

	for (int i=0; i< num_frames; i++) {
		free(ham[i]);
	}

	free(ham);
	
	free(bit_str);
	return 0;
}

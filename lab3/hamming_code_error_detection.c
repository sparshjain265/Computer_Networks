#include <stdio.h>

#define EVEN (0)
#define ODD (1)

int main() {
 // original data : 1101
    char input[] = "1100110";

    int parity = EVEN;
    for ( int mask = 4; mask; mask >>= 1 ){
        for ( int bit = 1; bit <= 7; bit++ ){
            if ( bit & mask ){
				if ( input[7-bit] == '1' )
				parity ^= mask;
			}

		}
    }

    printf("%d\n", parity);
}

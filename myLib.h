// Nastasescu George-Silviu, 321CB

#include "link_emulator/lib.h"

#define MYSIZE 1392

typedef struct {
  int index;
  char checksum;
  char payload[MYSIZE];
} myMsg;

char set_checksum(myMsg m, int size) {

	int i, index = m.index;
	char checksum = 0;

	for(i = 0; i < size; i++) {
		checksum ^= m.payload[i];
	}

	for(i = 0; i < 4; i++) {
		checksum ^= index % 256;
		index = index >> 8;
	}

	return checksum;
}

int correct_checksum(myMsg m, int size) {

	int i, j, index = m.index, temp;
	char checksum = m.checksum;

	for(i = 3; i >= 0; i--) {
		temp = 1;

		for(j = 0; j < i; j++) {
			temp = temp << 8;
		}

		checksum ^= (index >> (8 * i)) % 256;
	}

	for(i = size - 1; i >= 0; i--) {
		checksum = checksum ^ m.payload[i];
	}

	if(checksum == 0)
		return 1;
	
	return 0;
}
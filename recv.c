// Nastasescu George-Silviu, 321CB

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "myLib.h"
#include "link_emulator/lib.h"

#define HOST "127.0.0.1"
#define PORT 10001

int main(int argc,char** argv){
	msg r, t;
	myMsg m;
	char cnume = 0, *p, buffer[1200000], stream[105], nume[20];
	int fid = 0, i, count = 0, size = 0, nr = 15000;
	init(HOST,PORT);

	// Initializam bitstreamul cu zerouri
  	memset(stream, 0, 105); 

	// Cat timp nu s-au primit toate pachetele
	while(count <= nr) { 

		recv_message(&r);
		memcpy(&m, r.payload, sizeof(m));

		// Verificam daca s-a corupt un pachet
		if(correct_checksum(m, r.len) == 0) { 
			strcpy(t.payload, "ACK");
			t.len = -2;
			send_message(&t);
			continue;
		}

		i = m.index;
		
		// Verificam daca am primit pachetul cu numele fisierului
		if(i == -1 && cnume == 0) { 
			cnume = 1;
			count++;
			strcpy(nume, "recv_");
			p = strtok(m.payload, " ");
			strcat(nume, p);
			p = strtok(NULL, " ");
			size = atoi(p);
		  	nr = size / MYSIZE;

			if(size % MYSIZE != 0)
				nr++;
			fid = open(nume, O_CREAT | O_TRUNC | O_WRONLY, 0644);
		}

		// Verificam daca nu am mai primit acest pachet
		else if( (stream [i / 8] & (1 << (i % 8)) ) == 0) {
			memcpy(buffer + i * MYSIZE, m.payload, r.len);
			count++;
			stream[i / 8] |= (1 << i % 8);
		}

		strcpy(t.payload, "ACK");
		t.len = i;
		send_message(&t);
	}

	if(write(fid, buffer, size) < 0) {
		printf("[RECV] Nu a mers scrierea in fisier\n");
		exit(1);
	}

	close(fid);

	strcpy(t.payload, "ACK");
	t.len = -3;

	// Spunem ca am terminat scrierea in fisier
	while(1) {
		send_message(&t);
	}
	
  	return 0;
}

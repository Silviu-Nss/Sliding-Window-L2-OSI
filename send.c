// Nastasescu George-Silviu, 321CB

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <math.h>
#include "myLib.h"
#include "link_emulator/lib.h"

#define HOST "127.0.0.1"
#define PORT 10000

int main(int argc,char** argv){

	init(HOST,PORT);
	msg r, t, t_nume, t_last;
	char nume[15], size_c[10], stream[105], cnume = 0, clast = 0;
	myMsg m, my_msgs[850];

	int timeout, fid, size, v = atoi(argv[2]), tp = atoi(argv[3]);
  	short i = 0, j, sent = 0, window, index, nr, new_nr, indexes[850], old_nr;

  	// Deschidem fisierul si ii determinam dimensiunea
   	fid = open(argv[1], O_RDONLY);
  	size = lseek(fid, 0, SEEK_END);
  	lseek(fid, 0, SEEK_SET);
  	size = read(fid, my_msgs, size);
  	close(fid);

  	// Determinam numarul de pachete
  	nr = size / MYSIZE;
  	if(size % MYSIZE != 0)
  		nr++;

	// Initializam bitstreamul cu zerouri
  	memset(stream, 0, 105);

	window = (v * tp * 1000) / (8 * sizeof(msg));

  	timeout = tp + 1;

  	// Cream pachetul cu numele fisierului
	strcpy(nume, argv[1]);
	sprintf(size_c, " %d", size);
	strcat(nume, size_c);
	strcpy(m.payload, nume);
	m.index = -1;
	t_nume.len = strlen(argv[1]) + 1;
	m.checksum = set_checksum(m, t_nume.len);
	memcpy(t_nume.payload, &m, MSGSIZE);

  	// Cream mesajul cu ultima informatie din fisier
	nr--;
  	old_nr = nr;
	m.index = nr;
	t_last.len = size - nr * MYSIZE;
	memcpy(m.payload, ((char*)my_msgs) + m.index * MYSIZE, t_last.len);
	m.checksum = set_checksum(m, t_last.len);
	memcpy(t_last.payload, &m, MSGSIZE);

  	for(i = 0; i < nr; i++)
  		indexes[i] = i;

  	i = 0;

	send_message(&t_nume);
	send_message(&t_last);

  	while(1) {

  		// Trimitem mesaje pana cand se umple fereastra
  		// sau pana cand terminam indicii
		while(sent < window && i < nr) {

			index = indexes[i++];

			// Verificam daca nu am primit ACK pentru acest mesaj
			if( ( stream [index / 8] & (1 << (index % 8)) ) == 0) {
				memcpy(m.payload, ((char*)my_msgs) + index * MYSIZE, MYSIZE);
				m.index = index;

				// Ne asiguram ca mesajele speciale ajung la receiver
				if(i % 15 == 0) {
					if(cnume == 0)
						send_message(&t_nume);
					else if(clast == 0)
						send_message(&t_last);
					else
						sent--;
				}
				m.checksum = set_checksum(m, MYSIZE);
				t.len = MYSIZE;
				memcpy(t.payload, &m, MSGSIZE);
				sent++;
				send_message(&t);
			}
		}

		// Reconstruim vectorul de indici
		if(i == nr) {

			new_nr = 0;

			for(i = 0; i < nr; i++) {
				j = indexes[i];
				if( ( stream [j / 8] & (1 << (j % 8)) ) == 0)
					indexes[new_nr++] = j;
			}
			nr = new_nr;
			i = 0;
		}

		sent--;

		// Nu facem nimic deosebit daca primim timeout
		if(recv_message_timeout(&r, timeout) < 0);

		// Fisierul a fost scris
		else if(r.len == -3) 
			break;

		// Nu facem nimic deosebit daca s-a corupt un mesaj
		else if(r.len == -2);

		// ACK la numele fisierului
		else if(r.len == -1)
			cnume = 1;

		// ACK la ultimul mesaj
		else if(r.len == old_nr)
			clast = 1;

		// ACK la un alt pachet
		else if( ( stream [r.len / 8] & (1 << (r.len % 8)) ) == 0)
   			stream[r.len / 8] |= (1 << r.len % 8);
	}

  	return 0;
}

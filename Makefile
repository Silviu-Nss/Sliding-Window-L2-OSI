all: send recv

link_emulator/lib.o:
	$(MAKE) -C link_emulator

send: send.o link_emulator/lib.o
	gcc -g -Werror send.o link_emulator/lib.o -o send -O3

recv: recv.o link_emulator/lib.o
	gcc -g -Werror recv.o link_emulator/lib.o -o recv -O3

.c.o:
	gcc -Werror -Wall -g -c $? -O3

clean:
	$(MAKE) -C link_emulator clean
	-rm -f *.o send recv

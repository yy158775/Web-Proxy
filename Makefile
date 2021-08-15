.c: 
	cd ~/cslib; make csapp.o
	gcc -O2 -I ~/cslib -pthread -g -o $* $< -no-pie  csapp.o sbuf.o cache.o
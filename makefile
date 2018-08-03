build: main.c avl.c
	gcc -std=c99 -pedantic main.c avl.c mymem.c

avl.o: avl.c avl.h
	gcc -c avl.c

mymem.o: mymem.c mymem.h
	gcc -c mymem.c

clean:
	rm avl.o mymem.o

run:
	clear
	./a.out

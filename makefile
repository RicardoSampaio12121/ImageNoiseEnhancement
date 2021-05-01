output: main.o vc.o
		gcc main.o vc.o -lm -o prog

p1.o: main.c
		gcc -c main.c

vc.o: vc.c vc.h
	  gcc -c vc.c

clean: 
	   rm +*.o prog
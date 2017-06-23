all: myELF
 
hexeditplus: myELF.o 
	gcc -Wall -m32 -g  -o myELF myELF.o 
hexeditplus.o : myELF.c
	gcc -Wall -m32 -g -c myELF.c
clean:
	rm -f *.o  myELF

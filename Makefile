all: fs.o disk.o main.o
		gcc -g fs.o main.o disk.o -o main

main.o:
		gcc -g -c main.c -o main.o

disk.o:
		gcc -g -c disk.c -o disk.o

fs.o:
		gcc -g -c fs.c -o fs.o

clean:
		rm -f *.o main


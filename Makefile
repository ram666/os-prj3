all: fs.o disk.o main.o
		gcc fs.o main.o disk.o -o main

main.o:
		gcc -c main.c -o main.o

disk.o:
		gcc -c disk.c -o disk.o

fs.o:
		gcc -c fs.c -o fs.o


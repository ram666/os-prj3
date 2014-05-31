all: disk.o main.o
		gcc main.o disk.o -o main

main.o:
		gcc -c main.c -o main.o

disk.o:
		gcc -c disk.c -o disk.o


CC=g++

CFLAGS=-std=c++1z

map_reduce.o:
	$(CC) $(CFLAGS) -Icode/ code/main.cpp -o map_reduce -v
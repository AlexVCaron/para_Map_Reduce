

CC=g++

CFLAGS=-std=c++1z

map_reduce.o:
	$(CC) $(CFLAGS) main.cpp -o map_reduce
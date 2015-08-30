CC=g++
CFLAGS=-Wall -std=c++11

all:
	$(CC) $(CFLAGS) benchmark.cpp -o benchmark

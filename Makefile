#Copyright 2021 Mitroi Mihnea-Cristian
CC=gcc
CFLAGS=-std=c99 -Wall -Wextra
LOAD=load_balancer
SERVER=server
HT=hashtable
LL=linkedlist

.PHONY: build clean

build: tema2

tema2: main.o $(LOAD).o $(SERVER).o $(HT).o $(LL).o
	$(CC) $(CFLAGS) $^ -o $@

main.o: main.c
	$(CC) $(CFLAGS) $^ -c

$(LL).o: $(LL).c $(LL).h
	$(CC) $(CFLAGS) $^ -c

$(HT).o: $(HT).c $(HT).h
	$(CC) $(CFLAGS) $^ -c

$(SERVER).o: $(SERVER).c $(SERVER).h
	$(CC) $(CFLAGS) $^ -c

$(LOAD).o: $(LOAD).c $(LOAD).h
	$(CC) $(CFLAGS) $^ -c

clean:
	rm -f *.o tema2 *.h.gch

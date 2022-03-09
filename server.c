/* Copyright 2021 Mitroi Mihnea-Cristian */#include <stdlib.h>
#include <string.h>

#include "server.h"
#define BUCKET_NUMBER 41

unsigned int hash_function_key(void *a)
{
    unsigned char *puchar_a = (unsigned char *) a;
    unsigned int hash = 5381;
    int c;

    while ((c = *puchar_a++))
        hash = ((hash << 5u) + hash) + c;

    return hash;
}

/* Functia aloca spatiu pentru un server si initializeaza un hashtable. */
server_memory* init_server_memory()
{
	server_memory *server = malloc(sizeof(*server));
	DIE(server == NULL, "malloc error");
	server->hashtable = ht_create(BUCKET_NUMBER,
						hash_function_key, compare_function_strings);
	return server;
}

/* Functia stocheaza pe un server perechea cheie - valoare. */
void server_store(server_memory* server, char* key, char* value)
{
	ht_put(server->hashtable, key, strlen(key) + 1, value, strlen(value) + 1);
}

/* Functia sterge de pe un server perechea cheie - valoare. */
void server_remove(server_memory* server, char* key)
{
	ht_remove_entry(server->hashtable, key);
}

/* Functia verifica daca pe un server exista perechea cheie - valoare. */
char* server_retrieve(server_memory* server, char* key)
{
	return ht_get(server->hashtable, key);
}

/* Functia elibereaza memoria pentru hashtable si pentru server. */
void free_server_memory(server_memory* server)
{
	ht_free(server->hashtable);
	free(server);
}

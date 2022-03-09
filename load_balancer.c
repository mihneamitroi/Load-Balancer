/* Copyright 2021 Mitroi Mihnea-Cristian */
#include <stdlib.h>
#include <string.h>

#include "load_balancer.h"
#define SERVER_NUMBER 100000

typedef struct hashring hashring;
struct hashring {
	unsigned int hash;
	unsigned int tag;
};

struct load_balancer
{
	server_memory **servers;
    hashring *hashring;
    unsigned int size;
};

/* Implementare strdup */
static char *strdup(const char *src) {
    char *dst = malloc(strlen(src) + 1);
    if (dst == NULL) return NULL;
    memcpy(dst, src, strlen(src) + 1);
    return dst;
}

unsigned int hash_function_servers(void *a)
{
    unsigned int uint_a = *((unsigned int *)a);

    uint_a = ((uint_a >> 16u) ^ uint_a) * 0x45d9f3b;
    uint_a = ((uint_a >> 16u) ^ uint_a) * 0x45d9f3b;
    uint_a = (uint_a >> 16u) ^ uint_a;
    return uint_a;
}

/* Functia aloca spatiu pentru load balancer, apoi pentru vectorul de servere
si pentru hashring, ale carui elemente sunt initializate pe 0. */
load_balancer* init_load_balancer()
{
    load_balancer *balancer = malloc(sizeof(*balancer));
    DIE(balancer == NULL, "malloc error");
    balancer->servers = malloc(SERVER_NUMBER * sizeof(balancer->servers));
    DIE(balancer->servers == NULL, "malloc error");
    balancer->hashring = calloc(3 * SERVER_NUMBER, sizeof(balancer->hashring));
    DIE(balancer->hashring == NULL, "malloc error");
    balancer->size = 0;
    return balancer;
}

/* Functia afla hash-ul unei chei si apoi o compara cu hash-urile stocate pe
hashring. Daca gaseste un server cu id mai mare, pe acela o va stoca. In cazul
in care obiectul are un hash mai mare decat ultimul server, acesta va fi
stocat pe primul server din hashring. */
void loader_store(load_balancer* main, char* key, char* value, int* server_id)
{
	unsigned int hash = hash_function_key(key);
    unsigned int i;
    for (i = 0; i < main->size; ++i) {
        if (hash < main->hashring[i].hash) {
            *server_id = main->hashring[i].tag;
            break;
        }
    }
    if (hash > main->hashring[main->size - 1].hash)
        *server_id = main->hashring[0].tag;
    server_store(main->servers[*server_id], key, value);
}

/* Functia afla hash-ul unei chei si apoi o compara cu hash-urile stocate pe
hashring. Daca gaseste un server cu id mai mare, pe acela o va cauta. In cazul
in care obiectul are un hash mai mare decat ultimul server, acesta va fi
cautat pe primul server din hashring. */
char* loader_retrieve(load_balancer* main, char* key, int* server_id)
{
	unsigned int hash = hash_function_key(key);
    unsigned int i;
    for (i = 0; i < main->size; ++i) {
        if (hash < main->hashring[i].hash) {
            *server_id = main->hashring[i].tag;
            break;
        }
    }
    if (hash > main->hashring[main->size - 1].hash) {
        *server_id = main->hashring[0].tag;
    }
    return server_retrieve(main->servers[*server_id], key);
}

/* Functia se ocupa cu adaugarea unei perechi hash - id server pe hashring.
Daca hash-ul serverului de adaugat este mai mic decat unul existent deja,
elementele sunt deplasate cu o pozitie pentru a face loc pentru inserare.
Altfel, se adauga pe prima pozitie libera a vectorului si se mareste size-ul.*/
void ring_add(load_balancer* main, unsigned int hash, unsigned int server_id)
{
    unsigned int i, j;
    for (i = 0; i <= main->size; ++i) {
        if (hash < main->hashring[i].hash) {
            for (j = main->size; j > i; --j)
                main->hashring[j] = main->hashring[j - 1];
            main->hashring[i].hash = hash;
            main->hashring[i].tag = server_id;
            break;
        }
        if (i == main->size) {
            main->hashring[i].hash = hash;
            main->hashring[i].tag = server_id;
            break;
        }
    }
    main->size++;
}

/* Functia afla mai intai pe ce pozitie se afla in hashring serverul adaugat
si afla pozitia de pe hashring a urmatorului server in ordine crescatoare
deoarece de la el se vor prelua eventuale obiecte. Apoi, daca daca serverul
curent si cel urmator nu sunt acelasi, toate obiectele sunt sterse de pe
serverul urmator pe care erau stocate si sunt plasate pe serverul corespunzator
dupa modificarea hashring-ului de catre functia load_server, implementata deja.
*/
void move_objects(load_balancer* main, unsigned int hash,
                  unsigned int server_id)
{
    int x;
    unsigned int i, next_server_position;
    char *key, *value;
    ll_node_t *node;
    for (i = 0; i < main->size; ++i) {
        if (main->hashring[i].hash == hash) {
            if (i == main->size - 1) {
                next_server_position = 0;
            } else {
                next_server_position = i + 1;
            }
        }
    }
    if (server_id != main->hashring[next_server_position].tag) {
        for (i = 0; i < main->servers[main->hashring
                        [next_server_position].tag]->hashtable->hmax; ++i) {
            node = main->servers[main->hashring[next_server_position].tag]
                   ->hashtable->buckets[i]->head;
            while (node != NULL) {
                key = strdup(((info*)node->data)->key);
                value = strdup(((info*)node->data)->value);
                node = node->next;
                server_remove(main->servers[main->hashring
                              [next_server_position].tag], key);
                loader_store(main, key, value, &x);
                free(key);
                free(value);
            }
        }
    }
}

/* Functia calculeaza hash-ul serverului si al replicilor si le adauga pe ring.
Apoi initializeaza memoria pentru serverul corespunzator si muta eventualele
obiecte. */
void loader_add_server(load_balancer* main, int server_id)
{
	unsigned int copy1, copy2, hash, hash1, hash2;
    copy1 = SERVER_NUMBER + server_id;
    copy2 = 2 * SERVER_NUMBER + server_id;
    hash = hash_function_servers(&server_id);
    hash1 = hash_function_servers(&copy1);
    hash2 = hash_function_servers(&copy2);
    ring_add(main, hash, server_id);
    ring_add(main, hash1, server_id);
    ring_add(main, hash2, server_id);
    main->servers[server_id] = init_server_memory();
    move_objects(main, hash, server_id);
    move_objects(main, hash1, server_id);
    move_objects(main, hash2, server_id);
}

/* Functia cauta pe ring orice eticheta ce are id serverului care trebuie sters
si, in caz ca gaseste, deplaseaza elementele cu o pozitie la stanga, eliminand
perechea hash - server id si creand loc in vector. */
void ring_remove(load_balancer* main, unsigned int server_id)
{
    unsigned int i, j;
    for (i = 0; i < main->size; ++i) {
        if (server_id == main->hashring[i].tag) {
            for (j = i; j < main->size; ++j)
                main->hashring[j] = main->hashring[j + 1];
            main->size--;
            i--;
        }
    }
}

/* Functia elimina mai intai vectorul de pe hashring si, in concordanta cu noul
vector, stocheaza utilizand funtia loader_store obiectele care erau initial
aflate pe serverul care a fost sters. Apoi elibereaza memoria serverului. */
void loader_remove_server(load_balancer* main, int server_id)
{
    unsigned int i;
    int x;
    char *key, *value;
    ll_node_t *node;
    ring_remove(main, server_id);
    for (i = 0; i < main->servers[server_id]->hashtable->hmax; ++i) {
        node = main->servers[server_id]->hashtable->buckets[i]->head;
        while (node != NULL) {
            key = strdup(((info*)node->data)->key);
            value = strdup(((info*)node->data)->value);
            loader_store(main, key, value, &x);
            free(key);
            free(value);
            node = node->next;
        }
    }
    free_server_memory(main->servers[server_id]);
}

/* Functia cauta toate serverele care sunt initalizate deja, verificand in
hashring daca hash-ul stocat este acelasi cu hash-ul id-ului serverului.
Acest lucru se petrece pentru a evita stergerea unui anumit server in caz ca
se intalneste replica acestuia stocata pe hashring. Apoi elibereaza memoria
pentru hashring si pentru load balancer. */
void free_load_balancer(load_balancer* main)
{
    unsigned int i;
    for (i = 0; i < main->size; ++i) {
        if (main->hashring[i].hash ==
            hash_function_servers(&main->hashring[i].tag))
            free_server_memory(main->servers[main->hashring[i].tag]);
    }
    free(main->servers);
    free(main->hashring);
    free(main);
}

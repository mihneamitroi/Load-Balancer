/* Copyright 2021 Mitroi Mihnea-Cristian */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"

#include "hashtable.h"

/* Functie de comparare a cheilor */
int compare_function_strings(void *a, void *b)
{
	char *str_a = (char *)a;
	char *str_b = (char *)b;

	return strcmp(str_a, str_b);
}

/* Functie apelata dupa alocarea unui hashtable pentru a-l initializa. */
hashtable_t *ht_create(unsigned int hmax, unsigned int (*hash_function)(void*),
		int (*compare_function)(void*, void*))
{
	unsigned int i;
	hashtable_t *hashtable =  malloc(sizeof(*hashtable));
    DIE(hashtable == NULL, "Malloc failed");
	hashtable->buckets = malloc(hmax * sizeof(hashtable->buckets));
    DIE(hashtable->buckets == NULL, "Malloc failed");
	for (i = 0; i < hmax; ++i)
		hashtable->buckets[i] = ll_create(sizeof(info));
    hashtable->hmax = hmax;
    hashtable->size = 0;
    hashtable->hash_function = (*hash_function);
    hashtable->compare_function = (*compare_function);
    return hashtable;
}

/* Functie care cauta o anumita cheie intr-un bucket si intoarce pozitia la
care aceasta se afla, in caz ca exista. */
static ll_node_t *find_key(linked_list_t *bucket, void *key,
	int (*compare_function)(void*, void*), unsigned int *pos)
{
    ll_node_t *node = bucket->head;
    unsigned int i;
	for (i = 0; i < bucket->size; ++i) {
		if (compare_function(key, ((info*)node->data)->key) == 0) {
			*pos = i;
			return node;
		}
		node = node->next;
	}
	return NULL;
}

/* Functie care stocheaza in hashtable o pereche cheie - valoare. */
void ht_put(hashtable_t *ht, void *key, unsigned int key_size,
	void *value, unsigned int value_size)
{
	info data;
	unsigned int idx = ht->hash_function(key) % ht->hmax, pos;
	ll_node_t *node = find_key(ht->buckets[idx],
							   key, ht->compare_function, &pos);
	if (!node) {
        data.key = malloc(key_size);
		DIE(data.key == NULL, "Malloc failed");
        data.value = malloc(value_size);
		DIE(data.value == NULL, "Malloc failed");
		memcpy(data.key, key, key_size);
		memcpy(data.value, value, value_size);
		ll_add_nth_node(ht->buckets[idx], 0, &data);
		ht->size++;
    } else {
		free(((info*)node->data)->value);
		((info*)node->data)->value = malloc(value_size);
		memcpy(((info*)node->data)->value, value, value_size);
    }
}

/* Functie care returneaza valoarea asociata unei chei, in caz ca aceasta
exista. */
void *ht_get(hashtable_t *ht, void *key)
{
    unsigned int pos, idx = ht->hash_function(key) % ht->hmax;
    ll_node_t *node = find_key(ht->buckets[idx],
							   key, ht->compare_function, &pos);
    if (node != NULL) {
        return ((info*)node->data)->value;
	} else {
		return NULL;
	}
}

/* Functie care intoarce 1, daca pentru cheia care a fost asociata anterior
exista o valoare in hashtable si 0, altfel. */
int ht_has_key(hashtable_t *ht, void *key)
{
	unsigned int i, pos;
	for (i = 0; i < ht->hmax; ++i) {
		if (find_key(ht->buckets[i], key, ht->compare_function, &pos) != NULL)
	    	return 1;
	}
	return 0;
}

/* Functie care elimina din hashtable intrarea asociata cheii key. */
void ht_remove_entry(hashtable_t *ht, void *key)
{
	unsigned int idx = ht->hash_function(key) % ht->hmax, pos;
	linked_list_t *bucket = ht->buckets[idx];
	ll_node_t *node = find_key(bucket, key, ht->compare_function, &pos);
	ll_remove_nth_node(bucket, pos);
	free(((info*)node->data)->key);
	free(((info*)node->data)->value);
	free(node->data);
	free(node);
}

/* Functie care elibereaza memoria folosita de toate intrarile din hashtable
dupa care elibereaza si memoria folosita
pentru a stoca structura hashtable. */
void ht_free(hashtable_t *ht)
{
	unsigned int i;
	for (i = 0; i < ht->hmax; ++i) {
		ll_node_t *node = ht->buckets[i]->head, *prev;
		while (node != NULL) {
			prev = node;
			node = node->next;
			free(((info*)prev->data)->key);
			free(((info*)prev->data)->value);
			free(prev->data);
			free(prev);
			ht->buckets[i]->size--;
		}
		free(ht->buckets[i]);
	}
	free(ht->buckets);
	free(ht);
}




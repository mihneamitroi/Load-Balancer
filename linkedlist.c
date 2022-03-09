/* Copyright 2021 Mitroi Mihnea-Cristian */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "linkedlist.h"
#include "utils.h"

static int min(int a, int b)
{
    if (a <= b)
        return a;
    else
        return b;
}

/* Functia aloca si initializeaza o lista, setandu-i campurile pe 0 sau
    NULL, in functie de tipul campului. */
linked_list_t *ll_create(unsigned int data_size)
{
    linked_list_t *list = malloc(sizeof(*list));
    DIE(list == NULL, "Malloc error");
    list->head = NULL;
	list->size = 0;
	list->data_size = data_size;
    return list;
}

/* Functia parcurge lista de noduri si returneaza nodul de pe pozitia n. */
static ll_node_t* get_nth_node(unsigned int n, linked_list_t* list)
{
    unsigned int i;
    ll_node_t* node = list->head;
    n = min(n, list->size - 1);
    for (i = 0; i < n; ++i)
        node = node->next;
    return node;
}

/* Functia creeaza un nou nod ce stocheaza datele date ca parametru si apoi
il leaga la restul listei al carei pointer este transmis pe pozitia potrivita.
*/
void ll_add_nth_node(linked_list_t* list, unsigned int n, const void* new_data)
{
    ll_node_t *node, * new_node = malloc(sizeof(*new_node));
    DIE(new_node == NULL, "new_node malloc");
    new_node->data = malloc(list->data_size);
    DIE(new_node->data == NULL, "new_node->data malloc");
    memcpy(new_node->data, new_data, list->data_size);
    if (!n || list->head == NULL) {
        new_node->next = list->head;
        list->head = new_node;
    } else {
        node = get_nth_node(n - 1, list);
        new_node->next = node->next;
        node->next = new_node;
    }
    list->size++;
}

/* Functia elimina nodul de pe pozitia n din lista al carei pointer este trimis
ca parametru. */
ll_node_t *ll_remove_nth_node(linked_list_t* list, unsigned int n)
{
    ll_node_t* prev_node, *removed_node;
    if (!list || !list->size)
        return NULL;
    if (!n) {
        removed_node = list->head;
        list->head = list->head->next;
    } else {
        prev_node = get_nth_node(n - 1, list);
        removed_node = prev_node->next;
        prev_node->next = prev_node->next->next;
    }
    list->size--;
    return removed_node;
}

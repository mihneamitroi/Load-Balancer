/* Copyright 2021 Mitroi Mihnea-Cristian */
#ifndef UTILS_H_
#define UTILS_H_

#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

/* Functie pentru verificarea erorilor de alocare. */
#define DIE(assertion, call_description)                                       \
    do {                                                                       \
        if (assertion) {                                                       \
            fprintf(stderr, "(%s, %d): ", __FILE__, __LINE__);                 \
            perror(call_description);                                          \
            exit(errno);                                                       \
        }                                                                      \
    } while (0)

#endif  // UTILS_H_

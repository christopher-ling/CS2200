/*
    A header file to declare macros, type definitions, and functions used in 
    Arraylist.c. The arraylist is not generic; it is built to store pointers 
    to chars (strings) only.

    Authored by Tristan Rogers
*/

#ifndef ARRAYLIST_H
#define ARRAYLIST_H

#include <stdio.h>
#include <stdlib.h>

typedef unsigned int uint;
typedef struct arraylist {
    uint capacity;
    uint size;
    char **backing_array;
} arraylist_t;

arraylist_t *create_arraylist(uint capacity);
void add_at_index(arraylist_t *arraylist, char *data, int index);
void append(arraylist_t *arraylist, char *data);
char *remove_from_index(arraylist_t *arraylist, int index);
void resize(arraylist_t *arraylist);
void destroy(arraylist_t *arraylist);

#endif

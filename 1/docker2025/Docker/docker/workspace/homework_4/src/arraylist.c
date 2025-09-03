/**
 * Name: Christopher Ling
 * GTID: 903813567
 */

/*  PART 2: A CS-2200 C implementation of the arraylist data structure.
    Implement an array list.
    The methods that are required are all described in the header file. 
    Description for the methods can be found there.

    Hint 1: Review documentation/ man page for malloc, calloc, and realloc.
    Hint 2: Review how an arraylist works.
    Hint 3: You can use GDB if your implentation causes segmentation faults.
    Hint 4: Remember to check if memory allocations succeed.

    You will submit this file to gradescope.
*/

#include "arraylist.h"

/* Student code goes below this point */
arraylist_t *create_arraylist(uint capacity) {
    if(capacity == 0) {
        return NULL;
    }

    arraylist_t *arraylist = (arraylist_t *)malloc(sizeof(arraylist_t));
    if (arraylist == NULL) {
        return 0;
    }
    arraylist->backing_array = (char **)malloc(sizeof(char *) * capacity);
    if (arraylist->backing_array == NULL) {
        free(arraylist);
        return 0;
    }

    arraylist->size = 0;
    arraylist->capacity = capacity;
    return arraylist;
}

void add_at_index(arraylist_t *arraylist, char *data, int index) {
    if (arraylist == NULL || data == NULL || index < 0 || index > arraylist->size) {
        return;
    }
    if (arraylist->size == arraylist->capacity) {
        resize(arraylist);
    }
    for (int i = arraylist->size; i > index; i--) {
        arraylist->backing_array[i] = arraylist->backing_array[i - 1];
    }
    arraylist->backing_array[index] = data;
    arraylist->size++;
}
void append(arraylist_t *arraylist, char *data) {
    if (arraylist == NULL || data == NULL) {
        return;
    }
    add_at_index(arraylist, data, arraylist->size);
    return;
}
char *remove_from_index(arraylist_t *arraylist, int index) {
    if (arraylist == NULL || index < 0 || index >= arraylist->size) {
        return 0;
    }
    char *data = arraylist->backing_array[index];
    for (int i = index; i < arraylist->size - 1; i++) {
        arraylist->backing_array[i] = arraylist->backing_array[i + 1];
    }
    arraylist->size--;
    return data;
}
void resize(arraylist_t *arraylist) {
    if (arraylist == NULL) {
        return;
    }
    char **new_backing_array = (char **)realloc(arraylist->backing_array, sizeof(char *) * 2 * arraylist->capacity);
    if (new_backing_array == NULL) {
        return;
    }
    arraylist->capacity = arraylist->capacity * 2;
    arraylist->backing_array = new_backing_array;
}
void destroy(arraylist_t *arraylist) {
    if (arraylist == NULL) {
        return;
    }
    free(arraylist->backing_array);
}
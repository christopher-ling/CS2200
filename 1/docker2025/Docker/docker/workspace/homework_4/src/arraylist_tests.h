/**
 * @file arraylist_tests.h
 * @author Tristan Rogers & Charles Snider
 * @version 1.0
 * @date 2022-02-12
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "arraylist.h"

#define FAILURE 0
#define SUCCESS 1

#define NUM_TESTS 4

int run_tests();
int test_create();
int test_append();
int test_add_at_index();
int test_remove_from_index();
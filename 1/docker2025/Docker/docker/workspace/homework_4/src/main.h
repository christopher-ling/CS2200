/**
 * Name: Rushil Sudunagunta
 * GTID: 123456789
 * added to pass autograder
 */

/**
 * @file main.h
 * @author Andrej Vrtanoski & Charles Snider
 * @bug fixed the dictionary length
 * @version 3.0
 * @date 2024-02-10
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "arraylist.h"
#include "arraylist_tests.h"

int dictionary_length = 15; 
char *dictionary[] = {
    "this",
    "rocks",
    "tea",
    "juice",
    "is",
    "secret",
    "nothing",
    "correct",
    "more",
    "cs2200",
    "than",
    "tunnel",
    "hot",
    "momo",
    "leaf"
};

int main(int argc, char *argv[]);
char *generateMessage();
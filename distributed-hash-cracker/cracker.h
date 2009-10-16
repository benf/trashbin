#ifndef CRACK_H
#define CRACK_H

#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <crypt.h>

#define MD5	1
#define SHA1	2

// Define a cracing task and all attribus which related on this
typedef struct {
	int base;
	int keysize_max;
	char* charset;
	char* hash;
	char* salt;
	int algorithm;

	//additional information
	unsigned long long int keyrange;

	// for splitting calculation, e.g. threading or distributed computing
	char* start_key;
	char* end_key;
} crack_task;

unsigned long long int keyrange(crack_task);
int keynr_2_key(crack_task, int, char*);
int get_next_key(crack_task, char*, int);
int ben_next_key(crack_task, char*);
int compare_hash(char*, char*);

#endif

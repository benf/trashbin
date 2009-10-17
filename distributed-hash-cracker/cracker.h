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
	unsigned int base;
	unsigned int keysize_max;
	char* charset;
	char* hash;
	char* salt;
	unsigned int algorithm;

	//additional information
	unsigned long long int keyrange;

	// for splitting calculation, e.g. threading or distributed computing
	char* start_key;
	unsigned int keyarea;
} crack_task;

unsigned long long int keyrange(crack_task);
int keynr_2_key(crack_task, int, char*);
int get_next_key(crack_task, char*, int);
int ben_next_key(crack_task, char*);
int compare_hash(char*, char*);
void usage(void);

#endif

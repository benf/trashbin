#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
//#include <crack.h>

#define MD5	1
#define SHA1	2

typedef struct {
	int base;
	int keysize;
	char* charset;
	char* salt;
	int algorithm;

	// for splitting calculation, e.g. threading or distributed computing
	char* start_key;
	char* end_key;
} crack_task;

typedef struct {
	char* start_key;
	char* end_key;
} task_split;

int keynr_2_key( crack_task, int, char*);

int main( int argc, char **argv)
{
	crack_task crack;

	char *key;
	int key_nr = 365;

	crack.base = 3;
	crack.keysize = 6;

	key = (char*) calloc(sizeof(char), max_keysize);
	crack.charset = (char*) calloc(sizeof(char), base);

	strncpy( crack.charset, "abc", crack.base);

	keynr_2_key(crack, key_nr, key);

	printf("Keynumber: %d key: %s\n", key_nr, key);

	free(key);

	return 0;
}

// convert a position of a key in the keyrange to the spezific key
int keynr_2_key( crack_task crack, int key_nr, char *key)
{
	int char_nr = 0;
	int i = 0;
	while(key_nr != 0)
	{
		char_nr = key_nr % crack.base;
		key_nr = key_nr / crack.base;
		key[i] = crack.charset[char_nr];
		i++;
	}

	return 0;
}

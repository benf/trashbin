#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
//#include <crack.h>

#define MD5	1
#define SHA1	2

// Define a cracing task
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

int keynr_2_key(crack_task, int, char*);
int get_next_key(crack_task, char*, int);

int main( int argc, char **argv)
{
	crack_task crack;

	char *key;
	int key_nr = 365;

	crack.base = 3;
	crack.keysize = 6;

	key = (char*) calloc(sizeof(char), crack.keysize);
	crack.charset = (char*) calloc(sizeof(char), crack.base);

	strncpy( crack.charset, "abc", crack.base);

	while(get_next_key(crack, key, 0) != -1)
		printf("%s\n", key);

	keynr_2_key(crack, key_nr, key);

	printf("Keynumber: %d key: %s\n", key_nr, key);

	get_next_key(crack, key, 0);

	printf("next key: %s\n", key);

	
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

//increas the key by one
int get_next_key(crack_task crack, char* key, int pos)
{
	int i = 0;
	
	for(i = 0; i < crack.base; ++i)
	{
		if(key[pos] == crack.charset[i])
		{
			if((i + 1) < crack.base)
			{
				key[pos] = crack.charset[i+1];
				return 0;
			}
			else
			{
				key[pos] = crack.charset[0];
				if((pos + 1) < crack.keysize)
					return get_next_key(crack, key, pos + 1);
				else
					return -1;;
			}
		}
	}

	key[pos] = crack.charset[0];

	return 0;
}


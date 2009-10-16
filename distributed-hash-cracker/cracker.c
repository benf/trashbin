#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <crypt.h>

#include "cracker.h"

int main( int argc, char **argv)
{
	crack_task crack;
	char *key;
	int i = 0;
	int counter = 0;
	crack.hash = (char*) calloc(sizeof(char), 150+1);
	strncpy( crack.hash, "$6$qSR2U5h6$sWgBeng0MV80FRpFBNG9SQjFOwxDOPF7WNZchhifRQKXuxTnhptVR4y5A4YbMFya8qnSdic1UH0KoN2pMIl6O0", 150);
	crack.base = 21;
	crack.keysize_max = 4;

	key = (char*) calloc(sizeof(char), crack.keysize_max +1);
	crack.charset = (char*) calloc(sizeof(char), crack.base +1);

	strncpy( crack.charset, "abcdefghijklmnopqrstu", crack.base);

	crack.keyrange = keyrange(crack);
	printf("KeyRange: %d\n", crack.keyrange);

	do
	{
		++counter;
		if(compare_hash(key, crack.hash) == 1)
			printf("Find: %s\n", key);
	}
	while(get_next_key(crack, key, 0) == 0);
	while(ben_next_key(crack, key) == 0);

	printf("counter: %d\n", counter);

	free(key);
	free(crack.charset);

	return 0;
}

//calculate the keyrange
unsigned long long int keyrange(crack_task crack)
{
	int i;
	int keyrange = 0;
	for(i = 0; i <= crack.keysize_max; ++i)
		keyrange += pow(crack.base, i);
	
	return keyrange;
}

// convert a position of a key in the keyrange to the spezific key
int keynr_2_key(crack_task crack, int key_nr, char *key)
{
	int char_nr = 0;
	int i = 0;
	int keylen = 0;

	if(key_nr == 0)
	{
		key[0] = '\0';
		return 0;
	}

	for(keylen = 0; key_nr >= pow(crack.base, keylen); ++keylen)
		key_nr -= pow(crack.base, keylen);

	do
	{
		char_nr = key_nr % crack.base;
		key_nr = key_nr / crack.base;
		key[i] = crack.charset[char_nr];
		i++;
	}
	while(key_nr != 0 || strlen(key) != keylen);

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
				if((pos + 1) < crack.keysize_max)
					return get_next_key(crack, key, pos + 1);
				else
					return -1;;
			}
		}
	}

	key[pos] = crack.charset[0];

	return 0;
}

//ben's version of next string
int ben_next_key(crack_task crack, char *key) {
	int i;
	int l = strlen(key);
	for (i = 0; i < l; ++i) { 
		if (key[i] == crack.charset[crack.base-1]) {
			if (i >= crack.keysize_max-1)
				return -1;
			key[i] = crack.charset[0];
			if (i == l-1) {
				key[i+1] = crack.charset[0];
				// should be ensured by calloc, but safety first!
				key[i+2] = '\0';
				// not needed (for loop would end NOW also without this) - but for readability of the algorithm
				break;
			}
		} else {
			key[i] = *(strchr(crack.charset, key[i])+1);
			break;
		}
	}
	// just a little feature, if a empty key is given
	// the first character in the base is used as first key
	if (l == 0)
		key[0] = crack.charset[0];
	return 0;
}

//compare
int compare_hash(char* key, char* hash)
{
	char* key_hash;
	int result;
	
	key_hash = (char*) crypt(key, hash);

	if(strncmp(key_hash, hash, strlen(key_hash)) == 0)
		result = 1;
	else
		result = 0;

	return result;
}

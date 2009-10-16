#include <unistd.h>
#include <getopt.h>

#include "cracker.h"

int main( int argc, char **argv)
{
	crack_task crack;
	int option = 0;
	int optionindex = 0;
	int i = 0;

	static struct option long_options[] =
	{
		{"help"		, 0, NULL, 'h'},
		{"hash"		, 1, NULL, 'a'},
		{"charset"	, 1, NULL, 'c'},
		{"length"	, 1, NULL, 'l'},
		{"file"		, 1, NULL, 'f'},
		{0		, 0, 0, 0}
	};

	while((option = getopt_long( argc, argv, "h:c:l:f:", long_options, &optionindex)) != -1)
	{
		switch (option)
		{
			case 'a':
				crack.hash = (char*) calloc(sizeof(char), strlen(optarg) +1);
				strncpy(crack.hash, optarg, strlen(optarg));
				printf("hash: %s\n", crack.hash);
				break;

			case 'c':
				crack.charset = (char*) calloc(sizeof(char), strlen(optarg));
				strncpy(crack.charset, optarg, strlen(optarg));
				printf("charset: %s\n", crack.charset);
				break;
			
			case 'l':
				for(i = 0; i < strlen(optarg); ++i)
					if(isdigit(optarg[i]) == 0)
						return -1;

				crack.keysize_max = atoi(optarg);
				printf("Keysize: %d\n", crack.keysize_max);
				break;

			case 'f':
				printf("File: %s\nThis option is not implemented!\n", optarg);
				break;
			case 'h':
			default:
				usage();
				break;
		}
	}

	exit(0);

	char *key;
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

void usage(void)
{
	const char* usage_str =
	"usage: cracker <option>\n\n"
	"Option                   Discription\n\n"
	"-h, --hash <HASH>        Define a spezific hash to crack\n"
	"-c, --charset <CHARSET>  Define charset which the searched\n"
	"                         password consists of.\n"
	"-l, --length <NUM>       Maximum password length\n"
	"-f, --file <FILE>        Password file (like /etc/shadow)\n";

	printf("%s", usage_str);
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
	key_hash = (char*) crypt(key, hash);

	if(strncmp(key_hash, hash, strlen(key_hash)) == 0)
		return 1;
	
	return 0;
}

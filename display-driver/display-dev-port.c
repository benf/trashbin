#include <stdio.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <errno.h>
extern int errno;
#include <string.h>

#define ADDR 55296
//#define ADDR 888
//#define ADDR 0x378

void write_port(int, off_t, int);
void print_error();

int main()
{
	int fh = 0;
	int i = 0;

	fh = open( "/dev/port", O_RDWR);

	if(fh == -1)
	{
		printf("Could not open file.\n");
		print_error();
		return -1;
	}
	
	for( i = 0; i < 3; ++i)
	{
		write_port(fh, ADDR, 0x30);
	}

	close(fh);

	return 0;
}

void write_port( int fh, off_t address, int data)
{
	int status = lseek(fh, address, SEEK_SET);

	printf("Status seek: %d, address: 0x%x, %d\n", status, address, address);

	status = write(fh, (char) data, 1);

	printf("Status write: %d Data: 0x%x, %d\n", status, data, data);

	print_error();

	sleep(1);
}

void print_error(void)
{
	char *errstr;
	errstr = strerror( errno);
	printf("%s\n", errstr);
}


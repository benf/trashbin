#include <stdio.h>
#include <stdint.h>
#include <ieee1284.h>

void write_number(uint8_t num);
void write_binary_data(uint16_t data);
void print_status_open(int);

struct parport* curr_port;
struct parport_list port_list;


// left number
// upper
#define PIN_A1 (1 << )
// lower 
#define PIN_B1 (1 << )

// left dot 
#define PIN_F1 (1 << )

// right number
//topmost
#define PIN_A2 (1 << 2)
//upper left
#define PIN_B2 (1 << 0)
//lower left
#define PIN_C2 (1 << 4)
//bottom
#define PIN_D2 (1 << 6)
//lower left
#define PIN_E2 (1 << 5)
//upper right
#define PIN_F2 (1 << 3)
//middle
#define PIN_G2 (1 << 1)

// right dot
#define PIN_H2 (1 << )


// positive sign
// upper
#define PIN_D1 (1 << )
// lower 
#define PIN_E1 (1 << )

// negative sign
#define PIN_C1 (1 << )



int main(int argc, char **argv)
{
  if (argc < 2) {
    fprintf(stderr, "usage: %s <data>\n", argv[0]);
    return -1;
  }

  uint32_t status;
  uint32_t capability;

  int i;
	status = ieee1284_find_ports( &port_list, 0);

	if( status == E1284_NOMEM)
	{
		perror("There is not enought memory.\n");
		return -1;
	}

	printf("Number of ports: %d\n\n", port_list.portc);
/*	
	for(i = 0; i < port_list.portc; ++i)
	{
		curr_port = port_list.portv[i];
		printf("Name: \t\t%s\n", curr_port->name);
		printf("Base address: \t%d\n", curr_port->base_addr);
		printf("ERC address: \t%d\n", curr_port->hibase_addr);
		printf("Filename: \t%s\n\n", curr_port->filename);
		//break;
		
	}
*/
	curr_port = port_list.portv[1];
	capability = CAP1284_RAW;

	printf("Try to open port \"%s\"\n\n", curr_port->name);

	status = ieee1284_open(curr_port, 0, &capability);
	
	print_status_open(status);

	status = ieee1284_claim(curr_port);
	printf("Status: %d\n", status);

	write_number(atoi(argv[1]));

/*
	uint16_t info;

	sscanf(argv[1], "%x", &info);
	write_binary_data(info);
*/
	

	ieee1284_release(curr_port);
	status = ieee1284_close(curr_port);
	printf("Status: %d\n", status);

	ieee1284_free_ports(&port_list);

	return 0;
}


void write_number(uint8_t num) {
	uint16_t data = 0;
	switch (num) {
		case 0:
			data = PIN_A2 | PIN_B2 | PIN_C2 | PIN_D2 | PIN_E2 | PIN_F2;
			break;
		case 1:
			data = PIN_B2 | PIN_C2;
			break;
		case 2:
			data = PIN_A2 | PIN_B2 | PIN_G2 | PIN_E2 | PIN_D2;
			break;
		case 3:
			data = PIN_A2 | PIN_B2 | PIN_G2 | PIN_C2 | PIN_D2;
			break;
		case 4:
			data = PIN_F2 | PIN_G2 | PIN_B2 | PIN_C2;
			break;
		case 5:
			data = PIN_A2 | PIN_F2 | PIN_G2 | PIN_C2 | PIN_D2;
			break;
		case 6:
			data = PIN_A2 | PIN_F2 | PIN_G2 | PIN_C2 | PIN_D2 | PIN_E2;
			break;
		case 7:
			data = PIN_A2 | PIN_B2 | PIN_C2;
			break;
		case 8:
			data = PIN_A2 | PIN_B2 | PIN_C2 | PIN_D2 | PIN_E2 | PIN_F2 | PIN_G2;
			break;
		case 9:
			data = PIN_A2 | PIN_B2 | PIN_C2 | PIN_D2 | PIN_F2 | PIN_G2;
			break;
		case 10:
			data = PIN_A2 | PIN_B2 | PIN_C2 | PIN_E2 | PIN_F2 | PIN_G2;
			break;
		case 11:
			data = PIN_C2 | PIN_D2 | PIN_E2 | PIN_F2 | PIN_G2;
			break;
		case 12:
			data = PIN_A2 | PIN_E2 | PIN_F2 | PIN_D2;
			break;
		case 13:
			data = PIN_B2 | PIN_G2 | PIN_C2 | PIN_D2 | PIN_E2;
			break;
		case 14:
			data = PIN_A2 | PIN_D2 | PIN_E2 | PIN_F2 | PIN_G2;
			break;
		case 15:
			data = PIN_A2 | PIN_E2 | PIN_F2 | PIN_G2;
			break;

	}

	write_binary_data(data);
		
}

void write_binary_data(uint16_t data) {
  uint8_t ctrl = ((data & 0x0F00) >> 8) & 0xFF;
  uint8_t _data = data & 0xFF;

  ieee1284_write_control(curr_port, ctrl);
  ieee1284_write_data(curr_port, _data);
}

void print_status_open( int status)
{
	switch(status)
	{
		case E1284_OK:
			printf("Port is open.");
			break;

		case E1284_INIT:
			printf("There was a problem during port initialization.\nThis could be because another driver has opened the port exclusively,\nor some other reason.\n");
			break;

		case E1284_NOTAVAIL:
			printf("One or more of the supplied flags is not supported by this type of port.\n");
			break;

		case E1284_INVALIDPORT:
			printf("The port parameter is invalid (for instance, the port may already be open).\n");
			break;
			
		default:
			printf("unkown error.\n");
			break;
	}
}


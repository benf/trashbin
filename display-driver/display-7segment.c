#include <stdio.h>
#include <stdint.h>
#include <ieee1284.h>

void write_data(uint16_t data);
void print_status_open(int);

struct parport* curr_port;
struct parport_list port_list;

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
	capability = CAP1284_RAW;

	printf("Try to open port \"%s\"\n\n", curr_port->name);

	status = ieee1284_open(curr_port, 0, &capability);
	
	print_status_open(status);

	status = ieee1284_claim(curr_port);
	printf("Status: %d\n", status);

  uint16_t info;
  sscanf(argv[1], "%x", &info);

  write_data(info);

	ieee1284_release(curr_port);
	status = ieee1284_close(curr_port);
	printf("Status: %d\n", status);

	ieee1284_free_ports(&port_list);

	return 0;
}

void write_data(uint16_t data) {
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


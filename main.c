#include <jnxc_headers/network.h>
#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <signal.h>
#include <string.h>
#include "data_translation.h"
#include "executor.h"
/*
 * This program is designed to be so simple it is almost instantly understandable
 * It can operate in send or receive mode, where upon it transfers a simple char* protocol, using a predetermined delimiter
 * i.e xcode do some cool stuff -f 1902# build the project# post build steps
 * On the receiver side it will process the input steps via the executor which receieves a data_parcel (data_parcel is a struct made from the input 
 * protocol
 */
void catch_int (int signum) 
{
    pid_t my_pid;
    printf("\nReceived an interrupt! About to exit ..\n");
	cancel_listener();
    fflush(stdout);
    my_pid = getpid();
    kill(my_pid, SIGKILL);
}

void server_update(char *received_msg)
{
	printf("Raw received message: %s\n",received_msg);
	struct data_parcel *p = data_from_message(received_msg);
	if(execute_data_parcel(p) != 0)
	{
		printf("Error during execution step\n");
		free(p);
		exit(1);
	}
	free(p);
	printf("Execution completed\n");
}
int main(int argc, char **argv) 
{
	//Register for signal handling
	signal(SIGINT, catch_int);
	
	int i;
	int port;
	char* mode = NULL;
	char* host = NULL;
	char *inputstr = NULL;
	
	while(( i = getopt(argc, argv,"h:i:m:p:")) != -1)
	{
		switch(i)
		{
			case 'i':
			inputstr = optarg;
			break;
			case 'h':
			host = optarg;
			break;
			case 'm':
			mode = optarg;
			break;
			case 'p':
			port = atoi(optarg);
			break;
			case '?':
			if(optopt == 'p')
			{
				fprintf(stderr,"Option -%c requires an argument for port\n",optopt);
				return -1;
			}
			if(optopt == 'm')
			{
				fprintf(stderr,"Option -%c requires an argument for mode LISTEN/SEND\n",optopt);
				return -1;
			}
			else if (isprint(optopt))
			{
				abort();
				fprintf(stderr, "Unknown option `-%c .\n",optopt);
				return -1;abort();
			}
			default:
				printf("Requires argument for operation mode -m [SEND or RECEIVE]\n");
				printf("Requires argument for -p\n");
				abort();
			
		}
	}
	
	if(mode == NULL)
	{
		printf("No mode selected, please try again using -m\n");
		return 1;
	}
	
	if(strcmp(mode,"LISTEN") == 0)
	{
		if(!port) { printf("Requires port number, option -p\n");return 1; };
		
		printf("Listener mode\n");
		//******LISTENER MODE**********//
		printf("Starting server on port %d\n",port);
		Callback c = &server_update;
		setup_listener(port,c);
		//****************************//
		return 0;
	}
	if(strcmp(mode,"SEND") == 0)
	{
		if(!port) { printf("Requires port number, option -p\n");return 1; };
		if(host == NULL) { printf("Requires hostname, option -h\n");return 1; };
		if(inputstr == NULL) { printf("Requires input string, option -i\n"); return 1; };
		printf("Send mode\n");
		printf("Target host -> %s\n",host);
		printf("Target port -> %d\n",port);
		printf("Target message -> %s\n",inputstr);
		//******SENDER MODE**********//
		send_message(host,port,inputstr);
		//**************************//
		return 0;
	}
	
    return 0;
}

#include <jnxc_headers/jnxnetwork.h>
#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <signal.h>
#include <string.h>
#include <jnxc_headers/jnxlist.h>

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
	jnx_cancel_listener();
    fflush(stdout);
    my_pid = getpid();
    kill(my_pid, SIGKILL);
}

void server_update(char *received_msg)
{
	printf("Raw received message: %s of length %d\n",received_msg,(int)strlen(received_msg));
	
	char buffer[strlen(received_msg)]; //safety net from unsafe violations
	strcpy(buffer,received_msg);
	printf("String made safe %s\n",buffer);
	
	char *pch = strtok (buffer,"#");	
 	while (pch != NULL)
 	{
 		printf ("%s\n",pch);
		system(pch);
 		pch = strtok (NULL, "#");
 	}

	printf("Execution completed\n");
}

char* getstring_from_file(char*filepath)
{
	FILE *f;
	if((f = fopen(filepath,"r")) == NULL)
	{
		printf("Cannot open file\n");
		exit(0);
	}
    char line[256];
	char *request_builder = malloc(1024);
    while (fgets(line, sizeof(line), f)) 
	{
        /* note that fgets don't strip the terminating \n, checking its
           presence would allow to handle lines longer that sizeof(line) */
		
		//removing end of lines
		line[strlen(line) -1] = 0;
		strcat(request_builder,line);
		strcat(request_builder,"#");
		request_builder = realloc(request_builder,sizeof(request_builder) + sizeof(line));
    }
    fclose(f);
	return request_builder;
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
		printf("Satellite is a half duplex server/client in one for transmission of several shell commands\n");
		printf("No mode selected, please try again using -m\n");
		printf("Using -m LISTEN will enable listener mode where you will be asked to provide -p [PORT]\n");
		printf("Using -m SEND will enable send mode where you will be asked to provide -h [HOSTNAME] -p [PORT] -i [filepath]\n");
		return 1;
	}
	
	if(strcmp(mode,"LISTEN") == 0)
	{
		if(!port) { printf("Requires port number, option -p\n");return 1; };

		printf("Listener mode\n");
		//******LISTENER MODE**********//
		printf("Starting server on port %d\n",port);
		Callback c = &server_update;
		jnx_setup_listener(port,c);
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
		printf("Target file -> %s\n",inputstr);
		 
		//******SENDER MODE**********//		
		//getstring_from_file(inputstr);
		printf("%s\n",getstring_from_file(inputstr));
		jnx_send_message(host,port,getstring_from_file(inputstr));
		//**************************//
		return 0;
	}
	
    return 0;
}



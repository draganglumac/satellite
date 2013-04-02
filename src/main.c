#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <signal.h>
#include <string.h>
#include <sys/stat.h>
#include "backend/jnxsql_interface.h"
#include <jnxc_headers/jnxstring.h>
#include <jnxc_headers/jnxlist.h>
#include <jnxc_headers/jnxfile.h>
#include <jnxc_headers/jnxhash.h>
#include "network/jnx_transmitter.h"
#include "network/jnx_receiver.h"
#include "utils.h"
#define TIMEWAIT 5
jnx_hashmap *config;
enum ERROR_CODE { SER_UP, ARG_UP,SQL_UP }; 
#define DEFAULTCOLOR -1
void usage()
{
	print_streams(DEFAULTCOLOR,"Satellite is a half duplex server/client in one for transmission of several shell commands\n");
	print_streams(DEFAULTCOLOR,"No mode selected, please try again using -m [TRANSMIT,RECEIVE]\n");
}
char *conf_path()
{
	char *home_path = getenv("HOME");
	char *buffer = malloc(2048);
	strcpy(buffer,home_path);
	strcat(buffer,"/.satellite/satellite.conf");
	return buffer;
}
void catch_int (int signum) 
{ 
	pid_t my_pid;
	print_streams(DEFAULTCOLOR,"\nReceived an interrupt! About to exit ..\n");

	jnx_network_cancel_listener();
	fflush(stdout);
	my_pid = getpid();
	kill(my_pid, SIGKILL);	
	system("sudo killall -9 satellite");
}
int main(int argc, char **argv) 
{
	//Register for signal handling
	signal(SIGINT, catch_int);
	static struct option long_options[] = 
	{
		{"mode",required_argument,0,'m'},
		{0,      0,                 0, 0 }
	};
	int option_index = 0;
	int i;
	char* mode = NULL;
	while(( i = getopt_long_only(argc,argv,"m:",long_options,&option_index)) != -1)
	{
		switch(i)
		{
			case 'm':
				mode = optarg;
				break;
			default:
				usage();
				exit(1);
		}
	}
	if(mode == NULL)
	{
		usage();
		return ARG_UP;
	}
	char *_conf = conf_path();
	printf("Configuration path %s\n",_conf);
	config = utils_set_configuration(_conf);
	free(_conf);
	/*-----------------------------------------------------------------------------
	 *  Setup our log
	 *-----------------------------------------------------------------------------*/
	if(jnx_log_setup(jnx_hash_get(config,"logpath")) != 0)
	{
		jnx_term_printf_in_color(JNX_COL_RED,"WARNING: Could not start logger\n");
		return 1;
	}
	print_streams(JNX_COL_GREEN,"Satellite Started\n");
	print_streams(DEFAULTCOLOR,"Storing SQL credentials temporarily\n");
	/*-----------------------------------------------------------------------------
	 *  Store sql credentials
	 *-----------------------------------------------------------------------------*/
	if(perform_store_sql_credentials(jnx_hash_get(config,"sqlhost"),jnx_hash_get(config,"sqluser"),jnx_hash_get(config,"sqlpass")) != 0)
	{
		print_streams(JNX_COL_RED,"Error with sql credentials\n");
		return 1;
	}
	if(strcmp(mode,"RECEIVE") == 0)
	{
		if(!jnx_hash_get(config,"listenport"))
		{ print_streams(JNX_COL_RED,"Requires port number, option -p\n");return 1; };
		if(jnx_start_listener(jnx_hash_get(config,"listenport")) != 0)
		{
			print_streams(JNX_COL_RED,"Error starting the listener\n");
			return 1;
		}
	}
	if(strcmp(mode,"TRANSMIT") == 0)
	{
		jnx_start_transmitter();        
	}
	jnx_hash_delete(config);
	return 0;
}



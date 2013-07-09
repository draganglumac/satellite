#include "logic/job_control.h"
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <jnxc_headers/jnxnetwork.h>
#include <jnxc_headers/jnxterm.h>
void catch_int (int signum) 
{ 
	pid_t my_pid;
	jnx_term_printf_in_color(JNX_COL_RED,"\nReceived an interrupt! About to exit ..\n");
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
	signal(SIGTSTP,catch_int);
	job_control_start_listening();	

	return 0;
}


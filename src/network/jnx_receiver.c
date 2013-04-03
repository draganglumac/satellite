/*
 * =====================================================================================
 *
 *       Filename:  jnx_receiver.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  03/26/13 15:36:06
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alex Jones (), alexsimonjones@gmail.com
 *   Organization:  
 *
 * =====================================================================================
 */
#include "jnx_receiver.h"
#include <stdlib.h>
#include <stdio.h>
#include "../logic/jnx_results.h"
#include "../utils.h"
#include <string.h>
#include <unistd.h>
void jnx_receiver_listener_callback(char *message_buffer)
{

	//create an output directory
	// remember that user commands may change the cwd!
	int output_setup_complete = jnx_result_setup();
	char working_directory[1024];
	getcwd(working_directory,1024);	
	printf("Working base directory %s\n", working_directory);
	print_streams(DEFAULTCOLOR,"Raw received message: %s of length %d\n",message_buffer,(int)strlen(message_buffer));
	char *delimiter = "!";
	char *job_id = NULL;
	char *command = NULL;
	char cp[1024];
	strcpy(cp,message_buffer);
	char *token = strtok(cp,delimiter);
	command = token;
	print_streams(DEFAULTCOLOR,"COMMAND: %s\n",command);
	token = strtok(NULL,delimiter);
	//We should really chop off the job so it doesnt come out as a system error
	job_id = token;
	print_streams(DEFAULTCOLOR,"Job ID: %s\n",job_id);
	int ret = system(command);
	if(ret != 0)
	{
		print_streams(DEFAULTCOLOR,"Error with execution of %s : System returned %d\n",message_buffer,ret);
	}
	print_streams(JNX_COL_GREEN,"Execution completed\n");

	if(output_setup_complete == 0)
	{
		jnx_result_process(job_id);
		jnx_result_teardown();
	}
	else
	{
		printf("Error with results processing, probably means the directory already exists!\n");
	}
}
int jnx_start_listener(char *listener_port)
{
	jnx_network_listener_callback c = &jnx_receiver_listener_callback;
	return jnx_network_setup_listener(atoi(listener_port),c);
}

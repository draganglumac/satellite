/*
 * =====================================================================================
 *
 *       Filename:  job_control.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  09/07/2013 10:47:22
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *   Organization:  
 *
 * =====================================================================================
 */
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "job_control.h"
#include "result_control.h"
#include <jnxc_headers/jnxnetwork.h>
#include <stdarg.h>
#include <jnxc_headers/jnxhash.h>
#include <jnxc_headers/jnxstring.h>

int query(char *hostaddr, char* hostport, const char *template, ...)
{
	int max_len = 2 * 1024 * 1024;
	char query[max_len];
	va_list ap;
	va_start(ap,template);
	vsprintf(query,template,ap);
	va_end(ap);
	jnx_network_send_message(hostaddr,atoi(hostport),query,strlen(query));
	return 0;
}
void message_intercept(char *message, size_t msg_len, char *ip)
{
	api_command_obj *obj = transaction_api_create_obj(message);
	if(obj == NULL)
	{
		printf("Failed to create api_command_obj\n");
		return;
	}
	printf("OBJECT CMD:%d ID:%s DATA:%s OTHER:%s SENDER:%s PORT:%d\n",obj->CMD,obj->ID,obj->DATA,obj->OTHER,obj->SENDER,obj->PORT);
	char *node_ip = jnx_network_local_ip(INTERFACE);
	char *node_port = jnx_string_itos(LISTENPORT);
	/*  Agree to start job - set status to IN PROGRESS */
	char *target_port = jnx_string_itos(obj->PORT);
	query(obj->SENDER,target_port,API_COMMAND,"STATUS",obj->ID,"IN PROGRESS"," ",node_ip,node_port);
	/*  perform job */
	int output_setup_complete = jnx_result_setup();
	int ret = system(obj->DATA);
	if(ret != 0)
	{
		char retbuffer[25];
		sprintf(retbuffer,"%d",ret);
		query(obj->SENDER,target_port,API_COMMAND,"STATUS",obj->ID,"FAILED",retbuffer,node_ip,node_port);
	}
	/*  transmit results  */
	if(output_setup_complete == 0)
	{
		jnx_result_process(obj->SENDER, target_port,obj->ID,node_ip,node_port);
		jnx_result_teardown();
	}
	/*  set status to COMPLETED */
	query(obj->SENDER,target_port,API_COMMAND,"STATUS",obj->ID,"COMPLETED"," ",node_ip,node_port);
	free(node_port);	
	free(target_port);
}
void job_control_start_listening(void)
{
	jnx_network_listener_callback ll = message_intercept;
	jnx_network_setup_listener(LISTENPORT,25,ll);
}

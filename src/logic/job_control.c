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
#include <pthread.h>
#include <jnxc_headers/jnxhash.h>
#include <jnxc_headers/jnxstring.h>
#include <jnxc_headers/jnxlist.h>
#include <jnxc_headers/jnxterm.h>
#define TIME_WAIT sleep(5);
enum processing { WAITING, WORKING };
jnx_list *queue = NULL;
pthread_mutex_t lock;
int lquery(char *hostaddr, char *hostport,size_t data_offset, const char *template, ...)
{
	char *query = malloc(data_offset * sizeof(char) + 1);
	va_list ap;
	va_start(ap,template);
	vsprintf(query,template,ap);
	va_end(ap);
	jnx_network_send_message(hostaddr,atoi(hostport),query,strlen(query));
	free(query);
	return 0;
}
int query(char *hostaddr, char* hostport, const char *template, ...)
{
	char query[1024];
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
	if(queue == NULL)
	{
		queue = jnx_list_init();
	}
	printf("OBJECT CMD:%d ID:%s DATA:%s OTHER:%s SENDER:%s PORT:%d\n",obj->CMD,obj->ID,obj->DATA,obj->OTHER,obj->SENDER,obj->PORT);
	jnx_term_printf_in_color(JNX_COL_BLUE,"Pushing to queue\n");
	pthread_mutex_lock(&lock);
	jnx_list_add(queue,obj);
	pthread_mutex_unlock(&lock);
}
void job_control_process_job(api_command_obj *obj)
{
	char *node_ip = jnx_network_local_ip(INTERFACE);
	char *node_port = jnx_string_itos(LISTENPORT);
	char *target_port = jnx_string_itos(obj->PORT);
	switch(obj->CMD)
	{

		case JOB:
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
			break;
		case SYSTEM:
			printf("Received system command\n");
			system(obj->DATA);
			break;
	}
}
void *job_control_main_loop(void *arg)
{
	while(1)
	{
		if(queue){	
			pthread_mutex_lock(&lock);
			api_command_obj *current_obj = (api_command_obj*) jnx_list_remove(queue);
			pthread_mutex_unlock(&lock);
			if(current_obj != NULL)
			{
				job_control_process_job(current_obj);
				transaction_api_delete_obj(current_obj);
			}

		}
			TIME_WAIT
	}

}

void job_control_start_processing(void)
{
	pthread_t loop_thread;
	pthread_create(&loop_thread,NULL,job_control_main_loop,NULL);
}
void job_control_start_listening(void)
{
	jnx_network_listener_callback ll = message_intercept;
	jnx_network_setup_listener(LISTENPORT,25,ll);
}

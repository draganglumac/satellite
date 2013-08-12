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
#include <jnxc_headers/jnxbase64.h>
#define TIME_WAIT sleep(5);
enum processing { WAITING, WORKING };
jnx_list *queue = NULL;
pthread_mutex_t lock;
int lquery(char *hostaddr, char *hostport,size_t data_offset, const char *template, ...)
{
	char *query = malloc(data_offset +  strlen(template) + 256);
	va_list ap;
	va_start(ap,template);
	vsprintf(query,template,ap);
	va_end(ap);
	return jnx_network_send_message(hostaddr,atoi(hostport),query,strlen(query));
}
int query(char *hostaddr, char* hostport, const char *template, ...)
{
	char query[1024];
	va_list ap;
	va_start(ap,template);
	vsprintf(query,template,ap);
	va_end(ap);
	return jnx_network_send_message(hostaddr,atoi(hostport),query,strlen(query));
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
char *job_temp_log_path()
{
	time_t current_time = time(NULL);
	char _currentwd[256];
	if(getcwd(_currentwd,sizeof(_currentwd)) == NULL)
	{
		perror("job_temp_log_path:");
		return NULL;
	}
	char *path = malloc(sizeof(char)*1024);
	sprintf(path,"%s/%d_consoleoutput.txt",_currentwd,(int)current_time);
	return path;
}
void job_control_process_job(api_command_obj *obj)
{
	char *node_ip = jnx_network_local_ip(INTERFACE);
	if(!node_ip)
	{
		jnx_term_printf_in_color(JNX_COL_RED,"Alert, interface is incorretly configured. Cannot proceed\n");
		return;
	}
	char *node_port = jnx_string_itos(LISTENPORT);
	char *target_port = jnx_string_itos(obj->PORT);
	switch(obj->CMD)
	{
		case JOB:
			jnx_term_printf_in_color(JNX_COL_YELLOW,"Setting job to in progress\n");
			query(obj->SENDER,target_port,API_COMMAND,"STATUS",obj->ID,"IN PROGRESS"," ",node_ip,node_port);
			char *stdout_path = job_temp_log_path();
			if(!stdout_path)
			{
				jnx_term_printf_in_color(JNX_COL_RED,"Error with capture of stdout, could not create file\n");
				return;
			}else{
				printf("Creating console log path %s\n",stdout_path);
				jnx_term_override_stdout(stdout_path);
			}
			/*  perform job */
			jnx_term_printf_in_color(JNX_COL_YELLOW,"Creating results path\n");
			int output_setup_complete = jnx_result_setup();
			jnx_term_printf_in_color(JNX_COL_YELLOW,"Running job via system command\n");
			int ret = system(obj->DATA);
			jnx_term_printf_in_color(JNX_COL_YELLOW,"System command output returned %d\n", ret);
			if(ret != 0)
			{
				
				char retbuffer[25];
				sprintf(retbuffer,"%d",ret);
				jnx_term_printf_in_color(JNX_COL_YELLOW,"Setting job to failed\n");
				query(obj->SENDER,target_port,API_COMMAND,"STATUS",obj->ID,"FAILED",retbuffer,node_ip,node_port);
				jnx_term_reset_stdout();
				if(stdout_path)
				{
					char *console_string;
					size_t outputlen;
					size_t readbytes = jnx_file_read(stdout_path,&console_string);
					if(readbytes > 0){
						char *encoded_string = jnx_base64_encode(console_string,readbytes,&outputlen);
						jnx_term_printf_in_color(JNX_COL_YELLOW,"Sending console log\n");
						if(lquery(obj->SENDER,target_port,outputlen,API_COMMAND,"RESULT",obj->ID,encoded_string,"console_log.txt",node_ip,node_port) != 0)
						{
							jnx_term_printf_in_color(JNX_COL_RED,"Error sending console log\n");	
						}	
						fflush(stdout);
						printf("Send console_log\n");
						free(console_string);
						free(encoded_string);
					}
					free(stdout_path);
				}


				free(node_port);
				free(target_port);
				return;
			}
			/*  transmit results  */
			if(output_setup_complete == 0)
			{
				jnx_term_printf_in_color(JNX_COL_YELLOW,"Sending results\n");
				jnx_result_process(obj->SENDER, target_port,obj->ID,node_ip,node_port);
				jnx_result_teardown();
			}
			/*  set status to COMPLETED */
			jnx_term_printf_in_color(JNX_COL_YELLOW,"Setting job to completed\n");
			query(obj->SENDER,target_port,API_COMMAND,"STATUS",obj->ID,"COMPLETED"," ",node_ip,node_port);
			if(stdout_path)
			{
				char *console_string;
				size_t readbytes = jnx_file_read(stdout_path,&console_string);
				if(readbytes > 0)
				{
					size_t outputlen;
					char *encoded_string = jnx_base64_encode(console_string,readbytes,&outputlen);
					jnx_term_printf_in_color(JNX_COL_YELLOW,"Sending console log\n");
					if(lquery(obj->SENDER,target_port,outputlen,API_COMMAND,"RESULT",obj->ID,encoded_string,"console_log.txt",node_ip,node_port) != 0)
					{
						jnx_term_printf_in_color(JNX_COL_RED,"Error sending console log\n");
					}
					fflush(stdout);
					jnx_term_reset_stdout();
					printf("Send console_log\n");
					free(stdout_path);
					free(console_string);
					free(encoded_string);
				}
				remove(stdout_path);
			}
			free(node_port);	
			free(target_port);
			//Send back console log
			break;
		case SYSTEM:
			jnx_term_printf_in_color(JNX_COL_YELLOW,"Running system command\n");
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

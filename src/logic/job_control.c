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
#include <sys/wait.h>
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
	int written_bytes = jnx_network_send_message(hostaddr,atoi(hostport),query,strlen(query));
	free(query);
	if(written_bytes > 0)
	{
		return 0;
	}else
	{
		return 1;
	}
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
void job_send_status(api_command_obj *obj, char *STATUS,char *node_ip, char *node_port)
{
	jnx_term_printf_in_color(JNX_COL_YELLOW,"Setting job to %s\n",STATUS);
	query(obj->SENDER,jnx_string_itos(obj->PORT),API_COMMAND,"STATUS",obj->ID,STATUS," ",node_ip,node_port);
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
	printf("switching job on command %d\n",obj->CMD);
	switch(obj->CMD)
	{
		case JOB:
			jnx_term_printf_in_color(JNX_COL_YELLOW,"Setting job to in progress\n");
			query(obj->SENDER,target_port,API_COMMAND,"STATUS",obj->ID,"IN PROGRESS"," ",node_ip,node_port);
			jnx_term_printf_in_color(JNX_COL_YELLOW,"Running job via system command\n");
			pid_t process_pid= fork();
			if(process_pid == 0)
			{
				printf("Spawning job in new process\n");
				int ret = system(obj->DATA);
				printf("System returned %d\n",ret);
				if(ret > 0 || ret < 0)
				{
					exit(EXIT_FAILURE);
				}else if(ret == 0)
				{
					exit(EXIT_SUCCESS);
				}
			}else
			{
				printf("Waiting for child process to complete...\n");
				int status;
				do{

					pid_t w = waitpid(process_pid,&status, WUNTRACED | WCONTINUED);
					if(w == -1)
					{
						perror("Error with waitpid");
						exit(EXIT_FAILURE);
					}
					if (WIFEXITED(status)) {
						jnx_term_printf_in_color(JNX_COL_RED,"exited, status=%d\n", WEXITSTATUS(status));
						if(status != 0)
						{
							job_send_status(obj,"FAILED",node_ip,node_port);
						}else if(status == 0)
						{
							job_send_status(obj,"COMPLETED",node_ip,node_port);
						}
					} else if (WIFSIGNALED(status)) {
						jnx_term_printf_in_color(JNX_COL_RED,"killed by signal %d\n", WTERMSIG(status));
						job_send_status(obj,"FAILED",node_ip,node_port);
					} else if (WIFSTOPPED(status)) {
						jnx_term_printf_in_color(JNX_COL_RED,"stopped by signal %d\n", WSTOPSIG(status));
						job_send_status(obj,"FAILED",node_ip,node_port);
					} else if (WIFCONTINUED(status)) {
						jnx_term_printf_in_color(JNX_COL_RED,"continued\n");
						job_send_status(obj,"FAILED",node_ip,node_port);
					}
				}while(!WIFEXITED(status) && !WIFSIGNALED(status));
				printf("Job exited with %d\n",WEXITSTATUS(status));
				printf("Releasing resources from current job\n");
				free(target_port);
				free(node_ip);
				free(node_port);
			}
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
				printf("Found item in queue\n");
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

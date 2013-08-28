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
			printf("selected job\n");
			jnx_term_printf_in_color(JNX_COL_YELLOW,"Setting job to in progress\n");
			query(obj->SENDER,target_port,API_COMMAND,"STATUS",obj->ID,"IN PROGRESS"," ",node_ip,node_port);
			jnx_term_printf_in_color(JNX_COL_YELLOW,"Running job via system command\n");
				pid_t process_pid = fork();
				if(process_pid == 0)
				{
					int ret = system(obj->DATA);
					printf("Spawning job in new process\n");
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
							printf("exited, status=%d\n", WEXITSTATUS(status));
						} else if (WIFSIGNALED(status)) {
							printf("killed by signal %d\n", WTERMSIG(status));
						} else if (WIFSTOPPED(status)) {
							printf("stopped by signal %d\n", WSTOPSIG(status));
						} else if (WIFCONTINUED(status)) {
							printf("continued\n");
						}

					}while(!WIFEXITED(status) && !WIFSIGNALED(status));
					printf("Completed child process\n");
				}
			
	/*  		jnx_term_printf_in_color(JNX_COL_YELLOW,"System command output returned %d\n", ret);
			if(ret != 0)
			{

				char retbuffer[25];
				sprintf(retbuffer,"%d",ret);
				jnx_term_printf_in_color(JNX_COL_YELLOW,"Setting job to failed\n");
				query(obj->SENDER,target_port,API_COMMAND,"STATUS",obj->ID,"FAILED",retbuffer,node_ip,node_port);
				jnx_term_reset_stdout();
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
				if(output_setup_complete == 0)
				{
					jnx_term_printf_in_color(JNX_COL_YELLOW,"Sending results\n");
					jnx_result_process(obj->SENDER, target_port,obj->ID,node_ip,node_port);
					jnx_result_teardown();
				}
				free(node_port);
				free(target_port);
				return;
			}
			if(output_setup_complete == 0)
			{
				jnx_term_printf_in_color(JNX_COL_YELLOW,"Sending results\n");
				jnx_result_process(obj->SENDER, target_port,obj->ID,node_ip,node_port);
				jnx_result_teardown();
			}
			jnx_term_printf_in_color(JNX_COL_YELLOW,"Setting job to completed\n");
			query(obj->SENDER,target_port,API_COMMAND,"STATUS",obj->ID,"COMPLETED"," ",node_ip,node_port);
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
			free(node_port);	
			free(target_port);
			//Send back console log
		*/
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

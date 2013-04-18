/*
 * =====================================================================================
 *
 *       Filename:  jnx_infrastructure.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  04/10/13 11:27:24
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *   Organization:  
 *
 * =====================================================================================
 */
#include <stdlib.h>
#include "jnx_infrastructure.h"
#include <jnxc_headers/jnxnetwork.h>
#include <pthread.h>
#include "../utils.h"
#include <string.h>
#include <stdlib.h>

extern jnx_hashmap *config;
void jnx_network_post_status_callback(char *message, char *client_ip)
{

}
void broadcast_callback(char *message)
{
	print_streams(JNX_COL_YELLOW,"Received: %s",message);
	// /machines/status/id/status
 	char sendline[MAXBUFFER];
	int size = 0;	
	char relative_path_buffer[256];
	char *rel_path = "/machines/status/%s";
	snprintf(relative_path_buffer,256,rel_path,"ONLINE");
	
	char *post = "POST %s HTTP/1.1\r\n" 
		"Host: %s:%s\r\n"
		"Content-Type: text/plain\r\n"
		"Content-Length: %d\r\n"
		"\r\n";

	snprintf(sendline,MAXBUFFER,post,relative_path_buffer,jnx_hash_get(config,"frontendserver"),jnx_hash_get(config,"frontendport"),size);
	printf("TOTAL RESULT %s\n",sendline);
	
	jnx_network_send_message_callback c = jnx_network_post_status_callback;
	if(jnx_network_send_message(jnx_hash_get(config,"frontendserver"),atoi(jnx_hash_get(config,"frontendport")),sendline, c) == 1)
	{
		print_streams(JNX_COL_RED,"Failed to send updated machine status\n");
		return;
	}

}
void *start_broadcast_listener(void *arg)
{
	print_streams(JNX_COL_YELLOW, "Starting broadcast listener\n");
	jnx_network_broadcast_callback c = broadcast_callback;
	jnx_network_broadcast_listener(BPORT,BGROUP,c);	
}
void jnx_infrastructure_broadcast_listen(void)
{
	pthread_t _listener_thread;
	pthread_create(&_listener_thread,NULL,start_broadcast_listener,NULL);	
}
void jnx_infrastructure_broadcast_send(char *message)
{
	print_streams(JNX_COL_YELLOW,"Multicasting: %s\n",message);
	jnx_network_send_broadcast(BPORT,BGROUP,message);
}

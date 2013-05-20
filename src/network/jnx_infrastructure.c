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
#include "../backend/sql_commands.h"
#include <string.h>
#include <stdlib.h>
#define BROADCAST_TIMEWAIT 30
extern jnx_hashmap *config;
void jnx_network_post_status_callback(char *message)
{

}
void broadcast_callback(char *message)
{
	print_streams(JNX_COL_YELLOW,"Received: %s",message);

	print_streams(JNX_COL_YELLOW,"Replying to master...\n");
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
	jnx_network_send_message_callback c = jnx_network_post_status_callback;
	if(jnx_network_send_message(jnx_hash_get(config,"frontendserver"),atoi(jnx_hash_get(config,"frontendport")),sendline, c) == 1)
	{
		print_streams(JNX_COL_RED,"Failed to send updated machine status\n");
		return;
	}
}
void jnx_infrastructure_set_node_challenge()
{
	printf("jnx_infrastructure_set_node_challenge, setting all nodes temporarily to offline...\n");
	if(sql_set_machines_offline())
	{
		print_streams(JNX_COL_RED,"Unable to set all nodes offline in DB for update challenge\n");
	}
}
void *jnx_infrastructure_update_worker(void*args)
{
	while(1)
	{
		/*-----------------------------------------------------------------------------
		 *  Before getting all node status, we set all nodes to offline momentarily
		 *-----------------------------------------------------------------------------*/
		
		sleep(BROADCAST_TIMEWAIT);
		jnx_infrastructure_set_node_challenge();
		jnx_infrastructure_broadcast_send("[Multicast]: All nodes tell me your status\n");
	}
}
void jnx_infrastructure_update_daemon(void)
{
	/*-----------------------------------------------------------------------------
	 *  Spawns a thread to perform peroidic broadcasts
	 *-----------------------------------------------------------------------------*/
	pthread_t _updatethread;
	pthread_create(&_updatethread,NULL,jnx_infrastructure_update_worker,NULL);
}
void *start_broadcast_listener(void *arg)
{
	print_streams(JNX_COL_YELLOW, "Starting broadcast listener\n");
	jnx_network_broadcast_callback c = broadcast_callback;
	jnx_network_broadcast_listener(BPORT,BGROUP,c);	
}
void jnx_infrastructure_broadcast_listen(void)
{
	sleep(5);
	pthread_t _listener_thread;
	pthread_create(&_listener_thread,NULL,start_broadcast_listener,NULL);	
}
void jnx_infrastructure_broadcast_send(char *message)
{
	print_streams(JNX_COL_YELLOW,"Multicasting: %s\n",message);
	jnx_network_send_broadcast(BPORT,BGROUP,message);
}

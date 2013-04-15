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
void broadcast_callback(char *message)
{
	print_streams(JNX_COL_YELLOW,"Received: %s",message);
	
	/*-----------------------------------------------------------------------------
	 *  Work here to be done on sending back IP to master; either through broadcast or TCP connection
	 *-----------------------------------------------------------------------------*/
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

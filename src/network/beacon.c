/*
 * =====================================================================================
 *
 *       Filename:  beacon.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  07/12/13 11:21:51
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
#include <pthread.h>
#include <jnxc_headers/jnxstring.h>
#include <jnxc_headers/jnxnetwork.h>
#include <jnxc_headers/jnxhash.h>
#include "transaction_api.h"
#include "../logic/job_control.h"
#include "beacon.h"
#define BPORT 12345
#define BGROUP "225.0.0.37"
#define WAIT_PERIOD 10
#ifdef __APPLE__
#define INTERFACE "en0"
#else
#define INTERFACE "eth0"
#endif
void beacon_send(void)
{
}
void *beacon_loop(void*ar)
{
	while(1)
	{
		sleep(WAIT_PERIOD);
		beacon_send();
	}
}
void beacon_pulse(void)
{
	pthread_t beacon_thread;
	pthread_create(&beacon_thread,NULL,beacon_loop,NULL);
}
void beacon_message_intercept(char *msg)
{
	printf("Responding to challenge...%s\n",msg);
	
	api_command_obj *obj = transaction_api_create_obj(msg);


	char *node_ip = jnx_network_local_ip(INTERFACE);
	char *node_port = jnx_string_itos(LISTENPORT);
	char *port = jnx_string_itos(obj->PORT);
	query(obj->SENDER,port,API_COMMAND,"ALIVE","","ONLINE"," ",node_ip,node_port);

	free(port);
	transaction_api_delete_obj(obj);
}
void *beacon_receive(void*ar)
{
	jnx_network_broadcast_listener_callback c = beacon_message_intercept;
	jnx_network_broadcast_listener(BPORT,BGROUP,c);	
}
void beacon_listen(void)
{
	pthread_t beacon_listen_thread;
	pthread_create(&beacon_listen_thread,NULL,beacon_receive,NULL);
}

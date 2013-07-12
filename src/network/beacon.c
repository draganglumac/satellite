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
#include "beacon.h"
#include "../logic/job_control.h"
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
	char *message = "CHALLENGE";
	jnx_network_send_broadcast(BPORT,BGROUP,message);
	printf("beacon_send: Sending broadcast\n");
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
	if(strcmp(msg,"CHALLENGE") == 0)
	{
		printf("Responding to challenge...\n");
		
	}
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

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
#include "job_control.h"
#include "../network/transaction_api.h"
#include <jnxc_headers/jnxnetwork.h>
#include <jnxc_headers/jnxhash.h>
#define LISTENPORT 9090
void message_intercept(char *message, size_t msg_len, char *ip)
{
	printf("RAW %s\n",message);
	api_command_obj *obj = transaction_api_create_obj(message);
	if(obj == NULL)
	{
		printf("Failed to create api_command_obj\n");
		return;
	}
	printf("OBJECT CMD:%d ID:%s DATA:%s OTHER:%s SENDER:%s PORT:%d\n",obj->CMD,obj->ID,obj->DATA,obj->OTHER,obj->SENDER,obj->PORT);

}
void job_control_start_listening(void)
{
	jnx_network_listener_callback ll = message_intercept;
	jnx_network_setup_listener(LISTENPORT,ll);
}

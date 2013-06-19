/*
 * =====================================================================================
 *
 *       Filename:  jnx_transmitter.c
 *
 *    Description:  sending to nodes
 *
 *        Version:  1.0
 *        Created:  03/26/13 15:37:24
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alex Jones (), alexsimonjones@gmail.com
 *   Organization:  
 *
 * =====================================================================================
 */
#include <stdlib.h>
#include <unistd.h>
#include <jnxc_headers/jnxhash.h>
#include "../utils.h"
#include "../network/jnxsql_interface.h"
#include "../network/sql_command_interface.h"
#define TIMEWAIT 15
extern jnx_hashmap *config;
void transmit_orders_callback(char *message)
{
	printf("Transmit orders callback: %s",message);
}
int transmit_job_orders(char *job_id,char *job_name, char *machine_ip, char *command)
{
	/*  lets print our expected results for visual confirmation */
	int command_len = strlen(command); 
	char *transmission_string = (char*)malloc(command_len + 1);
	strcpy(transmission_string,command);
	jnx_string_join(&transmission_string,"!");
	jnx_string_join(&transmission_string,job_id);
	jnx_network_send_message_callback smc = transmit_orders_callback;
	if(jnx_network_send_message(machine_ip,9099,transmission_string, smc) != 0)
	{

		print_streams(JNX_COL_RED,"Failed to send message to target machine, aborting\n");
		free(transmission_string);
		//LOG ERROR
		return 1; 
	}
	free(transmission_string);
	//Write job in progress to sql
	if(sql_set_job_progress(job_id,"IN PROGRESS") != 0)
	{
		printf("Unable to set job to IN PROGRESS, aborting\n");
		return 1;
	}    
	return 0;
}
void parse_job(MYSQL_ROW row)
{
	/*-----------------------------------------------------------------------------
	 *  Checks to see whether the trigger time is within 60 seconds of current
	 *-----------------------------------------------------------------------------*/
	int is_recursive = atoi(row[7]);
	int trigger = utils_check_trigger_time(row[6],row[1]);	
	char *job_status = row[4];
	
	print_streams(JNX_COL_GREEN,"%s\n", job_status);

	switch(trigger)
	{
		case READYTORUN:
			if(is_recursive == NO && strcmp(job_status,"COMPLETED") == 0)
			{
				printf("Job has already been completed and is on a single run cycle\n");
				return;
			}
			if(strcmp(job_status,"IN PROGRESS") == 0)
			{
				printf("Job has already been run and is in progress\n");
				return;
			}
			print_streams(JNX_COL_GREEN,"Starting to run job\n");	
			
			int orders_ret = transmit_job_orders(row[0],row[1],sql_resolve_machine_ip(row[5]),row[3]);
			if(orders_ret != 0)
			{ 
				print_streams(JNX_COL_RED,"Warning catastrophic failure in transmit_job_orders\n"); 
				sql_set_job_progress(row[0],"FAILED");
			}	
			switch(is_recursive)
			{
				case YES:
					/*-----------------------------------------------------------------------------
					 *  Job is recursive
					 *-----------------------------------------------------------------------------*/
					sql_update_job_trigger(row[0]);					
					if(orders_ret == 0)
						sql_set_job_progress(row[0],"IN PROGRESS");
					break;
				case NO:
					/*-----------------------------------------------------------------------------
					 *  Job is not recursive
					 *-----------------------------------------------------------------------------*/

					if(orders_ret == 0)
						sql_set_job_progress(row[0],"IN PROGRESS");
					break;
			}
			printf("Run complete\n");
			break;
		
		/*  Jobs not ready to run */
		case ALREADYRUN:
			printf("Already run\n");
			break;
		case NOTREADYTORUN:
			printf("Not ready to run\n");
			break;
	}

}
int jnx_transmitter_perform_jobs()
{       
	MYSQL_RES *result = sql_get_candidate_jobs();
	if(result == NULL)
	{
		print_streams(JNX_COL_RED,"Error connecting to sql database, trying again...\n");
		return -1;
	}
	int i = 0, num_fields = mysql_num_fields(result);
	MYSQL_ROW row;
	while((row = mysql_fetch_row(result)))
	{
		for(i = 0; i < num_fields; ++i)
		{
			if(row[i] == NULL)
			{
				print_streams(DEFAULTCOLOR,"Error in sql syntax for row %d\n",i);
				return 1;
			}
		}
		parse_job(row);
	}
	mysql_free_result(result); 
	return 0;
}
int timeout_for_retry(int retry)
{
	int max_retries = atoi(jnx_hash_get(config, "max_retries"));
	if ( retry == max_retries )
		return -1;

	int  first = 1, second = 2, sum, i;
	for ( i = 0; i < retry; i++ )
	{
		if ( i == 0 )
		{
			sum = 1;
		}
		else if ( i == 1 )
		{
			sum = 2;
		}
		else
		{
			sum = first + second;
			first = second;
			second = sum;
		}
	}

	return 10 * sum;
}
void jnx_start_transmitter(void)
{
	print_streams(DEFAULTCOLOR,"Starting daemon\n");
	int retries_on_error = 0;
	while(1)
	{
		int ret = jnx_transmitter_perform_jobs();
		if(ret == 1)
		{
			print_streams(JNX_COL_RED,"Error in perform jobs\n");
			exit(1);
		}
		else if(ret == -1)
		{
			print_streams(JNX_COL_RED,"jnx_start_transmitter encountered an error communicating with connecting to the sql database\nAttempting recovery...\n");        
			retries_on_error++;
			int next_timeout = timeout_for_retry(retries_on_error);
			if ( next_timeout == -1 )
			{
				print_streams(JNX_COL_RED,"jnx_start_transmitter catastrophic failure!\nAborting satellite after the maximum number of unsucessful recovery attempts.\n");
				exit(1);
			}
			sleep(next_timeout);
			continue;
		}
		retries_on_error = 0;
		sleep(TIMEWAIT);
	}
}

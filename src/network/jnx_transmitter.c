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
#include "../backend/jnxsql_interface.h"
#include "../backend/sql_commands.h"
#define TIMEWAIT 15
extern jnx_hashmap *config;
void parse_job(MYSQL_ROW row)
{
	/*-----------------------------------------------------------------------------
	 *  Checks to see whether the trigger time is within 60 seconds of current
	 *-----------------------------------------------------------------------------*/
	int is_recursive = atoi(row[7]);
	int trigger = utils_check_trigger_time(row[6],row[1]);	

	char *job_status = row[4];

	if(strcmp(job_status,"COMPLETED") == 0)
		print_streams(JNX_COL_CYAN,"Job has already been run in this trigger period\n");
		return;

	switch(trigger)
	{
		case 0:
			print_streams(JNX_COL_GREEN,"Starting to run job\n");	
			/*-----------------------------------------------------------------------------
			 *  Ready to run job
			 *-----------------------------------------------------------------------------*/
			int orders_ret = sql_transmit_job_orders(row[0],row[1],sql_resolve_machine_ip(row[5]),row[3]);
			if(orders_ret != 0)
			{ 
				print_streams(JNX_COL_RED,"Warning catastrophic failure in transmit_job_orders\n"); 
				sql_set_job_progress(row[0],"FAILED");
			}	
			switch(is_recursive)
			{
				case 1:
					/*-----------------------------------------------------------------------------
					 *  Job is recursive
					 *-----------------------------------------------------------------------------*/
					sql_update_job_trigger(row[0]);					
					if(!orders_ret)
						sql_set_job_progress(row[0],"SCHEDULED");
					break;
				case 0:
					/*-----------------------------------------------------------------------------
					 *  Job is not recursive
					 *-----------------------------------------------------------------------------*/

					if(!orders_ret)
						sql_set_job_progress(row[0],"COMPLETED");
					break;
			}
			break;
		case 1:
			print_streams(JNX_COL_MAGENTA,"Not yet ready to run job\n");	
			/*-----------------------------------------------------------------------------
			 *  Job not ready to run
			 *-----------------------------------------------------------------------------*/
			break;
		case -1:
			print_streams(JNX_COL_MAGENTA,"Job already run\n");	
			/*-----------------------------------------------------------------------------
			 *  Job already run
			 *-----------------------------------------------------------------------------*/
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

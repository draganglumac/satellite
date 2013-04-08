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
#include "../utils.h"
#include "../backend/jnxsql_interface.h"
#include "../backend/sql_commands.h"
#define TIMEWAIT 5
void parse_job(MYSQL_ROW row)
{
	if(utils_check_trigger_time(row[6],row[1]) == 0)
	{      
		print_streams(JNX_COL_GREEN,"Trigger pulled! Running job\n");
		if(sql_transmit_job_orders(row[0],row[1],sql_resolve_machine_ip(row[5]),row[3]) != 0)
		{
			jnx_term_printf_in_color(JNX_COL_RED,"Warning catastrophic failure in transmit_job_orders\n");
			return;
		}

		/*-----------------------------------------------------------------------------
		 *  0 indicates no recursive pattern set, 1 indicates it has been.
		 *-----------------------------------------------------------------------------*/
		if(atoi(row[7]) != 0)
		{
			printf("RECURSIVE JOB\n");
			print_streams(DEFAULTCOLOR,"Updating unixtimestamp of job to +24hrs\n");
			if(sql_update_job_trigger(row[0]) != 0)
			{
				print_streams(JNX_COL_RED,"Error with update_job_trigger\n");
				return;
			}
			print_streams(DEFAULTCOLOR,"Setting job progress back to INCOMPLETE\n");
			sql_set_job_progress(row[0],"INCOMPLETE");
		}else
		{
			printf("NON RECURSIVE JOB\n");
			print_streams(DEFAULTCOLOR,"Setting non recursive to complete\n");
			sql_set_job_progress(row[0],"COMPLETED"); 
		}
	}
}
int jnx_transmitter_perform_jobs()
{       
	MYSQL_RES *result = sql_get_incomplete_jobs();
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
void jnx_start_transmitter(void)
{
	print_streams(DEFAULTCOLOR,"Starting daemon\n");
	while(1)
	{
		int ret = jnx_transmitter_perform_jobs();

		if(ret == 1)
		{
			print_streams(JNX_COL_RED,"Error in perform jobs\n");
			exit(0);
		}else if(ret == -1)
		{
			print_streams(JNX_COL_RED,"jnx_start_transmitter encountered an error communicating with connecting to the sql database\n");		
			sleep(10);	
		}
		sleep(TIMEWAIT);
	}
}

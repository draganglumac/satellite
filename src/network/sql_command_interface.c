/*
 * =====================================================================================
 *
 *       Filename:  interface_commands.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  03/07/13 10:53:05
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alex Jones (), alexsimonjones@gmail.com
 *   Organization:  
 *
 * =====================================================================================
 */
#include "sql_command_interface.h"
#include <stdlib.h>
//callback hooks for jnx_sql_query
void jnx_network_send_message_local_callback(char *message)
{
}
void generic_sql_callback(MYSQL_RES *res)
{
}
int sql_set_machines_offline(void)
{
	char *output = "use AUTOMATION; update machines set status='OFFLINE' where status!='OFFLINE';";
	MYSQL_RES *result;
	if(jnx_sql_resultfill_query(output,&result) != 0)
	{
		return 1;
	}
	return 0;
}
char* sql_resolve_machine_ip(char *machine_number)
{
	/* 
	 * Warning this uses stored procedures 
	 *
	 */
	char output[1024];
	char *current_machine_ip = NULL;
	strcpy(output,"USE AUTOMATION; call get_machine_ip_from_id(");
	MYSQL_ROW row;
	int i;
	int num_fields; 
	strcat(output,machine_number);
	strcat(output,");");
	printf("Raw machine number -> %s\n",machine_number);
	printf("%s\n",output);
	//perform the actual request
	MYSQL_RES *result;
	if(jnx_sql_resultfill_query(output,&result) != 0)
	{
		print_streams(JNX_COL_RED,"An error occured whilst sending query\n");
		return "ERROR WITH MACHINE IP";
	}
	num_fields = mysql_num_fields(result);
	while ((row = mysql_fetch_row(result)))
	{
		//row is the entire data line we want
		for(i = 0; i < num_fields; i++)
		{
			if(row[0] != NULL)
			{
				current_machine_ip = row[0];
				//break so we only return the first entry
				break;
			}
			else{
				current_machine_ip = "ERROR";
			}
		}
	}
	mysql_free_result(result);
	return current_machine_ip;
}
int sql_update_job_trigger(char *job_id)
{
	print_streams(JNX_COL_CYAN,"Incrementing job trigger by 24 hours\n");
	char output[256];
	strcpy(output,"use AUTOMATION; call add_day_to_trigger_from_id("); 
	strcat(output,job_id);
	strcat(output,");");
	sql_callback c = &generic_sql_callback;
	if(jnx_sql_query(output,c) != 0)
	{
		return 1;
	}else
		return 0;
}
int sql_set_job_progress(char *job_id,char*status)
{
	char output[256];
	strcpy(output,"use AUTOMATION; call set_job_status_from_id("); 
	strcat(output,job_id);
	strcat(output,",'");
	strcat(output,status);
	strcat(output,"'");
	strcat(output,");");
	printf("%s\n",output);
	sql_callback c = &generic_sql_callback;
	if(jnx_sql_query(output,c) != 0)
	{
		return 1;
	}else{
		//avoids a crash if sql fails
	}
	return 0;
}
MYSQL_RES *sql_get_candidate_jobs(void)
{
	MYSQL_RES *result;
	if(jnx_sql_resultfill_query("USE AUTOMATION; call get_candidate_jobs();",&result) != 0)
	{
		print_streams(DEFAULTCOLOR,"An error occured whilst sending query\n");
		return NULL;
	}
	//close our db connection to stop,
	return result;
}
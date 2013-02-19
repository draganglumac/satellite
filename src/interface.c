#include "interface.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <jnxc_headers/jnxstring.h>
#include <jnxc_headers/jnxterm.h>
#include <jnxc_headers/jnxnetwork.h>
#include <setjmp.h>

//Forward declarations
int setup_sql(char*sqlhost,char*sqluser,char*sqlpass);
//global vars
char *sqlhost = NULL,*sqluser = NULL,*sqlpass = NULL;
//callback hooks for jnx_sql_query
void generic_sql_callback(MYSQL_RES *res)
{
	//our generic callback
}
char* resolve_machine_ip(char *machine_number)
{
	if(setup_sql(sqlhost,sqluser,sqlpass) != 0)
	{
		printf("Error connecting to sql to resolve machine ip, aborting\n");
		exit(1);
	}
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
		printf("An error occured whilst sending query\n");
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
			}
			else{
				current_machine_ip = "ERROR";
			}
		}
	}
	mysql_free_result(result);
	jnx_sql_close();
	return current_machine_ip;
}
int set_job_to_in_progress(char *job_id)
{
	if(setup_sql(sqlhost,sqluser,sqlpass) != 0)
	{
		printf("Error connecting to sql\n");
		return 1;
	}
	char output[256];
	strcpy(output,"use AUTOMATION; call set_job_status_from_id("); 
	strcat(output,job_id);
	strcat(output,",'INPROGRESS'");
	strcat(output,");");
	printf("%s\n",output);
	sql_callback c = &generic_sql_callback;
	if(jnx_sql_query(output,c) != 0)
	{
		return 1;
	}
	jnx_sql_close();
	return 0;
}
//here we transmit the actual job
void transmit_job_orders(char *job_id,char *job_name, char *machine_ip, char *command)
{
	/*  lets print our expected results for visual confirmation */
	printf("Transmitting job_id -> %s\n",job_id);
	printf("Transmitting job_name -> %s\n",job_name);
	printf("Transmitting machine_ip -> %s\n",machine_ip);
	printf("Transmitting command -> %s\n",command);
	//Form job string 

	//Transmit job string

	//Append to command the job id ->
	int command_len = strlen(command); 
	char *transmission_string = (char*)malloc(command_len + 1);
	strcpy(transmission_string,command);
	printf("Copy of command is %s\n",transmission_string);
	jnx_string_join(&transmission_string,"!");
	jnx_string_join(&transmission_string,job_id);
	printf("Outgoing transmission %s\n",transmission_string);

	if(jnx_send_message(machine_ip,9099,transmission_string) != 0)
	{
		printf("Failed to send message to target machine, aborting\n");
		exit(1);
	}
	free(transmission_string);
	//Write job in progress to sql
	if(set_job_to_in_progress(job_id) != 0)
	{
		printf("Unable to set job to INPROGRESS, aborting\n");
		exit(1);
	}    
} 
int setup_sql(char* host_addr, char* username, char* port)
{
	return jnx_sql_interface_setup(host_addr,username,port);
}
int response_from_db(char *sqlh, char* sqlu, char *sqlp)
{
	int i;
	int num_fields;
	MYSQL_ROW row;
	printf("Started response_from_db\n");
	//set our sql data
	sqlhost = sqlh;
	sqluser = sqlu;
	sqlpass = sqlp;

	if(setup_sql(sqlhost,sqluser,sqlpass) != 0)
	{
		printf("Error connecting to sql\n");
		return 1;
	}
	/*
	 * Warning this uses stored procedures 
	 *
	 */
	MYSQL_RES *result;
	if(jnx_sql_resultfill_query("USE AUTOMATION; call get_incomplete_jobs();",&result) != 0)
	{
		printf("An error occured whilst sending query\n");
		return 1;
	}
	//close our db connection to stop it from sleeping
	jnx_sql_close();
	num_fields = mysql_num_fields(result);
	while ((row = mysql_fetch_row(result)))
	{
		//row is the entire data line we want
		for(i = 0; i < num_fields; i++)
		{
			if(row[i] == NULL)
			{
				printf("Error in sql syntax for row %d\n",i);
				return 1;
			}

		}
		transmit_job_orders(row[0],row[1]/*  row[2] is time stamp we don't use currently */,resolve_machine_ip(row[5]) /*  we do another call to find machine ip */,row[3]);
	}
	mysql_free_result(result);
	return 0;
}
int write_result_to_db(char *job_id,char *result_input)
{
	printf("Database to write to: %s %s %s\n",sqlhost,sqluser,sqlpass);
	if(setup_sql(sqlhost,sqluser,sqlpass) != 0)
	{
		printf("Error connecting to sql\n");
		return 1;

	}
	char query[1024];
	strcpy(query,"USE AUTOMATION; call add_result_from_job('");
	strcat(query,job_id);
	strcat(query,"','");
	strcat(query,result_input);
	strcat(query,"');");
	
	printf("Write result output : %s\n",query);

	MYSQL_RES *result;
	if(jnx_sql_resultfill_query(query,&result) != 0)
	{
		printf("An error occured whilst sending query\n");
		return 1;
	}
	//we don't need to free result as it will be null on an insert of this type
	jnx_sql_close();
	return 0;
}
void set_sql_data(char * sqlh,char* sqlu,char* sqlp)
{
	sqlhost = sqlh;
	sqluser = sqlu;
	sqlpass = sqlp;
}

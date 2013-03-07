#include <time.h>
#include "interface.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <jnxc_headers/jnxstring.h>
#include <jnxc_headers/jnxterm.h>
#include <jnxc_headers/jnxnetwork.h>
#include <jnxc_headers/jnxlog.h>
#include <setjmp.h>
//Forward declarations
int jnx_sql_interface_setup(char*sqlhost,char*sqluser,char*sqlpass);
//global vars
char *sqlhost = NULL,*sqluser = NULL,*sqlpass = NULL;
//callback hooks for jnx_sql_query
void generic_sql_callback(MYSQL_RES *res)
{
}
char* resolve_machine_ip(char *machine_number)
{
    if(jnx_sql_interface_setup(sqlhost,sqluser,sqlpass) != 0)
    {
        jnx_term_printf_in_color(JNX_COL_RED,"Error connecting to sql to resolve machine ip, aborting\n");
        jnx_log("Error connecting to sql to resolve machine ip, aborting");
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
        jnx_term_printf_in_color(JNX_COL_RED,"An error occured whilst sending query\n");
        jnx_log("An error occured whilst sending query rom resolve_machine_ip");
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
    jnx_sql_close();
    return current_machine_ip;
}
int update_job_trigger(char *job_id)
{
    if(jnx_sql_interface_setup(sqlhost,sqluser,sqlpass) != 0)
    {
        jnx_term_printf_in_color(JNX_COL_RED,"Error connecting to sql\n");
        jnx_log("Error connecting to sql in set_job_progress");
        return 1;
    }
    char output[256];
    strcpy(output,"use AUTOMATION; call add_day_to_trigger_from_id("); 
    strcat(output,job_id);
    strcat(output,");");
    printf("%s\n",output);
    
    sql_callback c = &generic_sql_callback;
    if(jnx_sql_query(output,c) != 0)
    {
        return 1;
    }else
        return 0;
}
int set_job_progress(char *job_id,char*status)
{
    if(jnx_sql_interface_setup(sqlhost,sqluser,sqlpass) != 0)
    {
        jnx_term_printf_in_color(JNX_COL_RED,"Error connecting to sql\n");
        jnx_log("Error connecting to sql in set_job_progress");
        return 1;
    }
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
        jnx_sql_close();
    }
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
    jnx_log("**Transmitting job***");
    jnx_log(job_id);
    jnx_log(job_name);
    jnx_log(machine_ip);
    jnx_log(command);
    //Transmit job string
    jnx_log("**Transmitting job***");

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
        jnx_term_printf_in_color(JNX_COL_RED,"Failed to send message to target machine, aborting\n");
        jnx_log("Failed to send message to target machine in transmit_job_orders");
        free(transmission_string);

        if(set_job_progress(job_id,"FAILED") != 0)
        {
            printf("Unable to set job to FAILED, aborting\n");
            exit(1);
        }    
        //LOG ERROR
        return; 
    }
    free(transmission_string);
    //Write job in progress to sql
    if(set_job_progress(job_id,"INPROGRESS") != 0)
    {
        printf("Unable to set job to INPROGRESS, aborting\n");
        exit(1);
    }    
}
int check_trigger_time(char *time_)
{
    time_t current_time = time(0);
    time_t triggertime = atoi(time_);
    printf("Trigger time: %d %s\n",(signed int)triggertime,ctime(&triggertime));
    printf("Current time: %d %s\n",(signed int)current_time,ctime(&current_time));
    //CHECK THE DIFFERENCE BETWEEN TIMES
    printf("The time difference is %ld\n",(triggertime - current_time));
    jnx_log("**check_trigger_time**");
    if((triggertime - current_time) <= 60)
    {
        jnx_term_printf_in_color(JNX_COL_BLUE,"Trigger time within 60 seconds of current\n");
        jnx_log("**pulling trigger**");        
        return 0;
    }
    return 1;
}
int update_on_recursive_job(char *recursionflag)
{
    if(atoi(recursionflag) == 0)
    {
        printf("No recursion flag set\n");
        return 1;
    }
    printf("Recursion flag found\n");
    return 0;
}
int response_from_db()
{
    int i;
    int num_fields;
    MYSQL_ROW row;
    printf("Started response_from_db\n");
    jnx_log("Started response_from_db");
    //set our sql data
    if(jnx_sql_interface_setup(sqlhost,sqluser,sqlpass) != 0)
    {
        jnx_term_printf_in_color(JNX_COL_RED,"Error connecting to sql\n");
        jnx_log("Error connecting to sql in response_from_db");
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
        jnx_log("Error sending query from response_from_db");
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
        //CHECK TRANSMISSION DUE TIME
        if(check_trigger_time(row[6]) == 0)
        {      
            jnx_term_printf_in_color(JNX_COL_GREEN,"Trigger pulled! Running job\n");

            //void transmit_job_orders(char *job_id,char *job_name, char *machine_ip, char *command)
            transmit_job_orders(row[0],row[1],resolve_machine_ip(row[5]),row[3]);
            //CHECK JOB RECURSION
            if(update_on_recursive_job(row[7]) == 0)
            {
                printf("Updating unixtimestamp of job to +24hrs\n");
                //we need to firstly, update the job timer, 
                //secondly update the progress
                printf("Setting job progress back to INCOMPLETE\n");
                if(update_job_trigger(row[0]) != 0)
                {
                    jnx_term_printf_in_color(JNX_COL_RED,"Error with update_job_trigger\n");
                    //if there is an error setting we won't continue the job recursion, otherwise it will create an infinite loop
                    continue;
                }
                set_job_progress(row[0],"INCOMPLETE");
            }else
            {
                printf("Setting non recursive to complete\n");
                set_job_progress(row[0],"COMPLETED"); 
            }
            //check if the job was on a recursive timer!
        }
    }
    mysql_free_result(result);
    return 0;
}
int write_result_to_db(char *job_id,char *result_input)
{
    if(jnx_sql_interface_setup(sqlhost,sqluser,sqlpass) != 0)
    {
        jnx_term_printf_in_color(JNX_COL_RED,"Error connecting to sql\n");
        jnx_log("Error connecting to sql in write_result_to_db");
        return 1;

    }
    char query[1024];
    strcpy(query,"USE AUTOMATION; call add_result_from_job('");
    strcat(query,job_id);
    strcat(query,"','");
    strcat(query,result_input);
    strcat(query,"');");
    printf("Write result output : %s\n",query);
    sql_callback c = &generic_sql_callback;
    if(jnx_sql_query(query,c) != 0)
    {
        jnx_term_printf_in_color(JNX_COL_RED,"Error with query in write_result_to_db\n");
        return 1;
    }
    jnx_sql_close();
    return 0;
}
int store_sql_credentials(char* host_addr, char* username, char* pass)
{
    sqlhost = host_addr;
    sqluser = username;
    sqlpass = pass;
    int ret = 0;
    MYSQL_RES *result;
    if(jnx_sql_interface_setup(sqlhost,sqluser,sqlpass) != 0)
    {
        jnx_term_printf_in_color(JNX_COL_RED,"Error connecting to sql\n");
        jnx_log("Error connecting to sql in store_sql_credentials");
        return 1;
    }
    ret = jnx_sql_resultfill_query("use AUTOMATION; select 'test';",&result);
    if(ret == 0 ) { mysql_free_result(result); jnx_sql_close();};

    return ret;
}

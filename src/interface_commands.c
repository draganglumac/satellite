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
#include "interface_commands.h"
#include <stdlib.h>
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
int transmit_job_orders(char *job_id,char *job_name, char *machine_ip, char *command)
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
        return 1; 
    }
    free(transmission_string);
    //Write job in progress to sql
    if(set_job_progress(job_id,"INPROGRESS") != 0)
    {
        printf("Unable to set job to INPROGRESS, aborting\n");
        exit(1);
    }    
    return 0;
}
MYSQL_RES *get_incomplete_jobs(void)
{
    if(jnx_sql_interface_setup(sqlhost,sqluser,sqlpass) != 0)
    {
        jnx_term_printf_in_color(JNX_COL_RED,"Error connecting to sql\n");
        jnx_log("Error connecting to sql in response_from_db");
        exit(1);
    }
    MYSQL_RES *result;
    if(jnx_sql_resultfill_query("USE AUTOMATION; call get_incomplete_jobs();",&result) != 0)
    {
        printf("An error occured whilst sending query\n");
        jnx_log("Error sending query from response_from_db");
        exit(1);
    }
    //close our db connection to stop it from sleeping
    jnx_sql_close();
    return result;
}
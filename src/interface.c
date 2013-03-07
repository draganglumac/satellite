#include <time.h>
#include "interface.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <jnxc_headers/jnxstring.h>
#include <jnxc_headers/jnxterm.h>
#include <jnxc_headers/jnxnetwork.h>
//global vars
char *sqlhost = NULL,*sqluser = NULL,*sqlpass = NULL;
//here we transmit the actual job
int check_trigger_time(char *time_, char *job_id)
{
    time_t current_time = time(0);
    time_t triggertime = atoi(time_);
    
    char *s = ctime(&triggertime);
    s[strlen(s)-1]=0; 
   
    printf("Job %s is set to run at %s which is %d from current\n",job_id,s,(int)(triggertime - current_time)); 
    
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
int perform_job_cycle()
{
    printf("Started response_from_db\n");
    jnx_log("Started response_from_db");
    MYSQL_RES *result = get_incomplete_jobs();
    int i;
    int num_fields;
    MYSQL_ROW row;
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
        /*
         * Row magic numbers list
         * 0 - Jobid
         * 1 - Job name
         * 2 - TIMESTAMP
         * 3 - command
         * 4 - status
         * 5 - machine_id
         * 6 - trigger_time
         * 7 - recursion
         */
        if(check_trigger_time(row[6],row[1]) == 0)
        {      
            jnx_term_printf_in_color(JNX_COL_GREEN,"Trigger pulled! Running job\n");
            if(transmit_job_orders(row[0],row[1],resolve_machine_ip(row[5]),row[3]) != 0)
            {
                jnx_log("Major failure in transmit_job_orders");
                jnx_term_printf_in_color(JNX_COL_RED,"Warning catastrophic failure in transmit_job_orders\n");
                continue;
            }
            //CHECK JOB RECURSION
            if(update_on_recursive_job(row[7]) == 0)
            {
                printf("Updating unixtimestamp of job to +24hrs\n");
                if(update_job_trigger(row[0]) != 0)
                {
                    jnx_term_printf_in_color(JNX_COL_RED,"Error with update_job_trigger\n");
                    continue;
                }
                printf("Setting job progress back to INCOMPLETE\n");
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
int perform_store_sql_credentials(char* host_addr, char* username, char* pass)
{
    sqlhost = host_addr;
    sqluser = username;
    sqlpass = pass;
    int ret = 0;
    MYSQL_RES *result;
    if(jnx_sql_interface_setup(sqlhost,sqluser,sqlpass) != 0)
    {
        jnx_term_printf_in_color(JNX_COL_RED,"Error connecting to sql\n");
        jnx_log("Error connecting to sql in perform_store_sql_credentials");
        return 1;
    }
    ret = jnx_sql_resultfill_query("use AUTOMATION; select 'test';",&result);
    if(ret == 0 ) { mysql_free_result(result); jnx_sql_close();};

    return ret;
}

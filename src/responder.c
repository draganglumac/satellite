#include "responder.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <jnxc_headers/jnxterm.h>

// Here we will do the actual sending
char *resolve_machine_ip(char *machine_number)
{
    return "NOT IMPLEMENTED";
}
void transmit_job_orders(char *job_id,char *job_name, char *machine_ip, char *command)
{
    /*  lets print our expected results for visual confirmation */
    printf("Transmitting job_id -> %s\n",job_id);
    printf("Transmitting job_name -> %s\n",job_name);
    printf("Transmitting machine_ip -> %s\n",machine_ip);
    printf("Transmitting command -> %s\n",command);

    //Form job string 
    //
    //Transmit job string
    //
    //Write job in progress to sql
} 
void get_incomplete_jobs_callback(MYSQL_RES* result)
{
    int num_fields = mysql_num_fields(result);
    MYSQL_ROW row;
    int i;
    while ((row = mysql_fetch_row(result)))
    {
        //row is the entire data line we want
        for(i = 0; i < num_fields; i++)
        {
            if(row[i] == NULL)
            {
                printf("Found empty row; job is malformed, aborting!\n");
                continue;
            }
        }
        printf("\n");
        //send it to our node!
        transmit_job_orders(row[0],row[1]/*  row[2] is time stamp we don't use currently */,resolve_machine_ip(row[3]) /*  we do another call to find machine ip */,row[4]);
    }
}
int setup_sql(char* host_addr, char* username, char* port)
{
    return jnx_sql_interface_setup(host_addr,username,port);
}
int response_from_db(char *sqlhost, char* sqluser, char *sqlport)
{
    if(setup_sql(sqlhost,sqluser,sqlport) != 0)
    {
        printf("Error connecting to sql\n");
        return 1;
    }
    sql_callback c = get_incomplete_jobs_callback;
    /*
     * Warning this uses stored procedures 
     *
     */
    if(jnx_sql_query("USE AUTOMATION; call get_incomplete_jobs();",c) != 0)
    {
        printf("An error occured whilst sending query\n");
        return 1;
    }
    jnx_sql_close();
    return 0;
}

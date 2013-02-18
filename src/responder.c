#include "responder.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <jnxc_headers/jnxterm.h>
#include <setjmp.h>
#include "jump_settings.h"
//return the ip address
char *sqlhost = NULL,*sqluser = NULL,*sqlport = NULL;
char *current_machine_ip = NULL;

void machine_ip_callback(MYSQL_RES *result)
{

    int num_fields = mysql_num_fields(result);
    MYSQL_ROW row;
    int i;
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
}
char* resolve_machine_ip(char *machine_number)
{
    if(setup_sql(sqlhost,sqluser,sqlport) != 0)
    {
        printf("Error connecting to sql to resolve machine ip, aborting\n");
        exit(1);
    }
    /* 
     * Warning this uses stored procedures 
     *
     */
    char output[1024];
    strcpy(output,"USE AUTOMATION; call get_machine_ip_from_id(");
    strcat(output,machine_number);
    strcat(output,");");
    printf("Raw machine number -> %s\n",machine_number);
    printf("%s\n",output);
    sql_callback c = &machine_ip_callback;
    //perform the actual request
    if(jnx_sql_query(output,c) != 0)
    {
        printf("An error occured whilst sending query\n");
        return "ERROR WITH MACHINE IP";
    }
    jnx_sql_close();
    return current_machine_ip;
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
    //
    //Transmit job string
    //
    //Write job in progress to sql
    printf("Done\n");
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
        //send it to our node!
        //WARNING HARDCODED MAGIC NUMBERS
        transmit_job_orders(row[0],row[1]/*  row[2] is time stamp we don't use currently */,resolve_machine_ip(row[5]) /*  we do another call to find machine ip */,row[3]);
    }
    printf("got to the end of get_incomplete_jobs_callback\n");

    longjmp(jumper,0);
}
int setup_sql(char* host_addr, char* username, char* port)
{
    return jnx_sql_interface_setup(host_addr,username,port);
}
int response_from_db(char *sqlh, char* sqlu, char *sqlp)
{
    printf("Started response_from_db\n");
    //set our sql data
    sqlhost = sqlh;
    sqluser = sqlu;
    sqlport = sqlp;

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

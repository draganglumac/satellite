#include "responder.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <jnxc_headers/jnxterm.h>
void get_incomplete_jobs_callback(MYSQL_RES* result)
{
    int num_fields = mysql_num_fields(result);
    MYSQL_ROW row;
    int i;
    while ((row = mysql_fetch_row(result)))
    {
        for(i = 0; i < num_fields; i++)
        {
            printf("%s ", row[i] ? row[i] : "NULL");
        }
        printf("\n");
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

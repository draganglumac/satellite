#include "jnxsql_interface.h"
#include <stdio.h>
#include <stdlib.h>
MYSQL *connection;
MYSQL_RES *result;
MYSQL_ROW row;
char *host;
char *username;
char *password;
char *database;
void jnx_sql_close(void )
{
    if(connection == NULL) return;
    mysql_close(connection);
    connection = NULL;
}
int jnx_sql_interface_setup(char* _host, char* _username, char* _password)
{
    printf("Connecting...\n");
    connection = mysql_init(connection);
    if(connection == NULL) return 1;

    username = _username;
    host = _host;
    password = _password;
    return 0;
}
int jnx_sql_query(char* query,void (*sql_callback)(MYSQL_RES*))
{
    if(connection == NULL) return 1;
    /* multi statements is useful for giving a string of commmands that are delimited with ; */
    if(mysql_real_connect(connection,host,username,password,0,0,NULL, CLIENT_MULTI_STATEMENTS) == NULL){ printf("CONNECTION ERROR\n"); return 1;};    
    int status = mysql_query(connection,query);
    if(status)
    {
        mysql_close(connection);
        printf("Problem with query\n");
        return 1;
    }
    /* process each statement result */
    do 
    {
        /* did current statement return data? */
        result = mysql_store_result(connection);
        if(result)
        {
            /* yes; process rows and free the result set */
            (*sql_callback)(result);
            mysql_free_result(result);
        }
        else          /* no result set or error */
        {
            if (mysql_field_count(connection) == 0)
            {
            }
            else  /* some error occurred */
            {
                printf("Could not retrieve result set\n");
                break;
            }
        }
        /* more results? -1 = no, >0 = error, 0 = yes (keep looping) */
        if ((status = mysql_next_result(connection)) > 0)
            printf("Could not execute statement in jnx_sql_query\n");
    } while (status == 0);
    return 0;
}
int jnx_sql_resultfill_query(char *query, MYSQL_RES **resultptr)
{
    if(connection == NULL) return 1;
    /* multi statements is useful for giving a string of commmands that are delimited with ; */
    if(mysql_real_connect(connection,host,username,password,0,0,NULL, CLIENT_MULTI_STATEMENTS) != connection) { printf("Connection error in jnx_sql_resultfill_query\n"); return 1; }
    int status = mysql_query(connection,query);
    if(status)
    {
        mysql_close(connection);
        printf("Problem with query\n");
        return 1;
    }
    /* process each statement result */
    do 
    {
        /* did current statement return data? */
        result = mysql_store_result(connection);
        if(result)
        {
            /* yes; process rows and free the result set */
            (*resultptr) = result;
        }
        else          /* no result set or error */
        {
            if (mysql_field_count(connection) == 0)
            {    
            }
            else  /* some error occurred */
            {
                printf("Could not retrieve result set\n");
                break;
            }
        }
        /* more results? -1 = no, >0 = error, 0 = yes (keep looping) */
        if ((status = mysql_next_result(connection)) > 0)
            printf("Could not execute statement in jnx_sql_resultfill_query\n");
    } while (status == 0);
    return 0;
}

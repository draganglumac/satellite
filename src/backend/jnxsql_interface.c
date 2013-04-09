#include "jnxsql_interface.h"
#include <stdio.h>
#include <stdlib.h>
#include "../utils.h"
MYSQL *connection;
MYSQL_RES *result;
MYSQL_ROW row;
char *host;
char *username;
char *password;

int jnx_sql_interface_setup(void);
void jnx_sql_close(void);

int perform_store_sql_credentials(char* host_addr, char* user, char* pass)
{
	host = host_addr;
	username = user;
	password = pass;
	int ret = 0;
	MYSQL_RES *result;
	if(jnx_sql_interface_setup() != 0)
	{
		print_streams(JNX_COL_RED,"Error connecting to sql\n");
		print_streams(DEFAULTCOLOR,"Error connecting to sql in perform_store_sql_credentials\n");
		return 1;
	}
	ret = jnx_sql_resultfill_query("use AUTOMATION; select 'test';",&result);
	if(ret == 0 ) { mysql_free_result(result); jnx_sql_close();};
	return ret;
}
void jnx_sql_close(void )
{
	if(connection == NULL) return;
	mysql_close(connection);
	connection = NULL;
}
int jnx_sql_interface_setup()
{
	connection = mysql_init(connection);
	if(connection == NULL) return 1;
	return 0;
}
int jnx_sql_query(char* query,void (*sql_callback)(MYSQL_RES*))
{
	jnx_sql_interface_setup();
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
	jnx_sql_close();
	return 0;
}
int jnx_sql_resultfill_query(char *query, MYSQL_RES **resultptr)
{
	jnx_sql_interface_setup();
	if(connection == NULL) 
	{
		jnx_term_printf_in_color(JNX_COL_RED,"Connecting to jnx_sql_resultfill_query is null\n");
		return 1;
	}
	/* multi statements is useful for giving a string of commmands that are delimited with ; */
	if(mysql_real_connect(connection,host,username,password,0,0,NULL, CLIENT_MULTI_STATEMENTS) != connection)
	{ 
		jnx_term_printf_in_color(JNX_COL_RED,"Connection error in jnx_sql_resultfill_query - Your request: %s\n",query);
		if(strcmp(query,"") == 0 ) { printf("query is empty\n");  } ;	
		/*-----------------------------------------------------------------------------
		 *  May require another return code for unsuccessful connection
		 *-----------------------------------------------------------------------------*/
		return 1;
	}
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
	jnx_sql_close();
	return 0;
}

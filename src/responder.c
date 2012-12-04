#include "responder.h"
#include <string.h>
#include <stdio.h>
void callback(MYSQL_RES* res)
{

}
void write_result(char* job_id)
{
	sql_callback _s = callback;

	jnx_sql_interface_setup("172.20.141.82","dummy","dummy");
	
	printf("JOB ID being written is %s\n",job_id);
	
	char query[256] = "UPDATE `AUTOMATION`.`jobs` SET status='COMPLETE' WHERE id=";
	
	//we dont want to modify the string literal so copy it

	char* cp = strdup(query);

	strcat(cp,job_id);
	strcat(cp,";");
	printf("%s\n",cp);

	jnx_sql_query(cp,_s);
	
	
	
}

#include "responder.h"
#include <string.h>
#include <stdio.h>
void callback(MYSQL_RES* res)
{

}
void write_result(char* job_id)
{
	sql_callback _s = callback;

	
	printf("JOB ID being written is %s",job_id);
	
	char query[128] = "UPDATE `AUTOMATION`.`jobs` SET status='COMPLETE' WHERE id=";
	char out[128];
	
	strcpy(query,out);
	strcat(query,job_id);
	
	jnx_sql_query(query,_s);
	
	
}

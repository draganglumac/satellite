#include "responder.h"
#include <string.h>

void callback(MYSQL_RES* res)
{

}
void write_result(int job_id)
{
	sql_callback _s = callback;

	jnx_sql_interface_setup("172.20.141.82","dummy","dummy");
	
	
	char query[128] = "UPDATE `AUTOMATION`.`jobs` SET status='INPROGRESS' WHERE id=";
	char out[128];
	
	strcpy(query,out);
	strcat(query,(char)(((int)'0')+job_id));
	
	jnx_sql_query(query,_s);
	
	jnx_sql_close();
}

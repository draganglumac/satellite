#include "responder.h"
#include <string.h>

void callback(MYSQL_RES* res)
{

}
void write_result(char* job_id)
{
	sql_callback _s = callback;

	jnx_sql_interface_setup("172.20.141.82","dummy","dummy");
	
	
	char query[128] = "UPDATE `AUTOMATION`.`jobs` SET status='COMPLETE' WHERE id=";
	char out[128];
	
	strcpy(query,out);
	strcat(query,job_id);
	
	jnx_sql_query(query,_s);
	
	jnx_sql_close();
}

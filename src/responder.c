#include "responder.h"
#include <string.h>
#include <stdio.h>

#define SQLDB "172.20.141.82"
#define SQLADMIN "dummy"
#define SQLPASS "dummy"

void callback(MYSQL_RES* res)
{

}
void write_result(char* job_id)
{
	sql_callback _s = callback;

	jnx_sql_interface_setup(SQLDB,SQLADMIN,SQLPASS);
	
	printf("JOB ID being written is %s\n",job_id);
	
	char query[256] = "UPDATE `AUTOMATION`.`jobs` SET status='COMPLETE' WHERE id=";
	
	//we dont want to modify the string literal so copy it

	char* cp = strdup(query);

	strcat(cp,job_id);
	strcat(cp,";");
	printf("%s\n",cp);

	jnx_sql_query(cp,_s);
	
	

	char result_one[256] = "INSERT INTO `AUTOMATION`.`results` (`id`,`testresult`,`jobs_id`,`jobs_machines_machine_id`)VALUES(NULL,'FAIL',";
	
	char* cp_two = strdup(result_one);
	strcat(cp_two,job_id);
	strcat(cp_two,",(select machines_machine_id from `AUTOMATION`.`jobs` where id=");
	strcat(cp_two,job_id);
	strcat(cp_two,"));");
	
	printf("Writing to results table -> %s\n",cp_two);
	//INSERT INTO `AUTOMATION`.`results` (`id`,`testresult`,`jobs_id`,`jobs_machines_machine_id`)VALUES(NULL,"FAIL",2,(select machines_machine_id from `AUTOMATION`.`jobs` where id=2));

}

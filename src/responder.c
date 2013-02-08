#include "responder.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
void callback(MYSQL_RES* result)
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
void write_result(char* job_id,char* sqlhost,char*sqluser,char*sqlpass)
{	
	if(setup_sql(sqlhost,sqluser,sqlpass) != 0)
	{
		printf("Critical error in SQL setup\n");
		exit(1);
	}
	if(job_id == NULL)
	{
		printf("Warning, job id is null therefore abandoning ship\n");
		exit(1);
	}
	sql_callback _s = callback;

	printf("JOB ID being written is %s\n",job_id);

	const char* query = "UPDATE `AUTOMATION`.`jobs` SET status='COMPLETE' WHERE id=";
	char cp[1024];
	strcpy(cp,query);
	strcat(cp,job_id);
	strcat(cp,";");
	printf("%s\n",cp);

	jnx_sql_query(cp,_s);


	const char* result_one = "INSERT INTO `AUTOMATION`.`results` (`id`,`DATETIME`,`testresult`,`jobs_id`,`jobs_machines_machine_id`)VALUES(NULL,CURRENT_TIMESTAMP,'";		

	char cp_two[256];
	strcpy(cp_two,result_one);

	const char *result = "COMPLETE";

	strcat(cp_two,result);
	strcat(cp_two,"',");
	strcat(cp_two,job_id);
	strcat(cp_two,",(select machines_machine_id from `AUTOMATION`.`jobs` where id=");
	strcat(cp_two,job_id);
	strcat(cp_two,"));");

	printf("Writing to results table -> %s\n",cp_two);
	jnx_sql_query(cp_two,_s);

	jnx_sql_close();
}


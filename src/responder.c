#include "responder.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

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

	char cp[1024];
	
	strcpy(cp,query);

	strcat(cp,job_id);
	strcat(cp,";");
	printf("%s\n",cp);

	jnx_sql_query(cp,_s);
	

	char result_one[256] = "INSERT INTO `AUTOMATION`.`results` (`id`,`testresult`,`jobs_id`,`jobs_machines_machine_id`)VALUES(NULL,'";
	
	char* cp_two[1024];
	strcpy(cp_two,result_one);
	
	
	FILE *fp = fopen("temp/test-reports/cuke.html","r");
	if(fp == NULL)
	{
		fprintf(stderr,"File error :\n");
		exit(0);
	}
	fseek(fp,0,SEEK_END);
	long int size = ftell(fp);
	rewind(fp);
	char *content = calloc(size + 1, 1);
	fread(content,1,size,fp);
	
	printf("HTML %s\n",content);
	
	
	char *res = "FAIL";
	strcat(cp_two,res);
	strcat(cp_two,"',");
	strcat(cp_two,job_id);
	strcat(cp_two,",(select machines_machine_id from `AUTOMATION`.`jobs` where id=");
	strcat(cp_two,job_id);
	strcat(cp_two,"));");
	
	printf("Writing to results table -> %s\n",cp_two);
	//INSERT INTO `AUTOMATION`.`results` (`id`,`testresult`,`jobs_id`,`jobs_machines_machine_id`)VALUES(NULL,"FAIL",2,(select machines_machine_id from `AUTOMATION`.`jobs` where id=2));
	jnx_sql_query(cp_two,_s);
}

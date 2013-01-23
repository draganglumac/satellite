#ifndef __RESPONDER_H__
#define __RESPONDER_H__
#include "jnxsql_interface.h"


void callback(MYSQL_RES*);
int setup_sql(char *host_addr,char *username, char*port);
void write_result(char* job_id);

#endif
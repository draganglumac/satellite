#ifndef __INTERFACE_H__
#define __INTERFACE_H__
#include "jnxsql_interface.h"

int response_from_db(void);
int store_sql_credentials(char* host_addr, char* username, char* pass);
int write_result_to_db(char *job_id,char *result);
#endif

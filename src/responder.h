#ifndef __RESPONDER_H__
#define __RESPONDER_H__
#include "jnxsql_interface.h"

void set_sql_data(char* sqlh,char* sqlu,char* sqlp);
//This uses a stored procedure to retrieve jobs that need to be run and outputs via 
//function pointer
int response_from_db(char *sqlhost, char*sqluser, char*sqlpass);
int write_result_to_db(char *job_id,char *result);
#endif

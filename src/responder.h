#ifndef __RESPONDER_H__
#define __RESPONDER_H__
#include "jnxsql_interface.h"


void callback(MYSQL_RES*);

void write_result(int job_id);

#endif
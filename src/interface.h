#ifndef __INTERFACE_H__
#define __INTERFACE_H__
#include "jnxsql_interface.h"
#include "interface_commands.h"
int perform_job_cycle(void);
int perform_store_sql_credentials(char* host_addr, char* username, char* pass);
#endif

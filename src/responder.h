#ifndef __RESPONDER_H__
#define __RESPONDER_H__
#include "jnxsql_interface.h"
int setup_sql(char *host_addr,char *username, char*port);
int response_from_db(char *sqlhost, char*sqluser, char*sqlpass);
#endif

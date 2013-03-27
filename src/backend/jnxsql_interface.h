#ifndef __JNXSQL_INTERFACE_H__
#define __JNXSQL_INTERFACE_H__
#ifdef __APPLE__
#include <mysql.h>
#else 
#include <mysql/mysql.h>
#endif
#include <jnxc_headers/jnxterm.h>
typedef void (*sql_callback)(MYSQL_RES*);

int perform_store_sql_credentials(char* host_addr, char* user, char* pass);
int jnx_sql_interface_setup();
int jnx_sql_query(char *query,void (*sql_callback)(MYSQL_RES*));
int jnx_sql_resultfill_query(char *query, MYSQL_RES** resultptr);
void jnx_sql_close();
#endif

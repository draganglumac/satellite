/*
 * =====================================================================================
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  03/07/13 10:56:03
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alex Jones (), alexsimonjones@gmail.com
 *   Organization:  
 *
 * =====================================================================================
*/
#ifndef __SQL_COMMANDS_H__
#define __SQL_COMMANDS_H__
#include "jnxsql_interface.h"
#include "../utils.h"
#include <jnxc_headers/jnxnetwork.h>
#include <jnxc_headers/jnxlog.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <jnxc_headers/jnxstring.h>

void sql_generic_sql_callback(MYSQL_RES *res);
char* sql_resolve_machine_ip(char *machine_number);
int sql_update_job_trigger(char *job_id);
int sql_set_job_progress(char *job_id,char*status);
int sql_transmit_job_orders(char *job_id,char *job_name, char *machine_ip, char *command);
MYSQL_RES *sql_get_candidate_jobs(void);
#endif

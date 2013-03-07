/*
 * =====================================================================================
 *
 *       Filename:  interface_commands.h
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
#ifndef __UTILS_H__
#define __UTILS_H__
#include "jnxsql_interface.h"
#include "utils.h"
#include <jnxc_headers/jnxnetwork.h>
#include <jnxc_headers/jnxlog.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <jnxc_headers/jnxstring.h>

extern char* sqlhost;
extern char* sqluser;
extern char* sqlpass;

int write_result_to_db(char *job_id,char *result_input);
void generic_sql_callback(MYSQL_RES *res);
char* resolve_machine_ip(char *machine_number);
int update_job_trigger(char *job_id);
int set_job_progress(char *job_id,char*status);
int transmit_job_orders(char *job_id,char *job_name, char *machine_ip, char *command);
//you are assuming responsibility to free this 
MYSQL_RES *get_incomplete_jobs(void);
#endif

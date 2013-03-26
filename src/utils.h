/*
 * =====================================================================================
 *
 *       Filename:  utils.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  03/07/13 12:23:45
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alex Jones (), alexsimonjones@gmail.com
 *   Organization:  
 *
 * =====================================================================================
 */
#include <jnxc_headers/jnxlog.h>
#include <jnxc_headers/jnxterm.h>
#include <jnxc_headers/jnxhash.h>
#include <jnxc_headers/jnxfile.h>
#define DEFAULTCOLOR -1
void print_streams(int col,const char* format, ...);
jnx_hashmap* utils_set_configuration(char *path);
int utils_check_trigger_time(char *time_, char *job_id);

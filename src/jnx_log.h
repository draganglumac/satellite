/*
 * =====================================================================================
 *
 *       Filename:  jnx_log.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  02/20/13 10:15:04
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *   Organization:  
 *
 * =====================================================================================
 */
#include <stdarg.h>
//setup the logger on a file path, will create the log if it doesn't exist
typedef enum { LOGWNEWLINE,LOGNOCHANGE} logopt;
int jnx_log_setup(char *path,logopt l);
//log message to be written to the logger file
void jnx_log(char *fmt,...);

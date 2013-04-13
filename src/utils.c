/*
 * =====================================================================================
 *
 *       Filename:  utils.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  03/07/13 12:25:21
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alex Jones (), alexsimonjones@gmail.com
 *   Organization:  
 *
 * =====================================================================================
 */
#include "utils.h"
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
int utils_check_trigger_time(char *time_, char *job_id)
{
	time_t current_time = time(0);
	time_t triggertime = atoi(time_);
	char *s = ctime(&triggertime);
	s[strlen(s)-1]=0; 
	long diff = (triggertime - current_time); 
	long  diff_time_h = diff / 3600;
	long diff_time_m = diff % 3600 /60;
	long diff_time_s = diff % 3600 % 60;
	print_streams(DEFAULTCOLOR,"Job %s is set to run at %s which is %dh %dm %ds from current\n",job_id,s,diff_time_h,diff_time_m,diff_time_s); 
	if((triggertime - current_time) < 0)
	{
		return -1;
	}
	if((triggertime - current_time) <= 60)
	{
		return 0;
	}

	return 1;
}
void print_streams(int fg_col,const char* format, ...)
{
	if(fg_col != -1)
		jnx_term_color(fg_col);

	char buffer[1024];
	va_list ap;
	va_start(ap, format);
	vsprintf(buffer,format,ap);
	va_end(ap);
	printf(buffer);

	if(fg_col > 0) 
		jnx_term_default();

	jnx_log(buffer);

}
jnx_hashmap* utils_set_configuration(char *path)
{
	jnx_hashmap *hash = jnx_hash_init(1024);
	jnx_file_kvp_node *kvpnode;
	if((kvpnode = jnx_file_read_keyvaluepairs(path,"=")) == NULL)
	{
		return NULL;
	}
	while(kvpnode)
	{
		jnx_hash_put(hash,kvpnode->key,kvpnode->value);
		kvpnode = kvpnode->next;
	}
	return hash;
}

/*
 * =====================================================================================
 *
 *       Filename:  jnx_results.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  04/01/13 21:03:19
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *   Organization:  
 *
 * =====================================================================================
 */
#include <stdlib.h>
#include "result_control.h"
#include "../network/jnx_network_additions.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <jnxc_headers/jnxfile.h>
#define OUTPUTDIR "output"
#include <errno.h>
#include <string.h>
char *current_id = NULL;
int jnx_result_setup(void)
{
    int retval = mkdir(OUTPUTDIR,S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    if ( retval != 0 && errno == EEXIST )
       retval = 0; 
	return retval;
}
int jnx_result_process_callback(const char *fpath,const struct stat *sb, int typeflag,struct FTW *ftwbuf)
{
	printf("File path: %s\n",fpath);	

	char *ext = strrchr(fpath,'.');
	if(!ext)
	{
		ext = "";
	}
	printf("File format is %s\n",ext);

	if(S_ISREG(sb->st_mode))
	{
		printf("Sending %s to jnx_network_post_file\n",fpath);
		jnx_network_post_file(fpath + ftwbuf->base, current_id);
	}
	return 0;
}
void jnx_result_process(char *job_id)
{
	current_id = job_id;
	nftw(OUTPUTDIR,jnx_result_process_callback,64,8|1);
	printf("Results finished processing and tree has been walked\n");
}
int jnx_result_teardown(void)
{
	printf("Called teardown \n");
	return system("rm -rf output");
}

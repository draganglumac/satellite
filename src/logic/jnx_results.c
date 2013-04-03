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
#include "jnx_results.h"
#include <sys/stat.h>
#include <stdio.h>
#define OUTPUTDIR "output"

int jnx_result_setup(void)
{
	return mkdir(OUTPUTDIR,S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
}
int jnx_result_process_callback(const char *fpath,const struct stat *sb, int typeflag,struct FTW *ftwbuf)
{
	printf("File path: %s\n",fpath);	
	/*-----------------------------------------------------------------------------
	 * Send files back over jnx_network connection 
	 *-----------------------------------------------------------------------------*/

}
void jnx_result_process(void)
{
	nftw(OUTPUTDIR,jnx_result_process_callback,64,8|1);
}
int jnx_result_teardown(void)
{
	return remove(OUTPUTDIR);
}

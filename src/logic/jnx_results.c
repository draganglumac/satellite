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
void jnx_result_process(void)
{

	/*-----------------------------------------------------------------------------
	 *  Check for entries in the directory
	 *-----------------------------------------------------------------------------*/
}
int jnx_result_teardown(void)
{
	/*-----------------------------------------------------------------------------
	 *  Remove the directory
	 *-----------------------------------------------------------------------------*/
	return remove(OUTPUTDIR);
}

/*
 * =====================================================================================
 *
 *       Filename:  test_transaction_api.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  08/19/13 09:52:30
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *   Organization:  
 *
 * =====================================================================================
 */
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <jnxc_headers/jnxterm.h>
#include "../src/logic/result_control.h"
#include <sys/types.h>
#include <sys/stat.h>
void test_directory_creation()
{
	printf("- test directory creation\n");
	int ret = jnx_result_setup();
	struct stat _st;
	if(stat("output",&_st) == -1)
	{
		jnx_term_printf_in_color(JNX_COL_RED,"  FAILED\n");
		abort();
	}else
	{
		if(S_ISDIR(_st.st_mode))
		{
			jnx_term_printf_in_color(JNX_COL_GREEN,"  OK\n");
		}else
		{

			jnx_term_printf_in_color(JNX_COL_RED,"  FAILED\n");
			abort();
		}
	}
}
void test_directory_deletion()
{
	printf("- test directory deletion\n");
	int ret = jnx_result_teardown();
	struct stat _st;
	if(stat("output",&_st) == -1)
	{
			jnx_term_printf_in_color(JNX_COL_GREEN,"  Ok\n");
	}else
	{
		jnx_term_printf_in_color(JNX_COL_RED,"  FAILED\n");
		abort();
	}
}
int main(int argc, char **argv)
{
	printf("Running test for result_control\n");

	test_directory_creation();
	test_directory_deletion();
	
	jnx_term_printf_in_color(JNX_COL_GREEN,"  OK\n");
	printf("result_control tests completed.\n");
	return 0;
}

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
#include "../src/network/transaction_api.h"
int main(int argc, char **argv)
{
	printf("Running test for transaction_api\n");

	printf("- test object building\n");

	char *test_string = "[{CMD`0}{ID`1}{DATA`TEST DATA}{OTHER`NULL}{SENDER`localhost}{PORT`9090}]";

	api_command_obj *obj = transaction_api_create_obj(test_string);

	assert(obj);
	jnx_term_printf_in_color(JNX_COL_GREEN,"  OK\n");
	printf("- test object properties\n");
	assert(strcmp(obj->DATA,"TEST DATA") == 0);
	assert(strcmp(obj->SENDER,"localhost") == 0);
	assert(obj->PORT == 9090);
	assert(strcmp(obj->OTHER,"NULL") == 0);
	jnx_term_printf_in_color(JNX_COL_GREEN,"  OK\n");
	printf("transaction_api tests completed.\n");
	return 0;
}

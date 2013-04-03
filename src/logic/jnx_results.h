/*
 * =====================================================================================
 *
 *       Filename:  jnx_results.h
 *
 *    Description:  results 
 *
 *        Version:  1.0
 *        Created:  04/01/13 20:52:43
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *   Organization:  
 *
 * =====================================================================================
 */
#ifndef __JNX_RESULTS_H__
#define __JNX_RESULTS_H__
#define __USE_XOPEN_EXTENDED
#include <ftw.h>
#include <jnxc_headers/jnxfile.h>
int jnx_result_setup(void);
void jnx_result_process(void);
int jnx_result_teardown(void);
int jnx_result_process_callback(const char *fpath,const struct stat *sb, int typeflag,struct FTW *ftwbuf);
#endif

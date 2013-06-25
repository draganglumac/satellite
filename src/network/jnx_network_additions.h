/*
 * =====================================================================================
 *
 *       Filename:  jnx_network_additions.h
 *
 *    Description:  Additional network functions mainly over straight HTTP
 *
 *        Version:  1.0
 *        Created:  03/04/2013 14:15:57
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *   Organization:  
 *
 * =====================================================================================
 */
#ifndef __JNX_NETWORK_ADDITIONS_H__
#define __JNX_NETWORK_ADDITIONS_H__
typedef enum { TEXT, BIN } file_type;
int jnx_network_post_file(file_type f,const char *filepath, char *jobid);
#endif




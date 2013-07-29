/*
 * =====================================================================================
 *
 *       Filename:  job_control.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  09/07/2013 10:46:45
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *   Organization:  
 *
 * =====================================================================================
 */
#ifndef __JOB_CONTROL_H__
#define __JOB_CONTROL_H__
#include "../network/transaction_api.h"
#define LISTENPORT 9090
#ifdef __APPLE__ 
#include <stddef.h>
#define INTERFACE "en0"
#else
#define INTERFACE "eth0"
#endif
void job_control_start_listening(void);
void job_control_start_processing(void);
int query(char *hostaddr, char* hostport, const char *template, ...);
int lquery(char *hostaddr, char *hostport,size_t data_offset, const char *template, ...);
#endif

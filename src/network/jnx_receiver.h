/*
 * =====================================================================================
 *
 *       Filename:  jnx_receiver.h
 *
 *    Description:  All communications received will be dealt with here
 *
 *        Version:  1.0
 *        Created:  03/26/13 15:35:51
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alex Jones (), alexsimonjones@gmail.com
 *   Organization:  
 *
 * =====================================================================================
 */
#ifndef __JNX_RECEIVER_H__
#define __JNX_RECEIVER_H__
#include <jnxc_headers/jnxnetwork.h>
int jnx_start_listener(char *listener_port);

#endif

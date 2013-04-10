/*
 * =====================================================================================
 *
 *       Filename:  jnx_infrastructure.h
 *
 *    Description:  Handles the multicast send and receive
 *
 *        Version:  1.0
 *        Created:  04/10/13 11:24:05
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *   Organization:  
 *
 * =====================================================================================
 */
#ifndef __JNX_INFRASTRUCTURE_H__
#define __JNX_INFRASTRUCTURE_H__
#define BPORT 12345
#define BGROUP "225.0.0.37"
void jnx_infrastructure_broadcast_listen(void);
void jnx_infrastructure_broadcast_send(char *message);
#endif

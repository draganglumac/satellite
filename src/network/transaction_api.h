/*
 * =====================================================================================
 *
 *       Filename:  transaction_api.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  07/01/13 13:54:11
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  AlexsJones (), alexsimonjones@gmail.com
 *   Organization:  
 *
 * =====================================================================================
 */
#ifndef __TRANSACTION_API_H__
#define __TRANSACTION_API_H__
/*-----------------------------------------------------------------------------
 *  Define CMD
 *  - JOB
 *  - RESULT
 *  - STATUS
 *-----------------------------------------------------------------------------*/
#define API_COMMAND "[{CMD`%s}{ID`%s}{DATA`%s}{OTHER`%s}{SENDER`%s}{PORT`%s}]"
typedef enum command_type{ JOB, RESULT, STATUS,UNKNOWN }command_type;
typedef struct{
	command_type CMD;
	char *ID;
	char *DATA;
	char *OTHER;
	char *SENDER;
	int PORT;
}api_command_obj;

api_command_obj *transaction_api_create_obj(char *query);
void transaction_api_delete_obj(api_command_obj *obj);
#endif

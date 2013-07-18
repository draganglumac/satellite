/*
 * =====================================================================================
 *
 *       Filename:  transaction_api.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  07/02/13 12:14:17
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  AlexsJones (), alexsimonjones@gmail.com
 *   Organization:  
 *
 * =====================================================================================
 */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "transaction_api.h"
api_command_obj *transaction_api_create_obj(char *query)
{	
	char *buffer = query;
	if(strlen(query) <= 0)
	{
		return NULL;
	
	}//Too small to even be a real part of the API
	else if(strlen(API_COMMAND) > strlen(query))
	{
		return NULL;
	}
	//Create the data structure
	api_command_obj *cmd_obj = malloc(sizeof(api_command_obj));
	while(*buffer != ']')
	{
		if(*buffer == '{')
		{
			*buffer++;
			char *line = malloc(sizeof(char));
			bzero(line,sizeof(char));
			while(*buffer != '}')
			{
				line = realloc(line,strlen(line) + sizeof(char) + 1);
				sprintf(line,"%s%c",line,*buffer);
				*buffer++;
			}
			//split the current KVP 
			char *candidate_string;
			char *split_string = strtok_r(line,"`",&candidate_string);


			/*-----------------------------------------------------------------------------
			 *  Will need to strdup into the char arrays before freeing line!
			 *-----------------------------------------------------------------------------*/
			if(strcmp(split_string,"CMD") == 0)
			{
				char *raw_command = strtok_r(NULL,"`",&candidate_string);
				if(!raw_command)
				{
					cmd_obj->CMD = UNKNOWN;
				}
				if(strcmp(raw_command,"JOB") == 0)
				{
					cmd_obj->CMD = JOB;
				}
				else if(strcmp(raw_command,"RESULT") == 0)
				{
					cmd_obj->CMD = RESULT;
				}else if(strcmp(raw_command,"STATUS") == 0)
				{
					cmd_obj->CMD = STATUS;
				}
				else if(strcmp(raw_command,"ALIVE") == 0)
				{
					cmd_obj->CMD = ALIVE;
				}
				else if(strcmp(raw_command,"SYSTEM") == 0)
				{
					cmd_obj->CMD = SYSTEM;
				}
				else
				{
					cmd_obj->CMD = UNKNOWN;
				}
				free(line);
			}
			else if(strcmp(split_string,"ID") == 0)
			{
				char *id = strtok_r(NULL,"`",&candidate_string);
				if(!id){
				cmd_obj->ID = strndup("NULL",strlen("NULL"));
				}else{
				cmd_obj->ID = strndup(id,strlen(id));
				}
				free(line);
			}
			else if(strcmp(split_string,"DATA") == 0)
			{
				char *data = strtok_r(NULL,"`",&candidate_string);
				if(!data){
					cmd_obj->DATA = strndup("NULL",strlen("NULL"));
				}else{
					cmd_obj->DATA = strndup(data,strlen(data));
				}
				free(line);
			}
			else if(strcmp(split_string,"OTHER") == 0)
			{
				char *other = strtok_r(NULL,"`",&candidate_string);
				if(!other)
				{
					cmd_obj->OTHER = strndup("NULL",strlen("NULL"));
				}else{
					cmd_obj->OTHER = strndup(other,strlen(other));
				}
				free(line);
			}
			else if(strcmp(split_string,"SENDER") == 0)
			{
				char *sender = strtok_r(NULL,"`",&candidate_string);
				if(!sender)
				{
					cmd_obj->SENDER = strndup("NULL",strlen("NULL"));
				}
				else{
					cmd_obj->SENDER = strndup(sender,strlen(sender));
				}
				free(line);
			}
			else if(strcmp(split_string,"PORT") == 0)
			{
				char *port = strtok_r(NULL,"`",&candidate_string);
				if(!port)
				{
					cmd_obj->PORT = -1;
				}else{
					cmd_obj->PORT = atoi(port);
				}
				free(line);
			}
			else
			{
				printf("Unable to match key from predefined expected list\n");
				return NULL;
			}
		}
		*buffer++;
	}

	return	cmd_obj;
}
void transaction_api_delete_obj(api_command_obj *obj)
{
	if(obj == NULL)
	{
		return;
	}
	free(obj->ID);
	free(obj->OTHER);
	free(obj->SENDER);
	free(obj->DATA);
	free(obj);
}

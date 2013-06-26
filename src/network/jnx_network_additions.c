/*
 * =====================================================================================
 *
 *       Filename:  jnx_network_additions.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  03/04/2013 14:22:02
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *   Organization:  
 *
 * =====================================================================================
 */
#include <jnxc_headers/jnxfile.h>
#include <jnxc_headers/jnxhash.h>
#include <jnxc_headers/jnxnetwork.h>
#include <stdio.h>
#include <stdlib.h>
#include "jnx_network_additions.h"
#include "../base64.h"
extern jnx_hashmap *config;
void jnx_network_post_file_callback(char *message)
{

}
int jnx_network_post_file(file_type f,const char *filepath, char *jobid)
{
	char sendline[MAXBUFFER];
	bzero(&sendline, MAXBUFFER);


	char *fcont = NULL;
	size_t size = 0;
	char *rel_path = NULL;
	switch(f)
	{
		case TEXT:
			printf("TEXT MODE TRANSMISSION\n");
			size = jnx_file_read((char*)filepath,&fcont); 
			
			
			rel_path = "/upload/%s/%s";	
			break;
		case BIN:
			printf("BIN MODE TRANSMISSION\n");
			printf("File name %s\n",filepath);
			//filename + 1
			FILE *fp = fopen(filepath,"r");
			if(fp == NULL)
			{
				return 1;
			}	
			fseek(fp,0,SEEK_END);
			long int fp_size = ftell(fp);
			rewind(fp);
			printf("size is %d\n",fp_size);
			char *data = calloc(fp_size,sizeof(char));
			fread(data,fp_size,sizeof(char),fp);
			fclose(fp);
			fcont  = base64_encode(data,fp_size,&size);	
			free(data);	
			rel_path = "/upload/%s/%s/bin";
			break;
	}
	if ( size < 0 ||  fcont == NULL)
	{
		printf("send_file_to_server error in determining the size of the file...");
		return 1;
	}

	char relative_path_buffer[256];
	snprintf(relative_path_buffer,256,rel_path,jobid,filepath);
	printf("jnx_network_post_file relative path : %s\n",relative_path_buffer);

	char *post = "POST %s HTTP/1.1\r\n" 
		"Host: %s:%s\r\n"
		"Content-Type: text/plain\r\n"
		"Content-Length: %d\r\n"
		"\r\n";

	snprintf(sendline,MAXBUFFER,post,relative_path_buffer,jnx_hash_get(config,"frontendserver"),jnx_hash_get(config,"frontendport"),size);
	printf("TOTAL RESULT %s\n",sendline);
	size_t msgsize = strlen(sendline) + size + 1;
	char *buffy = malloc(msgsize);
	bzero(buffy, msgsize);
	strcpy(buffy, sendline);
	strcat(buffy, fcont);

	jnx_network_send_message_callback c = jnx_network_post_file_callback;
	printf("SENDING MESSAGE!!!!\n");
	if(jnx_network_send_message(jnx_hash_get(config,"frontendserver"),atoi(jnx_hash_get(config,"frontendport")),buffy, c) == 1)
	{
		printf("Failed to transmit %s\n",filepath);
		return 1;
	}
	printf("Sent\n");
	free(buffy);
	free(fcont);
	return 0;
}

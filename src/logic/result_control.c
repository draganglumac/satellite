/*
 * =====================================================================================
 *
 *       Filename:  jnx_results.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  04/01/13 21:03:19
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *   Organization:  
 *
 * =====================================================================================
 */
#include <stdlib.h>
#include "../network/transaction_api.h"
#include "result_control.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <jnxc_headers/jnxbase64.h>
#include <jnxc_headers/jnxfile.h>
#define OUTPUTDIR "output"
#include <errno.h>
#include <string.h>
int query(char *hostaddr, char* hostport, const char *template, ...);
char *current_id = NULL;
char *current_host = NULL;
char *current_port = NULL;
char *current_sender_ip = NULL;
char *current_sender_port = NULL;
int accepted_file_format_count = 5;
char *accepted_file_formats[5] =
{
	"jpeg","jpg","txt","html","png"
};

int jnx_result_setup(void)
{
	int retval = mkdir(OUTPUTDIR,S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	if ( retval != 0 && errno == EEXIST )
		retval = 0; 
	return retval;
}
int jnx_result_process_callback(const char *fpath,const struct stat *sb, int typeflag,struct FTW *ftwbuf)
{
	printf("File path: %s\n",fpath);	
	char *ext = strrchr(fpath,'.');
	char *filename = strrchr(fpath,'/');
	if(filename)
	{
		//removing our slash from the beginning
		filename = filename +1;
	}
	if(!ext)
	{
		ext = "";
	}
	printf("File format is %s\n",ext);

	//check the fileformat is on the approved list before continuing.
	if(S_ISREG(sb->st_mode))
	{
		int count; 
		for(count = 0; count < accepted_file_format_count; ++count)
		{
			if(strcmp(accepted_file_formats[count], ext +1) == 0)
			{
				printf("File format is on approved list, sending through\n");
				const char* filepath = fpath + ftwbuf->base;
				char *raw;	
				size_t bytes_read = jnx_file_readb((char*)filepath,&raw);
				size_t outputlen;
				char *encoded_string = jnx_base64_encode(raw,bytes_read,&outputlen);
				printf("Encoded string %s\n",encoded_string);

				query(current_host,current_port,API_COMMAND,"RESULT",current_id,encoded_string,filename,current_sender_ip,current_sender_port);
			}
		}
	}
	return 0;
}
void jnx_result_process(char *host, char *port,char *job_id, char *sender_ip, char *sender_port)
{
	current_id = job_id;
	current_host = host;
	current_port = port;
	current_sender_ip = sender_ip;
	current_sender_port = sender_port;
	nftw(OUTPUTDIR,jnx_result_process_callback,64,8|1);
	printf("Results finished processing and tree has been walked\n");
}
int jnx_result_teardown(void)
{
	printf("Called teardown \n");
	return system("rm -rf output");
}

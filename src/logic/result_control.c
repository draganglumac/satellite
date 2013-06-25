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
#include "result_control.h"
#include "../network/jnx_network_additions.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <jnxc_headers/jnxfile.h>
#define OUTPUTDIR "output"
#include <errno.h>
#include <string.h>
#include "../base64.h"
char *current_id = NULL;
int accepted_file_format_count = 4;
char *accepted_file_formats[4] =
{
	"jpeg","jpg",".txt",".html"
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
		
				if((strcmp(ext +1,"jpeg")) == 0 || (strcmp(ext +1,"jpg")) == 0)
				{	
					//what directory are we in?
					//open the file 
					char *filename = strrchr(fpath,'/');
					if(!filename)
					{
						//something went wrong here 
						perror("jnx_result_process_callback ");
						return 1;
					}
					//filename + 1
					
					FILE *fp = fopen(filename +1,"r");
					if(fp == NULL)
					{
						perror("jnx_result_process_callback ");
						return 1;
					}	
					fseek(fp,0,SEEK_END);
					long int fp_size = ftell(fp);
					rewind(fp);
					char *data = calloc(fp_size,sizeof(char));

					fread(data,fp_size,sizeof(char),fp);
				
					fclose(fp);
					size_t *output_len;

					char *encoded_data = base64_encode(data,fp_size,output_len);	
	
					printf("%s",encoded_data);
					//encode the file
					jnx_network_post_file(BIN,fpath + ftwbuf->base, current_id);
					
					//transmit the file
				}else
				{
					jnx_network_post_file(TEXT,fpath + ftwbuf->base, current_id);
				}
			}
		}
	
	}
	return 0;
}
void jnx_result_process(char *job_id)
{
	current_id = job_id;
	nftw(OUTPUTDIR,jnx_result_process_callback,64,8|1);
	printf("Results finished processing and tree has been walked\n");
}
int jnx_result_teardown(void)
{
	printf("Called teardown \n");
	return system("rm -rf output");
}

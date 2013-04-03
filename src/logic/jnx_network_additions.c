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
extern jnx_hashmap *config;
void jnx_network_post_file_callback(char *message)
{

}
int file_size(char *path)
{


	printf("Checking file size of %s\n", path);
	FILE *fp = NULL;

	if ((fp = fopen(path, "r")) == NULL) {
		perror("file: ");
		return -1;
	}
	if(fseek(fp, 0, SEEK_END) != 0)
	{
		perror("file: ");
		exit(1);
	}

	int size = ftell(fp);
	fclose(fp);

	return size;
}
int jnx_network_post_file(char *filepath, char *jobid)
{
	char sendline[MAXBUFFER];
	bzero(&sendline, MAXBUFFER);
    
    char *fcont = jnx_file_read(filepath); 
    int size = file_size(filepath);
    if ( size < 0 )
    {
        printf("send_file_to_server error in determining the size of the file...");
        return 1;
    }

    char *rel_path = "/upload/%s/%s";
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
    if(jnx_network_send_message(jnx_hash_get(config,"frontendserver"),atoi(jnx_hash_get(config,"frontendport")),buffy, c) == 1)
	{
		printf("Failed to transmit %s\n",filepath);
		return 1;
	}
	free(buffy);
	return 0;
}

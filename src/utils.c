/*
 * =====================================================================================
 *
 *       Filename:  utils.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  03/07/13 12:25:21
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alex Jones (), alexsimonjones@gmail.com
 *   Organization:  
 *
 * =====================================================================================
 */
#include "utils.h"
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <stdio.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <time.h>

int utils_check_trigger_time(char *time_, char *job_id)
{
	time_t current_time = time(0);
	time_t triggertime = atoi(time_);
	char *s = ctime(&triggertime);
	s[strlen(s)-1]=0; 
	long diff = (triggertime - current_time); 
	long  diff_time_h = diff / 3600;
	long diff_time_m = diff % 3600 /60;
	long diff_time_s = diff % 3600 % 60;
	print_streams(DEFAULTCOLOR,"Job %s is set to run at %s which is %dh %dm %ds from current with current status of ",job_id,s,diff_time_h,diff_time_m,diff_time_s); 
	if((triggertime - current_time) < 0)
	{
		return ALREADYRUN;
	}
	if((triggertime - current_time) <= 60)
	{
		return READYTORUN;
	}
	return NOTREADYTORUN;
}
void print_streams(int fg_col,const char* format, ...)
{
	if(fg_col != -1)
		jnx_term_color(fg_col);

	char buffer[1024];
	va_list ap;
	va_start(ap, format);
	vsprintf(buffer,format,ap);
	va_end(ap);
	printf(buffer);

	if(fg_col > 0) 
		jnx_term_default();

	jnx_log(buffer);
}
jnx_hashmap* utils_set_configuration(char *path)
{
	jnx_hashmap *hash = jnx_hash_init(1024);
	jnx_file_kvp_node *kvpnode;
	if((kvpnode = jnx_file_read_keyvaluepairs(path,"=")) == NULL)
	{
		return NULL;
	}
	while(kvpnode)
	{
		jnx_hash_put(hash,kvpnode->key,kvpnode->value);
		kvpnode = kvpnode->next;
	}
	return hash;
}
int calcDecodeLength(const char* b64input) { //Calculates the length of a decoded base64 string
	int len = strlen(b64input);
	int padding = 0;

	if (b64input[len-1] == '=' && b64input[len-2] == '=') //last two chars are =
		padding = 2;
	else if (b64input[len-1] == '=') //last char is =
		padding = 1;

	return (int)len*0.75 - padding;
}
int Base64Decode(char* b64message, char** buffer) { //Decodes a base64 encoded string
	BIO *bio, *b64;
	int decodeLen = calcDecodeLength(b64message),
		len = 0;
	*buffer = (char*)malloc(decodeLen+1);
	FILE* stream = fmemopen(b64message, strlen(b64message), "r");

	b64 = BIO_new(BIO_f_base64());
	bio = BIO_new_fp(stream, BIO_NOCLOSE);
	bio = BIO_push(b64, bio);
	BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL); //Do not use newlines to flush buffer
	len = BIO_read(bio, *buffer, strlen(b64message));
	//Can test here if len == decodeLen - if not, then return an error
	(*buffer)[len] = '\0';

	BIO_free_all(bio);
	fclose(stream);

	return (0); //success
}
int utils_base64_encode(const char* message, char **buffer)
{
	BIO *bio, *b64;
	size_t size;
	FILE* stream;
	int encodedSize = 4*ceil((double)strlen(message)/3);
	*buffer = (char *)malloc(encodedSize+1);

	stream = fmemopen(*buffer, encodedSize+1, "w");
	b64 = BIO_new(BIO_f_base64());
	bio = BIO_new_fp(stream, BIO_NOCLOSE);
	bio = BIO_push(b64, bio);
	BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL); //Ignore newlines - write everything in one line
	BIO_write(bio, message, strlen(message));
	BIO_flush(bio);
	BIO_free_all(bio);
	fclose(stream);

	return (0); //success
}
int utils_base64_decode(char *b64message,char **buffer)
{
	BIO *bio, *b64;
	int decodeLen = calcDecodeLength(b64message),
		len = 0;
	*buffer = (char*)malloc(decodeLen+1);
	FILE* stream = fmemopen(b64message, strlen(b64message), "r");

	b64 = BIO_new(BIO_f_base64());
	bio = BIO_new_fp(stream, BIO_NOCLOSE);
	bio = BIO_push(b64, bio);
	BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL); //Do not use newlines to flush buffer
	len = BIO_read(bio, *buffer, strlen(b64message));
	//Can test here if len == decodeLen - if not, then return an error
	(*buffer)[len] = '\0';

	BIO_free_all(bio);
	fclose(stream);

	return (0); //success
}

/*
 * =====================================================================================
 *
 *       Filename:  base64.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  06/25/13 16:52:30
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *   Organization:  
 *
 * =====================================================================================
 */
#ifndef __BASE64_H__
#define __BASE64_H__
char *base64_encode(const unsigned char *data,
		size_t input_length,
		size_t *output_length);

unsigned char *base64_decode(const char *data,
		size_t input_length,
		size_t *output_length);
void build_decoding_table();
#endif

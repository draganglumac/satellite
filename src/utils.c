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

void print_streams(int fg_col,const char* format, ...)
{
    jnx_term_color(fg_col);
    va_list ap;
    va_start(ap, format);
    vprintf(format, ap);
    
    char temp[1024];
    sprintf(temp,format,ap);
//    jnx_log(temp); 
    va_end(ap);

    jnx_term_default();
}

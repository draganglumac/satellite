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
#include <string.h>
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

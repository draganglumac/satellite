/*
 * =====================================================================================
 *
 *       Filename:  jnx_log.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  02/20/13 10:17:02
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *   Organization:  
 *
 * =====================================================================================
 */
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include "jnx_log.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
char *log_path = NULL;
logopt _logoption;

int jnx_log_setup(char *path,logopt l)
{
    if(l >= 2) { printf("Unknown log option\n"); return 1; };
    struct stat s;
    int err = stat(path,&s);
    if(-1 == err)
    {
        printf("No log file at %s\n",path);
        printf("Creating new log\n");

        FILE *fp = fopen(path,"w+");
        if(fp == NULL)
        {
            printf("Error creating log file\n");
            return 1;
        }
        else
        {
            printf("Done\n");
            fclose(fp);
            //after our successful log creation, we'll assign the logpath
            log_path = path;
        } 

    }else
    {
        if(S_ISDIR(s.st_mode))
        {
            printf("Exists but is a directory\n");
            return 1;
        }
        printf("Found existing log, will continue logging\n");
        //assigning to our global log path
        log_path = path;
    } 
    return 0;
}
//warning, have taken the liberty of auto new line
void *jnx_write_to_log(char *message)
{
    FILE *fp = fopen(log_path,"a");
    if(fp == NULL) { printf("Unable to open file for log writing\n");};
    switch(_logoption){
        case LOGWNEWLINE:
            fprintf(fp,"%s\n",message);
            break;
        case LOGNOCHANGE:
            fprintf(fp,"%s",message);
            break;
        default:
            fprintf(fp,"%s",message);
    }
    fclose(fp);
}
void jnx_log(char *fmt,...)
{
    char str_buffer[256];
    int counter;

    va_list args;
    va_start(args,fmt);
    vsprintf(str_buffer,fmt,args);
    //str_buffer at this point has the current argument
    //writes to the log for each argument in the args list 
    pthread_t _thr;
    pthread_create(&_thr,NULL,(void*)jnx_write_to_log,(void*)str_buffer);
    pthread_join(_thr,NULL);
    va_end(args);  

    //    pthread_t _thr;
    //    jnx_write_to_log(str_buffer);
}


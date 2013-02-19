#include <jnxc_headers/jnxnetwork.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <signal.h>
#include <string.h>
#include <sys/stat.h>
#include <jnxc_headers/jnxstring.h>
#include <jnxc_headers/jnxlist.h>
#include <jnxc_headers/jnxfile.h>
#include <jnxc_headers/jnxterm.h>
#include "responder.h"
#define TIMEWAIT 5
char *sql_h = NULL, *sql_u = NULL, *sql_p = NULL;
enum ERROR_CODE { SER_UP, ARG_UP,SQL_UP }; 
void usage()
{
    printf("Satellite is a half duplex server/client in one for transmission of several shell commands\n");
    printf("No mode selected, please try again using -m\n");
    printf("Using -m LISTEN will enable listener mode where you will be asked to provide -p [PORT] --sqlhost --sqluser --sqlpass\n");	
    printf("Using -m SEND will enable a daemon that sends jobs it finds in the database that need to be completed. Requires: --sqlhost --sqluser --sqlpass\n");
}
void catch_int (int signum) 
{ 
    pid_t my_pid;
    printf("\nReceived an interrupt! About to exit ..\n");
    jnx_cancel_listener();
    fflush(stdout);
    my_pid = getpid();
    kill(my_pid, SIGKILL);	
}
void server_update(char *received_msg)
{
    printf("Raw received message: %s of length %d\n",received_msg,(int)strlen(received_msg));

    char *delimiter = "!";
    char *job_id = NULL;
    char *command = NULL;
    char cp[1024];
    strcpy(cp,received_msg);
    char *token = strtok(cp,delimiter);
    printf("COMMAND: %s\n",command);
    command = token;
    token = strtok(NULL,delimiter);
    //We should really chop off the job so it doesnt come out as a system error
    job_id = token;
    printf("Job ID: %s\n",job_id);
    int ret = system(command);
    if(ret != 0)
    {
        printf("Error with execution of %s : System returned %d\n",received_msg,ret);
    }
    printf("Execution completed\n");
    //this step sets up our sql database globals
    set_sql_data(sql_h,sql_u,sql_p);
    if(write_result_to_db(job_id,"COMPLETED") != 0)
    {
	    printf("Error with write_result_to_db\n");
    	    //this needs to be logged or exit
    }
}
int main(int argc, char **argv) 
{
    //Register for signal handling
    signal(SIGINT, catch_int);
    static struct option long_options[] = 
    {
        {"mode",required_argument,0,'m'},
        {"port",required_argument,0,'p'},
        {"sqlhost",required_argument,0,'s'},
        {"sqluser",required_argument,0,'u'},
        {"sqlpass",required_argument,0,'w'},
        {0,      0,                 0, 0 }
    };
    int option_index = 0;
    int i;
    int port;
    char* mode = NULL;
    while(( i = getopt_long_only(argc,argv,"m:p:s:u:w:",long_options,&option_index)) != -1)
    {
        switch(i)
        {
            case 'm':
                mode = optarg;
                break;
            case 'p':
                port = atoi(optarg);
                break;
            case 's':
                sql_h = optarg;
                break;
            case 'u':
                sql_u = optarg;
                break;
            case 'w':
                sql_p = optarg;
                break;
            default:
                usage();
                exit(1);
        }
    }
    if(mode == NULL)
    {
        usage();
        return ARG_UP;
    }
    if(strcmp(mode,"LISTEN") == 0)
    {
        if(!port) { printf("Requires port number, option -p\n");return 1; };
        printf("Starting server on port %d\n",port);
        printf("Saving sql data as : %s %s %s\n",sql_h,sql_u,sql_p);
	set_sql_data(sql_h,sql_u,sql_p);
	jnx_listener_callback c = &server_update;
        jnx_setup_listener(port,c);
        return 0;
    }
    if(strcmp(mode,"SEND") == 0)
    {
        if(!sql_h || !sql_u || ! sql_p) { usage(); exit(1); }
        printf("Sender mode\n");
        printf("Starting deamon\n");
        printf("Saving sql data as : %s %s %s\n",sql_h,sql_u,sql_p);
        while(1)
        {
            //setjmp is the return point after the sql functions have left the main loop
            printf("Checking sql\n");
            if(response_from_db(sql_h,sql_u,sql_p) != 0)
            {
                printf("An error occured with sql request\n");
                exit(1);
            }  
            
            sleep(TIMEWAIT);
        }
        return 0;
    }

    return 0;
}



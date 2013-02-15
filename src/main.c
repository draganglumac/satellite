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
#define TIMEWAIT 10
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
    char cp[1024];
    strcpy(cp,received_msg);
    char *token = strtok(cp,delimiter);
    token = strtok(NULL,delimiter);
    //We should really chop off the job so it doesnt come out as a system error
    printf("Job ID is %s\n",token);
    int ret = system(received_msg);
    if(ret != 0)
    {
        printf("Error with execution of %s : System returned %d\n",received_msg,ret);
    }

    printf("Execution completed\n");
    printf("Writing information to database\n");

    //lets have a form of capturing whether we've got a job id
    if(token == NULL)
    {
        printf("Could not find a job ID, therefore not proceeding with writing results to sql\n");
    }
    else
    {
        printf("This is where we should write the output of the job back to the sinatra API\n");
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
    char *sqlhost = NULL, *sqluser = NULL, *sqlpass = NULL;
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
                sqlhost = optarg;
                break;
            case 'u':
                sqluser = optarg;
                break;
            case 'w':
                sqlpass = optarg;
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
        printf("Saving sql data as : %s %s %s\n",sqlhost,sqluser,sqlpass);
        jnx_listener_callback c = &server_update;
        jnx_setup_listener(port,c);
        return 0;
    }
    if(strcmp(mode,"SEND") == 0)
    {
        if(!sqlhost || !sqluser || ! sqlpass) { usage(); exit(1); }
        printf("Sender mode\n");
        printf("Starting deamon\n");
        printf("Saving sql data as : %s %s %s\n",sqlhost,sqluser,sqlpass);
        while(1)
        {
            printf("Checking sql\n");
            if(response_from_db(sqlhost,sqluser,sqlpass) != 0)
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



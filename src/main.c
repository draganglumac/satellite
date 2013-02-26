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
#include <jnxc_headers/jnxhash.h>
#include <jnxc_headers/jnxlog.h>
#include "interface.h"
#define TIMEWAIT 5
jnx_hashmap *config;
enum ERROR_CODE { SER_UP, ARG_UP,SQL_UP }; 
void usage()
{
    printf("Satellite is a half duplex server/client in one for transmission of several shell commands\n");
    printf("No mode selected, please try again using -m [TRANSMIT,RECEIVE]\n");
    printf("--conf [PATH TO CONF FILE]\n");
}
void catch_int (int signum) 
{ 
    pid_t my_pid;
    jnx_log("\nReceived an interrupt! About to exit ..\n");

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
    command = token;
    printf("COMMAND: %s\n",command);
    token = strtok(NULL,delimiter);
    //We should really chop off the job so it doesnt come out as a system error
    job_id = token;
    printf("Job ID: %s\n",job_id);
    int ret = system(command);
    if(ret != 0)
    {
        printf("Error with execution of %s : System returned %d\n",received_msg,ret);
        jnx_log("Error with execution of command");
    }
    printf("Execution completed\n");
    //this step sets up our sql database globals
    if(write_result_to_db(job_id,"COMPLETED") != 0)
    {
        printf("Error with write_result_to_db\n");
        jnx_log("Error writing write_result_to_db");
        //this needs to be logged or exit
    }
    if(set_job_progress(job_id,"COMPLETED")!=0)
    {
        printf("Error with write_result_to_db\n");
    }
}
jnx_hashmap* set_configuration(char *path)
{
    jnx_hashmap *hash = jnx_hash_init(1024);
    jnx_file_kvp_node *kvpnode = jnx_file_read_keyvaluepairs(path,"="); 
    while(kvpnode)
    {
        jnx_hash_put(hash,kvpnode->key,kvpnode->value);
        kvpnode = kvpnode->next;
    }
    return hash;
}
int main(int argc, char **argv) 
{
    //Register for signal handling
    signal(SIGINT, catch_int);
    static struct option long_options[] = 
    {
        {"mode",required_argument,0,'m'},
        {"conf",required_argument,0,'c'},
        {0,      0,                 0, 0 }
    };
    int option_index = 0;
    int i;
    int port;
    char *conf = NULL;
    char* mode = NULL;
    while(( i = getopt_long_only(argc,argv,"c:m:",long_options,&option_index)) != -1)
    {
        switch(i)
        {
            case 'c':
                conf = optarg;
                break;
            case 'm':
                mode = optarg;
                break;
            default:
                usage();
                exit(1);
        }

    }
    if(conf == NULL || mode == NULL)
    {
        usage();
        return ARG_UP;
    }
    config = set_configuration(conf);

    if(jnx_log_setup(jnx_hash_get(config,"logpath"),LOGWNEWLINE) != 0)
    {
        printf("WARNING: Could not start logger\n");
        exit(0);
    } 
    jnx_log("Satellite Started");

    jnx_log("Storing SQL credentials temporarily");
    
   printf("%s %s %s %s\n","Saving sql data as",(char*)jnx_hash_get(config,"sqlhost"),(char*)jnx_hash_get(config,"sqluser"),(char*)jnx_hash_get(config,"sqlpass"));
   
   if(store_sql_credentials(jnx_hash_get(config,"sqlhost"),jnx_hash_get(config,"sqluser"),jnx_hash_get(config,"sqlpass")) != 0)
    {
        // could not store creds?
        printf("Error with sql credentials\n");
        jnx_log("Error in store_sql_credentials");
        exit(1);
    }
    jnx_log("Done");
    
    if(strcmp(mode,"RECEIVE") == 0)
    {
        if(!jnx_hash_get(config,"listenport"))
        { printf("Requires port number, option -p\n");return 1; };

        jnx_log("Starting listener");
        jnx_listener_callback c = &server_update;
        jnx_setup_listener(atoi(jnx_hash_get(config,"listenport")),c);
        return 0;
    }
    if(strcmp(mode,"TRANSMIT") == 0)
    {
        jnx_log("Starting daemon");
        while(1)
        {
            //setjmp is the return point after the sql functions have left the main loop
            jnx_log("Connecting to sql");
            if(response_from_db() != 0)
            {
                jnx_log("Error with sql request");
                printf("An error occured with sql request\n");
                printf("Continuing\n");
            }  

            sleep(TIMEWAIT);
        }
        return 0;
    }

    return 0;
}



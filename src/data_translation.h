#ifndef __DATA_TRANSLATION_H___
#define __DATA_TRANSLATION_H___
#define MAX_BUF 1024
#include <jnxc_headers/jnxlist.h>
//This is supposed to be low complexity and few points of failure
//essentially we take a string and break it down into a struct 

struct list* data_from_message(char *msg);

#endif
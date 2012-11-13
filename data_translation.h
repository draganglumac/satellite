#ifndef __DATA_TRANSLATION_H___
#define __DATA_TRANSLATION_H___
#define MAX_BUF 1024
//This is supposed to be low complexity and few points of failure
//essentially we take a string and break it down into a struct 
typedef struct data_parcel
{
	char *prebuild;
	char *build;
	char *postbuild;
};
struct data_parcel* data_from_message(char *msg);
char *data_to_message(struct data_parcel *parcel);
#endif
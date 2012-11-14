#include "data_translation.h"
#include <stdlib.h>
#include <string.h>

#define DELIMITERTOKEN "#"

void from_message(struct list *parcel,char *msg)
{
  char *pch = strtok (msg,DELIMITERTOKEN);
  int counter = 0;
  while (pch != NULL)
  {
    printf ("%s\n",pch);
	switch(counter)
	{
		list_add(parcel,pch);
		break;
	}
    pch = strtok (NULL, DELIMITERTOKEN);
	++counter;
  }
}
char* to_message(struct data_parcel *parcel)
{
	char *msg = malloc(sizeof(char*));
	
	printf("Entered to message\n");
	char *token = (char*)DELIMITERTOKEN;
	printf("Got to to_message\n");

	
	printf("Parcel has been deconstructed\n");
	printf("Prebuild step is: %s\n",parcel->prebuild);
	printf("Build step is: %s\n",parcel->build);
	printf("Postbuild step is: %s\n",parcel->postbuild);

	strcpy(msg,parcel->prebuild);
	strcat(msg,token);
	strcat(msg,parcel->build);
	strcat(msg,token);
	strcat(msg,parcel->postbuild);
	
	printf("String produced : %s\n",msg);
	
	return msg;
}
struct list* data_from_message(char* msg)
{
	struct list *parcel = list_make();
	
	from_message(parcel,msg);

	
	return parcel;
}
char* data_to_message(struct data_parcel* parcel)
{
	return to_message(parcel);
}


#include "data_translation.h"
#include <stdlib.h>
#include <string.h>

#define DELIMITERTOKEN "#"

void from_message(struct list *parcel,char *msg)
{
  char *pch = strtok (msg,DELIMITERTOKEN);
  while (pch != NULL)
  {
    printf ("%s\n",pch);
	list_add(parcel,pch);
    pch = strtok (NULL, DELIMITERTOKEN);

  }
}
struct list* data_from_message(char* msg)
{
	struct list *parcel = list_make();
	
	from_message(parcel,msg);

	return parcel;
}


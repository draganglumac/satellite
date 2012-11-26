#include "executor.h"
#include <stdlib.h>

int execute_data_parcel(char *received_msg)
{
	
	char buffer[strlen(received_msg)]; //safety net from unsafe violations
	strcpy(buffer,received_msg);
	printf("String made safe %s\n",buffer);
	
	char *pch = strtok (buffer,"#");	
 	while (pch != NULL)
 	{
 		printf ("%s\n",pch);
 		pch = strtok (NULL, "#");
 	}

	free(received_msg);
	return 0;
}


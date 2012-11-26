#include "executor.h"
#include <stdlib.h>

int execute_data_parcel(struct list* data_list)
{
	struct node *n = data_list->head;
	while(n)
	{
		printf("-> %s\n",(char*)n->_data);
		if(system(n->_data) != 0) return 1;
		n = n->next_node;

	}
	free(n);
	return 0;
}


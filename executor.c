#include "executor.h"
#include <stdlib.h>

int execute_data_parcel(struct list* data_list)
{
	while(data_list->head)
	{
		printf("-> %s\n",data_list->head->_data);
		if(system(data_list->head->_data) != 0) return 1;
		data_list->head = data_list->head->next_node;
	}
	return 0;
}


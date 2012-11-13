#include "executor.h"
#include <stdlib.h>

int execute_data_parcel(struct data_parcel* parcel)
{
	int prebuild = system(parcel->prebuild);
	int build = system(parcel->build);
	int postbuild = system(parcel->postbuild);
	
	if(prebuild != 0 || build != 0 || postbuild != 0)
	{
		return 1;
	}
	return 0;
}


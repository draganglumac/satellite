#!/bin/bash

echo "Uninstalling satellite"

path=`which satellite`

if [ $? -ne 0 ]; then

	echo "An error occured finding satellite, this may mean that it is not installed on your system"
	exit 1
fi

sudo rm $path

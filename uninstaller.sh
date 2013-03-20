#!/bin/bash

echo "Uninstalling satellite"

path=`which satellite`

if [ $? -ne 0 ]; then

	echo "An error occured finding satellite, this may mean that it is not installed on your system"
	exit 1
fi

OS=`uname`

if [ "$OS" = "Darwin" ]; then
    rm -rf /Users/`whoami`/.satellite
else
    rm -rf /home/`whoami`/.satellite
fi

sudo rm $path

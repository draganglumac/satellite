subdir=`find . -type f -iname *.c -print`
all:
	gcc $(subdir) -o satellite -ljnxc -lmysqlclient -ldl -pthread -g 
install:
	 mv satellite /usr/bin

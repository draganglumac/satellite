all:
	gcc src/*.c -o satellite -ljnxc -lmysqlclient -ldl

install:
	 mv satellite /usr/bin

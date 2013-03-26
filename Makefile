all:
	gcc src/*.c src/network/*.c src/backend/*.c -o satellite -ljnxc -lmysqlclient -ldl -pthread -g

install:
	 mv satellite /usr/bin

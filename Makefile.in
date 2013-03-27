all:
	gcc src/network/*.c src/backend/*.c src/*.c -o satellite -ljnxc -lmysqlclient -ldl -pthread -g

install:
	 mv satellite /usr/bin

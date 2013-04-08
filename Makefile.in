all:
	gcc src/logic/*.c src/network/*.c src/backend/*.c src/*.c -o satellite -ljnxc -lmysqlclient -ldl -pthread -g

install:
	 mv satellite /usr/bin

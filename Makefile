all:
	gcc src/logic/*.c src/network/*.c src/*.c -o satellite -ljnxc -lmysqlclient -ldl -pthread -g -lcrypto -lm 

install:
	 mv satellite /usr/bin

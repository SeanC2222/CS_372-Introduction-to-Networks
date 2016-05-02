default: chatserve client

chatserve: chatserver.cpp inetLib.o
	g++ chatserver.cpp inetLib.o -o chatserve

inetLib.o: inetLib.cpp inetLib.hpp
	g++ inetLib.cpp -c

client: chatclient.c
	gcc chatclient.c -o client

clean:
	rm client chatserve inetLib.o

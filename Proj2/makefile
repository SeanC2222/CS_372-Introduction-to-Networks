default: ftserv

ftserv: ftserver.cpp inetLib.o
	g++ ftserver.cpp inetLib.o -o ftserv

inetLib.o: inetLib.cpp inetLib.hpp
	g++ inetLib.cpp -c

clean:
	rm ftserv *.o *.cpy

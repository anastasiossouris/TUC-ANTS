all: client 

client: client.cpp board comm tucants_all.hpp
	g++ -std=c++11 -Ofast -o client client.cpp board.o comm.o

comm: comm.cpp board tucants_all.hpp
	g++ -std=c++11 -Ofast -c comm.cpp

board: board.cpp tucants_all.hpp
	g++ -std=c++11 -Ofast -c board.cpp

clean:
	rm -f *.o client

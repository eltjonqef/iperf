all:
	g++ -o client client.cpp
	g++ -g -o server server.cpp

clean:
	rm client server
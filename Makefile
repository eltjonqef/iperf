all:
	g++ -pthread -o client client.cpp
	g++ -pthread -o server server.cpp

clean:
	rm client server

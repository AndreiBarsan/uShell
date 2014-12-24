all:
	g++ hello.cc -o bin/hello -std=c++11 -Wall
	g++ main.cc -o bin/main -std=c++11 -Wall -lreadline
	bin/main

clean:
	rm -r bin/* 

all:
	g++ hello.cc -o bin/hello -std=c++11 -Wall
	g++ main.cc -o bin/main -std=c++11 -Wall
	bin/main

clean:
	rm hello
	rm main

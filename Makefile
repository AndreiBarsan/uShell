GPP=g++

OPTS+='-std=c++11'
OPTS+='-Wall'

all:
	mkdir -p bin/
	$(GPP) hello.cc -o bin/hello $(OPTS)
	$(GPP)  main.cc -o bin/ushell $(OPTS) -lreadline

# Makes my life easier
	bin/ushell

clean:
	rm -r bin/* 

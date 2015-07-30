GPP=g++
BIN=bin

OPTS+='-std=c++11'
OPTS+='-Wall'
OPTS+='-g'

UTIL_CC=hello.cc
SHELL_CC=$(filter-out $(UTIL_CC), $(wildcard *.cc))

all: hello shell
# Makes my life easier
	bin/ushell

shell:
	mkdir -p $(BIN)
	$(GPP) $(SHELL_CC) -o $(BIN)/ushell $(OPTS) -lreadline

hello:
	mkdir -p $(BIN)
	$(GPP) $(UTIL_CC) -o $(BIN)/hello $(OPTS)

clean:
	rm -r bin/* 

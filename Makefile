CC ?= cc
CFLAGS ?= -std=c11 -Wall -Wextra -Wpedantic -Werror -g

.PHONY: all clean test

all: vaxiom lexer

vaxiom: MyParser.c lexer.c lexer.h ast.c ast.h
	$(CC) $(CFLAGS) MyParser.c lexer.c ast.c -o $@

lexer: myflex.c lexer.c lexer.h
	$(CC) $(CFLAGS) myflex.c lexer.c -o $@

test: all
	./tests/run_tests.sh

clean:
	$(RM) vaxiom lexer
	$(RM) -r vaxiom.dSYM lexer.dSYM

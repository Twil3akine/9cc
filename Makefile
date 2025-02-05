CFLAGS=-std=c11 -g -static

9cc: 9cc.c

test: 9cc 
				./test

check: 9cc
				@read -p "Enter expression >> " arg; \
				./check "$$arg"

clean:
				rm -f 9cc *.o *~ tmp*

.PHONY: test clean

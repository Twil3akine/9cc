CFLAGS=-std=c11 -g -static
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

9cc: $(OBJS)
				$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(OBJS): 9cc.h

debug: 9cc
				gdb ./9cc

test: 9cc 
				./test

check: 9cc
				@read -p "Enter expression >> " arg; \
				./check "$$arg"

clean:
				rm -f 9cc *.o *~ tmp*

commit:
				git add . && git status && sleep 2 && git commit

push:
				git checkout master && git merge develop && git push origin master develop && git checkout develop

.PHONY: test clean

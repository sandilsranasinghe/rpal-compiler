CHECKFLAGS = --verbose
SRCS = ../tokenizer.c

TESTS = $(patsubst %.c, %, $(wildcard test*.c))
TESTCASES = $(patsubst %.rpal, %, $(wildcard test*.rpal))

build: $(TESTS)

$(TESTS): %: %.c
	gcc -o $@.o $< $(SRCS) -I ..

test: build $(TESTCASES)

$(TESTCASES): %:
	./$(shell echo "$@" | sed 's/[0-9]*$$//').o $@.rpal > $@.out.txt
	python3 -m check $@ $(CHECKFLAGS)

clean:
	rm -f *.o *.out.txt
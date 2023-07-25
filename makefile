rpal: rpal.c tokenizer.c
	gcc -o rpal20 rpal.c tokenizer.c -I .

sample: rpal
	./rpal20 sample.rpal

clean:
	rm -f *.o rpal20
	@$(MAKE) -C test clean

test: rpal
	@$(MAKE) -C test test
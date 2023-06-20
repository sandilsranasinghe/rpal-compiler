rpal:
	gcc rpal.c -o rpal.o

clean:
	rm -f *.o

test: rpal
	./rpal.o sample.rpal
all: ttcdt-huff ttcdt-huff-ar stress

ttcdt-huff.o: ttcdt-huff.c ttcdt-huff.h
	cc -g -Wall $< -c

ttcdt-huff: ttcdt-huff-main.c ttcdt-huff.o
	cc -g -Wall $< ttcdt-huff.o -o $@

ttcdt-huff-ar: ttcdt-huff-ar.c ttcdt-huff.o
	cc -g -Wall $< ttcdt-huff.o -o $@

stress: stress.c ttcdt-huff.o
	cc -g -Wall $< ttcdt-huff.o -o $@

test: stress
	./stress

dist: clean
	rm -f ttcdt-huff.tar.gz && cd .. && tar czvf ttcdt-huff/ttcdt-huff.tar.gz ttcdt-huff/*

clean:
	rm -f ttcdt-huff ttcdt-huff-ar *.o stress *.tar.gz *.asc


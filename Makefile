PREFIX=$(HOME)

default: libperfsmpl.so

install: libperfsmpl.so
	cp perfsmpl.h $(PREFIX)/include && \
	cp libperfsmpl.so $(PREFIX)/lib

libperfsmpl.so: perfsmpl.cpp perfsmpl.h
	g++ -pthread -shared -fPIC -o libperfsmpl.so perfsmpl.cpp

test: test.cpp
	g++ -g -o test test.cpp -L./ -lperfsmpl

clean:
	rm -rf libperfsmpl.so test

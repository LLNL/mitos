PREFIX=$(HOME)

default: libperfsmpl.so

install: libperfsmpl.so
	cp perfsmpl.h $(PREFIX)/include && \
	cp libperfsmpl.so $(PREFIX)/lib

libperfsmpl.so: perfsmpl.cpp perfsmpl.h
	g++ -g -pthread -shared -fPIC -o libperfsmpl.so perfsmpl.cpp \
		-L$(HOME)/lib -lperfsmpl -lsymtabAPI

test: test.cpp
	g++ -g -o test test.cpp -lperfsmpl -L$(HOME)/lib -lperfsmpl -lsymtabAPI

clean:
	rm -rf libperfsmpl.so test

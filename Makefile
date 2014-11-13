CFLAGS=-O2 -Wall -std=c11

DESTDIR?=/
PREFIX?=/usr

all: pipe2udp

pipe2udp: pipe2udp.o

install: all
	install -d -m 0755 ${DESTDIR}${PREFIX}/bin
	install -c -m 0755 pipe2udp ${DESTDIR}${PREFIX}/bin/pipe2udp

clean:
	@rm -f pipe2udp *.o

.PHONY: clean all install

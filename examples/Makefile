CC = gcc
#CFLAGS = -W -Wall -pedantic -s -O3 -mcpu=pentiumpro -march=pentiumpro
#LDFLAGS = -lz -lm -L../libopenspc -s
CFLAGS = -W -Wall -pedantic -g
LDFLAGS = -lz -lm -L../libopenspc
INSTALL_PATH = /usr/local

INSTALL_BIN_PATH = $(INSTALL_PATH)/bin

all: ospcplay spccomm

install: ospcplay
	mkdir -p $(INSTALL_BIN_PATH)
	install -m 755 ospcplay $(INSTALL_BIN_PATH)/ospcplay

ospcplay: ospcplay.o libopenspc.so Makefile
	$(CC) $(LDFLAGS) -o ospcplay ospcplay.o -lopenspc

spccomm: spccomm.o libopenspc.so Makefile
	$(CC) $(LDFLAGS) -o spccomm spccomm.o -lopenspc

libopenspc.so:
	make -C ../libopenspc libopenspc.so

ospcplay.o: ospcplay.c ../libopenspc/openspc.h Makefile
	$(CC) $(CFLAGS) -I../libopenspc -c ospcplay.c -o ospcplay.o

spccomm.o: spccomm.c ../libopenspc/openspc.h Makefile
	$(CC) $(CFLAGS) -I../libopenspc -c spccomm.c -o spccomm.o

clean:
	rm -f ospcplay spccomm *.o

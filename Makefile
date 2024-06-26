# cig

VERSION  = 0.0.1
DEST     = /usr/local/bin
CC       = cc
CPPFLAGS = -D_DEFAULT_SOURCE -D_BSD_SOURCE
CFLAGS   = -Wall -Wextra -pedantic -Os ${CPPFLAGS} -std=c99
LIBS     = -lgit2 -lreadline
SRC      = main.c
OBJ      = ${SRC:.c=.o}

all: cig

.c.o:
	${CC} -c ${CFLAGS} $<

${OBJ}: config.h

config.h:
	cp config.def.h $@

cig: ${OBJ}
	${CC} -o $@ ${OBJ} ${LIBS}

clean:
	rm -f cig ${OBJ}

demo:
	vhs ./demo.tape

install: all
	mkdir -p ${DEST}
	cp -f cig ${DEST}/cig
	chmod 755 ${DEST}/cig

.PHONY: all clean install

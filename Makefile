# cig

VERSION  = 0.0.1
CC       = clang
CPPFLAGS = -D_DEFAULT_SOURCE -D_BSD_SOURCE
CFLAGS   = -Wall -Wextra -pedantic -Os ${CPPFLAGS}
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

.PHONY: all clean

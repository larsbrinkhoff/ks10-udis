CC = gcc
CFLAGS = -W -Wall -O -g

PROG = udis
OBJS = main.o udis.o fields.o

all: $(PROG)

clean:
	rm -f $(PROG) $(OBJS) core

distclean: clean
	rm -f *~

$(PROG): $(OBJS)
	gcc $(OBJS) -o $(PROG)

fields.o: fields.c udis.h
main.o: main.c udis.h
udis.o: udis.c udis.h

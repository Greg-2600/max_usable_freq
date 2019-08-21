#
# Makefile for minimuf
#
PROGRAM= minimuf
COMPILER= gcc
COPTS= -O 
BINDIR= /usr/local/bin
INSTALL= install
#
CFLAGS= $(COPTS)
CC= $(COMPILER)
LIB= ./lib/libm.so
#
SOURCE= shell.c minimuf.c
OBJS= shell.o minimuf.o
EXEC= minimuf

all:	$(PROGRAM)

minimuf:	$(OBJS)
	$(CC) $(COPTS) -o $@ $(OBJS) $(LIB)

install: $(BINDIR)/$(PROGRAM)

$(BINDIR)/$(PROGRAM): $(PROGRAM)
	$(INSTALL) -c -m 0755 $(PROGRAM) $(BINDIR)

tags:
	ctags *.c *.h

depend:
	mkdep $(CFLAGS) $(SOURCE)

clean:
	-@rm -f $(PROGRAM) $(EXEC) $(OBJS)

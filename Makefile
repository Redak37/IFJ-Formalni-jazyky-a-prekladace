SHELL = /bin/bash
CC = gcc
CFLAGS = -std=c99 -Wall -g -Wextra -Werror -pedantic #-O2
LDFLAGS =
SRC = htab
OBJ = obj


.PHONY: FINAL

.PHONY: adresare
.PHONY: clean
.PHONY: zip



ZDROJAKY =	main.c\
			scanner.c\
			scanner_fce.c\
			preprocesor.c\
			parser.c\
			symtable.c\
			ramfile.c\
			vestavene_fce.c\
			matematika.c\
			gassembler.c\
			prvni_pruchod.c\
			zasobniky.c\
			opti2.c\
			optimalizace.c



all: FINAL


FINAL: $(ZDROJAKY)
	$(CC) $(CFLAGS) -DNDEBUG -o KOMP  $^



clean:
	@rm -f KOMP

#@rmdir $(OBJ)

#@rm -f *.o libhtab.a


zip:
	rm -f xducho07.zip
	zip xducho07.zip Makefile rozsireni rozdeleni dokumentace.pdf *.c *.h

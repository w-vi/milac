# Define required macros here
SHELL = /bin/sh

LEX = flex
LFLAGS = 
YACC = bison 
YFLAGS = -d
CC = gcc
CFLAGS = -std=c99 -c -Wall -Werror -O2 -D_POSIX_C_SOURCE=200809L -DNDEBUG
CFLAGSD = -std=c99 -c -Wall -Werror -O0 -D_POSIX_C_SOURCE=200809L -DDEBUG -ggdb 
INC =-I.
LIB = 
SRC =	\
main.c 		\
trace.c  	\
list.c 		\
stack.c 	\
hashtab.c 	\
symtab.c	\
util.c 		\
tree.c 		\
3ac.c 		\
bb.c 		\
optimize.c 	\
lva.c	 	\
const_prop.c	\
copy_prop.c	\
cse.c		\
regalloc.c	\
gen.c		\
code.c		\
mila.tab.c 	\
mila.lex.c

OBJ =$(SRC:.c=.o)
OBJD =$(SRC:.c=.od)
EXE = mila
EXED = mila_d

all: release

release: $(SRC) $(EXE)
	cp $(EXE) ../

debug: ${SRC} ${EXED}
	cp $(EXED) ../

$(EXE): $(OBJ)
	$(CC) $(OBJ) $(LIB) -o $@ 

$(EXED): $(OBJD)
	$(CC) $(OBJD) $(LIB) -o $@

%.o: %.c
	$(CC) $(CFLAGS) ${INC} $< -o $@

%.od: %.c
	$(CC) $(CFLAGSD) ${INC} $< -o $@

mila.tab.c: mila.y
	$(YACC) $(YFLAGS) $< -o $@  

mila.lex.c: mila.l
	$(LEX) $(LFLAGS) -o $@ $< 

.PHONY: clean
clean:
	rm -rf ../${EXE} ../${EXED}
	rm -rf *.od *.o ${EXE} ${EXED}
	rm -rf mila.tab.c mila.lex.c mila.tab.h *.dot *.png


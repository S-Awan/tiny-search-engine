# Makefile for the 'utils' directory of the Tiny Search Engine (TSE)
#
# Author: Insecticide
# Date: 10-19-2025
#

# Compiler flags
CFLAGS = -Wall -pedantic -std=c11 -g

# The library archive file to be created in the ../lib/ directory
LIB = ../lib/libutils.a

# The object files that will be bundled into the library.
OFILES = queue.o hash.o webpage.o

# The default target, which is to build the library.
all: $(LIB)

# Rule to create the library archive from the object files.
$(LIB): $(OFILES)
	ar cr $(LIB) $(OFILES)

# Rules to compile each .c file into a .o object file.
# The '-c' flag tells gcc to compile but not link.
queue.o: queue.c queue.h
	gcc $(CFLAGS) -c queue.c -o queue.o

hash.o: hash.c hash.h queue.h
	gcc $(CFLAGS) -c hash.c -o hash.o

webpage.o: webpage.c webpage.h
	gcc $(CFLAGS) -c webpage.c -o webpage.o

# A 'clean' rule to remove generated files.
clean:
	rm -f $(OFILES) $(LIB)

#
# Makefile for the 'test' directory of the Tiny Search Engine (TSE)
#
# Author: Insecticide
# Date: 10-30-2025
#

# Define compiler and flags
CC = gcc
CFLAGS = -Wall -pedantic -std=c11 -I../utils -L../lib -g

# Define the libraries to link against
LIBS = -lutils -lcurl

# List all test targets
TARGETS = indextest pageiotest

# The default build rule builds all targets
all: $(TARGETS)

# Rule to link the indextest executable
indextest: indextest.c
	$(CC) $(CFLAGS) indextest.c $(LIBS) -o indextest

# Rule to link the pageiotest executable
pageiotest: pageiotest.c
	$(CC) $(CFLAGS) pageiotest.c $(LIBS) -o pageiotest

# A 'clean' rule to remove all compiled programs and test files
clean:
	rm -f $(TARGETS) *.dat

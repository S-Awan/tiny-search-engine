#
# Makefile for the 'indexer' directory of the Tiny Search Engine (TSE)
#
# Author: Insecticide
# Date: 10-30-2025
#

# Define compiler and flags
CC = gcc
# -I../utils tells gcc where to find .h files (queue.h, etc.)
# -L../lib tells the linker where to find the library file (libutils.a)
CFLAGS = -Wall -pedantic -std=c11 -I../utils -L../lib -g

# Define the libraries to link against
# -lutils links libutils.a, -lcurl links the curl library
LIBS = -lutils -lcurl

# The target executable
TARGET = indexer

# The default build rule
all: $(TARGET)

# Rule to link the final executable
# It depends on the indexer.c source file
$(TARGET): indexer.c
	$(CC) $(CFLAGS) indexer.c $(LIBS) -o $(TARGET)

# A 'clean' rule to remove the compiled program
clean:
	rm -f $(TARGET)

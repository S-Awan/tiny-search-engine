#
# Makefile for the 'crawler' directory of the Tiny Search Engine (TSE)
#
# Author: Insecticide
# Date: 10-19-2025
#

# Define compiler and flags
CC = gcc
# -I../utils tells gcc where to find .h files (queue.h, etc.)
# -L../lib tells the linker where to find the library file (libutils.a)
CFLAGS = -Wall -pedantic -std=c11 -I../utils -L../lib -g

# Define the libraries to link against
# -lutils links libutils.a, -lcurl links the curl library for networking
LIBS = -lutils -lcurl

# The default build rule
all: crawler

# Rule to link the final executable
# It depends on the crawler.c source file
crawler: crawler.c
	gcc $(CFLAGS) crawler.c $(LIBS) -o crawler

# A 'clean' rule
clean:
	rm -f crawler

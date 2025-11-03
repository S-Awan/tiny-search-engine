#
# Makefile for the 'querier' directory of the Tiny Search Engine (TSE)
#
# Author: Insecticide (Adapted by Gemini)
# Date: 11-02-2025
#

# Define compiler and flags
CC = gcc
CFLAGS = -Wall -pedantic -std=c11 -I../utils -L../lib -g

# Define the libraries to link against
# -lutils links libutils.a
# -lcurl links the curl library (needed by webpage.o inside libutils.a)
LIBS = -lutils -lcurl

# The target executable
TARGET = query
SRC = query.c

# The default build rule
all: $(TARGET)

# Rule to link the final executable
$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) $(LIBS) -o $(TARGET)

# A 'clean' rule to remove the compiled program
clean:
	rm -f $(TARGET)

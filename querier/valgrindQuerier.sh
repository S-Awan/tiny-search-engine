#!/bin/bash

EXE="./query"
LOGFILE="valgrind_querier.log"
QUERYFILE="valgrind_queries.txt"

# --- IMPORTANT: Set these to your test data ---
PAGEDIR="../pages"
INDEXFILE="../indexer/index.txt"

# --- Check for required files/dirs ---
if [ ! -f "$EXE" ]; then
    echo "FAIL: Executable '$EXE' not found. Please compile with 'make'."
    exit 1
fi
if [ ! -d "$PAGEDIR" ]; then
    echo "FAIL: Page directory '$PAGEDIR' not found."
    exit 1
fi
if [ ! -f "$INDEXFILE" ]; then
    echo "FAIL: Index file '$INDEXFILE' not found."
    exit 1
fi
if [ ! -f "$QUERYFILE" ]; then
    echo "FAIL: Query file '$QUERYFILE' not found."
    exit 1
fi

echo "Running Valgrind with '$QUERYFILE'... check $LOGFILE for details."

# --- Run Valgrind ---
# We pipe the queries into the program and use the -q flag 
valgrind --leak-check=full --show-leak-kinds=all --log-file="$LOGFILE" \
    "$EXE" "$PAGEDIR" "$INDEXFILE" -q < "$QUERYFILE"

# --- Check output ---
if grep -q "ERROR SUMMARY: 0 errors" "$LOGFILE"; then
    echo "PASS: Valgrind found no memory errors or leaks."
    exit 0
else
    echo "FAIL: Valgrind detected memory errors or leaks. See '$LOGFILE' for details."
    grep "ERROR SUMMARY" "$LOGFILE"
    exit 1
fi

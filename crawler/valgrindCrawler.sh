#!/bin/bash

EXE="./crawler"
OUT="valgrind_crawler.log"

if [ ! -f "$EXE" ]; then
    echo "FAIL: Executable '$EXE' not found. Please compile it first with 'make'."
    exit 1
fi

valgrind --leak-check=full --show-leak-kinds=all --log-file="$OUT" "$EXE"

if grep -q "ERROR SUMMARY: 0 errors" "$OUT"; then
    echo "PASS: Valgrind found no memory errors or leaks."
    exit 0
else
    echo "FAIL: Valgrind detected memory errors or leaks. See '$OUT' for details."
    grep "ERROR SUMMARY" "$OUT"
    exit 1
fi

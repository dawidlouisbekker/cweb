#!/bin/bash

# Compile the program
/usr/lib/llvm-18/bin/clang -target wasm32 -nostdlib -Wl,--no-entry -Wl,--export-all  -o wasm32.wa
sm wasm32.c

# Check if compilation was successful
if [ $? -eq 0 ]; then
    echo "Compilation successful! Executable created: cweb"
else
    echo "Compilation failed. Please check for errors."
fi
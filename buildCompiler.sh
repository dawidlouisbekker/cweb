#!/bin/bash

# Compile the program
gcc ../compiler/include/main.c -o ../compiler/compiler ../compiler/include/heapscan.o

# Check if compilation was successful
if [ $? -eq 0 ]; then
    echo "Compilation successful! Executable created: compiler"
else
    echo "Compilation failed. Please check for errors."
fi
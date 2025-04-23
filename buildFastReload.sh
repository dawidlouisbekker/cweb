#!/bin/bash

# Compile the program
gcc fastreload/include/main.c -o fastreload/rload

# Check if compilation was successful
if [ $? -eq 0 ]; then
    echo "Compilation successful! Executable created: fastreload"
else
    echo "Compilation failed. Please check for errors."
fi
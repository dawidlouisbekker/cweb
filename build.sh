#!/bin/bash

# Compile the program
gcc main.c compiler/main.c -o cweb

# Check if compilation was successful
if [ $? -eq 0 ]; then
    echo "Compilation successful! Executable created: cweb"
else
    echo "Compilation failed. Please check for errors."
fi
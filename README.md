# This is programming language and web framework designed explicitly for linux #

### /build ###
Where the cweb code is written
return (

)
is where html must reside. 

To run you must do.
./webc

To build you must do 
./build.sh

WASM file structure 

+----------------+---------------------------------------+
| Magic Number   | 0x00 61 73 6D (ASCII: "\0asm")       |
+----------------+---------------------------------------+
| Version        | 0x01                                  |
+----------------+---------------------------------------+
| Type Section   | Contains function signatures (types) |
+----------------+---------------------------------------+
| Import Section | Imports functions, tables, memory    |
+----------------+---------------------------------------+
| Function Section | List of function references         |
+----------------+---------------------------------------+
| Code Section   | Contains function bytecode           |
+----------------+---------------------------------------+
| Data Section   | Initialized data for the module      |
+----------------+---------------------------------------+

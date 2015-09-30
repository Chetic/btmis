#!/bin/bash -e
# Assumes libcanusb is in a directory parallel to btmis
# Compiles and retrieves external dependencies
make clean
mkdir -p include
mkdir -p lib
cd ../libcanusb
make clean && make
cp src/*.h ../btmis/include/
cp lib/* ../btmis/lib
cd ../btmis
make


#!/bin/bash -x

rm -f dm_double.o
rm -f dm_muldiv.o
rm -f DoubleTests.exe

if [ "$1" == "clean" ]; then
   exit
fi

gcc -c -Wall -Wextra -Wpedantic -s -O3 dm_double.c dm_muldiv.c
g++ -o DoubleTests -Wall -Wextra -Wpedantic -s -O3 -I../External/googletest/include DoubleTests.cpp dm_double.o dm_muldiv.o ../External/googletest/lib/libgtest.a ../External/googletest/lib/libgtest_main.a
./DoubleTests.exe

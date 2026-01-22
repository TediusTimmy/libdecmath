#!/bin/bash -x

rm -f dm_float.o
rm -f dm_double.o
rm -f dm_muldiv.o

rm -f dm_float.gcda
rm -f dm_float.gcno
rm -f dm_double.gcda
rm -f dm_double.gcno
rm -f dm_muldiv.gcda
rm -f dm_muldiv.gcno

rm -f Base.info
rm -f Run.info
rm -f Full.info

rm -f FloatTests-FloatTests.gcda
rm -f FloatTests-FloatTests.gcno

rm -f FloatTests.exe

rm -rf cov

if [ "$1" == "clean" ]; then
   exit
fi

gcc -c -Wall -Wextra -Wpedantic -g --coverage -O0 dm_float.c dm_double.c dm_muldiv.c
g++ -o FloatTests -Wall -Wextra -Wpedantic -g --coverage -O0 -I../External/googletest/include FloatTests.cpp dm_float.o dm_double.o dm_muldiv.o ../External/googletest/lib/libgtest.a ../External/googletest/lib/libgtest_main.a


if [ "$1" == "nocov" ]; then
   exit
fi

../External/lcov/bin/lcov --rc lcov_branch_coverage=1 --no-external --capture --initial --directory . --output-file Base.info
./FloatTests.exe
../External/lcov/bin/lcov --rc lcov_branch_coverage=1 --no-external --capture --directory . --output-file Run.info
../External/lcov/bin/lcov --rc lcov_branch_coverage=1 --add-tracefile Base.info --add-tracefile Run.info --output-file Full.info
mkdir cov
../External/lcov/bin/genhtml --rc lcov_branch_coverage=1 --branch-coverage --output-directory cov Full.info

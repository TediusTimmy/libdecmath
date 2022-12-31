#!/bin/bash -x

rm -f dm_double.o
rm -f dm_muldiv.o
rm -f DoubleTests.exe

if [ "$1" == "clean" ]; then
   exit
fi

x86_64-w64-mingw32-gcc.exe -c -Wall -Wextra -Wpedantic -g -O0 dm_double.c dm_muldiv.c
x86_64-w64-mingw32-g++.exe -o DoubleTests -Wall -Wextra -Wpedantic -g -O0 -I../External/googletest/include DoubleTests.cpp dm_double.o dm_muldiv.o ../External/googletest/lib-w64/libgtest.a ../External/googletest/lib-w64/libgtest_main.a

./DoubleTests.exe

x86_64-w64-mingw32-gcc.exe -c -Wall -Wextra -Wpedantic -s -O3 dm_double.c dm_muldiv.c
x86_64-w64-mingw32-g++.exe -o DoubleTests -Wall -Wextra -Wpedantic -s -O3 -I../External/googletest/include DoubleTests.cpp dm_double.o dm_muldiv.o ../External/googletest/lib-w64/libgtest.a ../External/googletest/lib-w64/libgtest_main.a

./DoubleTests.exe

x86_64-w64-mingw32-gcc.exe -c -Wall -Wextra -Wpedantic -g -O0 -DDM_NO_128_BIT_TYPE dm_double.c dm_muldiv.c
x86_64-w64-mingw32-g++.exe -o DoubleTests -Wall -Wextra -Wpedantic -g -O0 -I../External/googletest/include DoubleTests.cpp dm_double.o dm_muldiv.o ../External/googletest/lib-w64/libgtest.a ../External/googletest/lib-w64/libgtest_main.a

./DoubleTests.exe

x86_64-w64-mingw32-gcc.exe -c -Wall -Wextra -Wpedantic -s -O3 -DDM_NO_128_BIT_TYPE dm_double.c dm_muldiv.c
x86_64-w64-mingw32-g++.exe -o DoubleTests -Wall -Wextra -Wpedantic -s -O3 -I../External/googletest/include DoubleTests.cpp dm_double.o dm_muldiv.o ../External/googletest/lib-w64/libgtest.a ../External/googletest/lib-w64/libgtest_main.a

./DoubleTests.exe

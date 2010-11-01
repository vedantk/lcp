cd bin
g++ -O3 -Wextra -c ../src/*.cpp
del main.o
g++ -O3 -Wextra ../src/main.cpp *.o -o ../lcp


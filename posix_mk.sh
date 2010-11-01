flags='-Wextra -O3'

mkdir -p bin/
cd bin/
g++ $flags -c ../src/*.cpp
rm main.o
g++ $flags ../src/main.cpp *.o -o ../lcp


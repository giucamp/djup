

	# clang-8

echo clang-8
export CC=/usr/bin/clang-8
export CXX=/usr/bin/clang++-8

mkdir build/clang8 || false
pushd build/clang8
rm sources/test/test
rm test
cmake -S../../
make

cp sources/test/test test

chmod +x ./test
./test
popd


	# gcc-9

echo gcc-9
export CC=/usr/bin/gcc-9
export CXX=/usr/bin/g++-9
export CXX_FLAGS=-stdlib=libc++

mkdir build/gcc9 || false
pushd build/gcc9
rm sources/test/test
rm test
cmake -S../../
make

cp sources/test/test test

chmod +x ./test
./test
popd





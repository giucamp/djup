

	# clang-8

echo clang-8
export CC=/usr/bin/clang-8
export CXX=/usr/bin/clang++-8

mkdir build/clang8 || false
pushd build/clang8
rm -f sources/test/djup_test
rm -f djup_test
cmake -S ../../
make

cp sources/test/djup_test djup_test

chmod +x ./djup_test
./djup_test
popd


	# gcc-9

echo gcc-9
export CC=/usr/bin/gcc-9
export CXX=/usr/bin/g++-9
export CXX_FLAGS=-stdlib=libc++

mkdir build/gcc9 || false
pushd build/gcc9
rm -f sources/test/djup_test
rm -f djup_test
cmake -S ../../
make

cp sources/test/djup_test djup_test

chmod +x ./djup_test
./djup_test
popd





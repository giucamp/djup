

	# clang-8

echo clang-8
export CC=/usr/bin/clang-8
export CXX=/usr/bin/clang++-8

#clan
if [[ "$*" == *"--clean"* ]]; then
	echo "cleaning clang-8 build"
	pushd build/
	rm -rf ./clang8
	popd
fi

#build
mkdir -p build/clang8
pushd build/clang8
rm -f sources/test/djup_test
rm -f djup_test
cmake -S ../../
make

#copy exe and execute it
cp sources/test/djup_test djup_test
chmod +x ./djup_test
./djup_test
popd


	# gcc-9

echo gcc-9
export CC=/usr/bin/gcc-9
export CXX=/usr/bin/g++-9
export CXX_FLAGS=-stdlib=libc++

#clan
if [[ "$*" == *"--clean"* ]]; then
	echo "cleaning gcc-9 build"
	pushd build
	rm -rf ./gcc9
	popd
fi

#build
mkdir -p build/gcc9
pushd build/gcc9
rm -f sources/test/djup_test
rm -f djup_test
cmake -S ../../
make

#copy exe and execute it
cp sources/test/djup_test djup_test
chmod +x ./djup_test
./djup_test
popd





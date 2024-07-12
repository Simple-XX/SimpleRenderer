mkdir build;
cmake -S . -B build -DCMAKE_BUILD_TYPE=Coverage;
cd build;
make;
cd ..;
./build/test/system_test/system_test;

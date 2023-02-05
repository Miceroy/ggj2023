#!/bin/sh
mkdir release
cd release

# Linux
echo 
echo Building linux binaries...
mkdir linux
cd linux
cmake -DCMAKE_RUNTIME_OUTPUT_DIRECTORY=linux ../../
cmake --build . -- -j24
cp -R assets linux/assets
rm -R assets
zip -r9 ../build_linux.zip linux
cd ..
echo
echo Building linux binaries DONE
echo

# Windows
echo 
echo Building windows binaries...
mkdir windows
cd windows
cmake -DCMAKE_RUNTIME_OUTPUT_DIRECTORY=windows -DCMAKE_TOOLCHAIN_FILE=../../cmake/mingw-w64-x86_64.cmake ../../ 
cmake --build . -- -j24
cp -R assets windows/assets
rm -R assets
zip -r9 ../build_windows.zip windows
cd ..
echo
echo Building windows binaries DONE
echo


#!/bin/sh
set -e
D=$(cd "$(dirname "$0")" && pwd)
SDK=D:/Fallo/herramientas/ps5-payload-sdk
LLVM=/d/Ghidra-Reload/herramientas/llvm-mingw-20261006-ucrt-x86_64/bin
export PATH="/d/Fallo/herramientas/ps5-payload-sdk/win:$LLVM:$PATH"
export TMP=D:/Fallo/tmp TEMP=D:/Fallo/tmp
mkdir -p "$D/salida"

$LLVM/clang.exe -target x86_64-sie-ps5 -isysroot $SDK -isystem $SDK/target/include -L $SDK/target/lib \
  -fno-stack-protector -fno-plt -femulated-tls -std=gnu99 -O2 -Wall -Wno-unused-parameter \
  "$D/src/main.c" -o "$D/salida/pes2021_fc.elf" -lc -lkernel_web -lSceLibcInternal

ls -la "$D/salida/pes2021_fc.elf"

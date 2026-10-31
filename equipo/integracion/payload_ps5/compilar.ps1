param([string]$Sdk='D:/Fallo/herramientas/ps5-payload-sdk',
 [string]$LLVM='D:/Ghidra-Reload/herramientas/llvm-mingw-20261006-ucrt-x86_64/bin',
 [ValidateSet(0,1)][int]$DefaultPes=0)
$ErrorActionPreference='Stop'
$env:PATH="$Sdk/win;$LLVM;$env:PATH"
$env:TMP=$PSScriptRoot
$env:TEMP=$PSScriptRoot
& "$LLVM/clang.exe" -target x86_64-sie-ps5 -isysroot $Sdk -isystem "$Sdk/target/include" -L "$Sdk/target/lib" -fno-stack-protector -fno-plt -femulated-tls -std=gnu99 -O2 -Wall -Wextra -Werror "-DDEFAULT_PES=$DefaultPes" "$PSScriptRoot/main.c" -o "$PSScriptRoot/pes2021_fc.elf" -lc -lkernel_web -lSceLibcInternal
if($LASTEXITCODE -ne 0) { throw 'Compilacion fallida' }
& "$LLVM/llvm-readelf.exe" -h "$PSScriptRoot/pes2021_fc.elf"
if($LASTEXITCODE -ne 0) { throw 'ELF invalido' }

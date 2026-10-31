param([string]$Sdk='D:/Fallo/herramientas/ps5-payload-sdk',
 [string]$LLVM='D:/Ghidra-Reload/herramientas/llvm-mingw-20261006-ucrt-x86_64/bin',
 [ValidateSet('main.c','src/main.c')][string]$Source='src/main.c',
 [string]$OutputDir=(Join-Path $PSScriptRoot ('build-pes-default-' + [guid]::NewGuid().ToString('N'))))
$ErrorActionPreference='Stop'
$env:PATH="$Sdk/win;$LLVM;$env:PATH"
$env:TMP=$PSScriptRoot
$env:TEMP=$PSScriptRoot
if (Test-Path -LiteralPath $OutputDir) { throw 'La salida debe ser una carpeta nueva; no se sobrescriben binarios.' }
New-Item -ItemType Directory -Path $OutputDir | Out-Null
$OutputElf=Join-Path $OutputDir 'pes2021_fc.elf'
& "$LLVM/clang.exe" -target x86_64-sie-ps5 -isysroot $Sdk -isystem "$Sdk/target/include" -L "$Sdk/target/lib" -fno-stack-protector -fno-plt -femulated-tls -std=gnu99 -O2 -Wall -Wextra -Werror "$PSScriptRoot/$Source" -o $OutputElf -lc -lkernel_web -lSceLibcInternal
if($LASTEXITCODE -ne 0) { throw 'Compilacion fallida' }
& "$LLVM/llvm-readelf.exe" -h $OutputElf
if($LASTEXITCODE -ne 0) { throw 'ELF invalido' }

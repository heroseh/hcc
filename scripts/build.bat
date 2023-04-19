@echo off

SET FLAGS=-pedantic -std=c11 -Werror -Wfloat-conversion -Wextra -g -Wl,-nodefaultlib:libcmt -ldbghelp -lonecore -lsynchronization -lmsvcrt -luser32
IF "%~1" == "release" (
  SET FLAGS=%FLAGS% -O2
)

IF NOT EXIST "build" mkdir "build"

cd build
IF NOT EXIST "libc" mklink /d libc ..\libc
IF NOT EXIST "libhmaths" mklink /d libhmaths ..\libhmaths
IF NOT EXIST "libhccintrinsics" mklink /d libhccintrinsics ..\libhccintrinsics
cd ..

clang %FLAGS% -Wimplicit-fallthrough -o build\hcc.exe src\hcc_main.c && ^
build\hcc.exe -O -fi samples/shaders.c -fo samples/shaders.spirv -fomc samples/shaders-metadata.h && ^
clang %FLAGS% -Ilibhmaths -Ilibhccintrinsics -Iinterop -I%VULKAN_SDK%/Include/ -L%VULKAN_SDK%/Lib/ -lvulkan-1 -o samples\samples.exe samples\app\main.c && ^
clang %FLAGS% -Ilibhmaths -Ilibhccintrinsics -Iinterop -I%VULKAN_SDK%/Include/ -L%VULKAN_SDK%/Lib/ -lvulkan-1 -o playground\playground.exe playground\app\main.c

SET EXIT_CODE=%ERRORLEVEL%
IF %EXIT_CODE% NEQ 0 (
  exit /b %EXIT_CODE%
)

IF "%~1" == "release" (
  echo "=========== Building Release Package ==========="
  cd build
  powershell Compress-Archive -Force -DestinationPath hcc-0.0.2-windows.zip -LiteralPath hcc.exe, ..\libc, ..\libhmaths, ..\libhccintrinsics, ..\interop, ..\samples, ..\playground, ..\docs, ..\README.md, ..\LICENSE
  cd ..
)

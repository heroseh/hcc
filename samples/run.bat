@echo off

..\hcc.exe -O -fi shaders.c -fo shaders.spirv -fomc shaders-metadata.h && ^
clang -pedantic -std=c11 -Werror -Wfloat-conversion -Wextra -g -Wl,-nodefaultlib:libcmt -ldbghelp -lonecore -lsynchronization -lmsvcrt -luser32 -Ilibhmaths -Ilibhccintrinsics -Iinterop -I%VULKAN_SDK%/Include/ -L%VULKAN_SDK%/Lib/ -lvulkan-1 -o samples.exe main.c

SET EXIT_CODE=%ERRORLEVEL%
IF %EXIT_CODE% NEQ 0 (
  exit /b %EXIT_CODE%
)

samples.exe


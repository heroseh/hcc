@Echo off

build\hcc.exe -O -I shader-include -I libc-gpu -fi samples/shaders.c -fi shader-include/hmaths/maths.c -fo samples/shaders.spirv -fomc samples/shaders-metadata.h

IF %ERRORLEVEL% NEQ 0 (Exit /b 1)

clang -pedantic -std=c11 -I./shader-include -I./interop -Werror -Wfloat-conversion -Wextra -Wl,-nodefaultlib:libcmt -ldbghelp -lonecore -lsynchronization -lmsvcrt -luser32 -g -I%VULKAN_SDK%/Include/ -L%VULKAN_SDK%/Lib/ -lvulkan-1 -g -o build/samples.exe samples/app/main.c

IF %ERRORLEVEL% NEQ 0 (Exit /b 1)

build\samples.exe

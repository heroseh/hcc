
clang -pedantic -std=c11 -Werror -Wfloat-conversion -Wextra -g -Wl,-nodefaultlib:libcmt -ldbghelp -lonecore -lsynchronization -lmsvcrt -luser32 -Ilibhmaths -Ilibhccintrinsics -Iinterop -I%VULKAN_SDK%/Include/ -L%VULKAN_SDK%/Lib/ -lvulkan-1 -o playground.exe main.c

SET EXIT_CODE=%ERRORLEVEL%
IF %EXIT_CODE% NEQ 0 (
  exit /b %EXIT_CODE%
)

playground.exe

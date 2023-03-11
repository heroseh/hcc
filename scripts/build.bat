if not exist build mkdir build
clang -pedantic -std=c11 -Werror -Wfloat-conversion -Wimplicit-fallthrough -Wextra -g -o build/hcc src/hcc_main.c

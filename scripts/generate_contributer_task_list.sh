#!/bin/sh
clang -pedantic -D_GNU_SOURCE -DHCC_ENABLE_CONTRIBUTER_TASK_LOG -lm -std=gnu11 -Werror -Wfloat-conversion -Wextra -g -o /dev/null src/hcc_main.c > contributer_task_list.txt

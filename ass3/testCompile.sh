#!/bin/bash

set -e
set -x
for a in *.c; do
    gcc -std=gnu99 -Wall -pedantic -c $a
done
rm *.o

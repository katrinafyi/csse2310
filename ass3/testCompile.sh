#!/bin/bash

set -e
set -x
for a in *.c *.h; do
    gcc -std=gnu99 -Wall -pedantic -c $a
done
rm *.gch

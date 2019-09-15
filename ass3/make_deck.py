#!/usr/bin/env python3
import random
import string
import sys

if __name__ == '__main__':
    if len(sys.argv) < 2:
        print('specify number of cards as argument', file=sys.stderr)
        sys.exit(1)

    suits = 'ADCH'
    ranks = string.hexdigits.lower()

    n = int(sys.argv[1])

    print(n)
    for i in range(n):
        print(random.choice(suits), random.choice(ranks), sep='')


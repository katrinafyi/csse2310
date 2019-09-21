#!/usr/bin/env python3
import random
import string
import sys
from itertools import product

if __name__ == '__main__':
    if len(sys.argv) < 2:
        print('specify number of cards as argument', file=sys.stderr)
        sys.exit(1)

    suits = 'SDCH'
    ranks = string.hexdigits.lower().replace('0', '');

    cards = list(product(suits, ranks))

    n = int(sys.argv[1])
    if n > len(cards):
        raise ValueError(f'{n} exceeds the maximum number of cards, {len(cards)}')
    print(n)
    random.shuffle(cards)
    for i in range(n):
        s, r = cards[i]
        print(s, r, sep='')


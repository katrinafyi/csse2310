#!/usr/bin/env python3

import sys

if __name__ == '__main__':
    for header in sys.argv[1:]:
        print(header + '.h')
        with open(header + '.h', 'x') as f:
            guard = header.upper() + '_H'
            f.write(f'#ifndef {guard}\n')
            f.write(f'#define {guard}\n')
            f.write('\n\n\n')
            f.write('#endif\n')

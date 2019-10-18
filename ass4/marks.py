#!/usr/bin/env python2

# WARNING: grum.py is python 2

from __future__ import print_function, generators, with_statement, generators
import fnmatch
import random
import os
import sys

from collections import defaultdict
from pipes import quote

class TestProcess(object):
    def __init__(self, i, args, stdin):
        self.w = lambda x: x+''
        self.lines_read = 0
        self.i = i
        self.args = args
        self.stdin = stdin
        self.pid_var = 'pid' + str(i)

        self.stdin_file = 'stdin' + str(i)
        self.stdin_fd = str(i) + '0'

        self.stdout_file = 'stdout' + str(i)
        self.stdout_fd = str(i) + '1'

        self.stderr_file = 'stderr' + str(i)
        self.stderr_fd = str(i) + '2'

        self.line_var = 'line' + str(i)

        cmd = [quote(x) if not x.startswith('$') else x for x in args]
        if stdin:
            cmd += ['<', quote(stdin)]
        mktemp = lambda f, fd: print(f+'=$TEMP_DIR/'+f+'; :>$'+f)#+'; exec '+self.w(fd)+'<>$'+f)
        mktemp(self.stdin_file, self.stdin_fd)
        mktemp(self.stdout_file, self.stdout_fd)
        mktemp(self.stderr_file, self.stderr_fd)
        if not stdin:
            pass
        else:
            print('cat', quote(stdin), '>$'+self.stdin_file)
        cmd += ['<$'+self.stdin_file, '1>$'+self.stdout_file, '2>$'+self.stderr_file, '&']
        print(*cmd)
        print(self.pid_var + '=$!')

    def send(self, msg):
        print('printf', '"%s"', quote(msg), '>$'+self.stdin_file)

    def finish_input(self):
        print('wait', '$'+self.pid_var)

    def readline_stdout(self):
        print('read -r '+self.line_var+' <$' + self.stdout_file)
        print('echo READ:', '$'+self.line_var)
        return '$'+self.line_var

    def send_signal(self, sig):
        print('kill -'+ str(sig), '$'+self.pid_var)

    def diff_fd(self, fd, comparison):
        if fd == 1:
            f = self.stdout_file
        elif fd == 2:
            f = self.stderr_file
        fd = str(self.i) + str(fd)
        #redir = '<$'+self.stderr_file
        print('cat', '<$'+f)
        print('colordiff', quote(comparison), '-', '<$'+f,
                '|| { echo diff mismatch on fd '+str(fd)+' >&2; exit 1; }')

    def expect_exit(self, code):
        self.finish_input()
        print('[[ $? != '+str(code)+' ]]', '&&', '{ exit 2; }')

    def assert_signalled(self, sig):
        self.expect_exit(sig + 128)

    def kill(self):
        print('kill', '-9', '$'+self.pid_var)


class TestCase(object):
    def __init__(self):
        print('#!/bin/bash')
        print('set', '-v')
        print('TEMP_DIR=$(mktemp -d --tmpdir=. tmp_test.XXXX)')
        self._i = 0
        self.prog = './2310depot' # TODO: generalise

    def process(self, args, stdin=None):
        self._i += 1
        return TestProcess(self._i, args, stdin)
    
    def delay(self, t):
        print('sleep', (t))

    def assert_stdout_matches_file(self, proc, f):
        proc.diff_fd(1, f)
    def assert_stderr_matches_file(self, proc, f):
        proc.diff_fd(2, f)

    def assert_exit_status(self, proc, status):
        proc.expect_exit(status)


def marks(category=None, category_marks=0):
    def wrapper(f):
        f.marks = {'category': category,
            'marks': category_marks}
        return f;
    return wrapper


def eprint(*args, **kwargs):
    kwargs['file'] = sys.stderr
    print(*args, **kwargs)

def main():
    eprint('starting shim marks main')
    tests = {}
    for cls in TestCase.__subclasses__():
        eprint('test case:', cls.__name__)
        for fn in dir(cls):
            fn = getattr(cls, fn)
            if not hasattr(fn, 'marks'): continue
            name = cls.__name__+'.'+fn.__name__
            tests[name] = (cls, fn)
            eprint('  '+name)
    
    cls, fn = None, None
    while True:
        eprint('select test case: ', end='')
        line = raw_input()
        matched = fnmatch.filter(tests, '*'+line+'*')
        eprint('  ', end='')
        eprint(*matched, sep='\n  ')
        if len(matched) == 1:
            cls, fn = tests[matched[0]]
            break
    eprint(cls, fn)

    if len(sys.argv) < 2:
        SH_FILE = 'test.sh'
        sys.stdout = open(SH_FILE, 'w')
        os.chmod(SH_FILE, 0755)
    
    cls.__marks_options__ = defaultdict(lambda: '.')
    # causes problems
    #cls.setup_class()
    c = cls()
    getattr(c, fn.__name__)()
    

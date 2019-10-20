#!/usr/bin/env python2

"""
Shim marks.py for executing grum.py tests independent of moss.

Features:
 - Wildcard matching of test names (using Bash's *, ?, []).
 - More verbose output (up to -vvv), including full diffs.
 - Delay time multiplier to adjust delays by a factor between 0 and 1.
 - Display of all failure conditions for a test, not just first failure.

"""

# WARNING: grum.py is python 2

from __future__ import print_function, generators, with_statement, generators
import fnmatch
import random
import os
import sys
import time
import logging
import signal

from pprint import pprint
import warnings
from warnings import warn
import subprocess
import difflib
from collections import defaultdict
from pipes import quote
import argparse

logger = logging.getLogger(__name__)
logger.addHandler(logging.NullHandler())

delay = 1 # delay time coefficient

class Colour:
    HEADER = '\033[95m'
    OKBLUE = '\033[94m'
    OKGREEN = '\033[92m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'
    REVERSE = '\033[7m'
    GREY = '\033[38;5;8m'
    CYAN = '\033[38;5;14m'
    MAGENTA = '\033[38;5;13m'

class TestProcess(object):
    def __init__(self, i, args, stdin):
        logger.debug('starting process {} with stdin {}'
                .format(i, repr(stdin)))
        logger.debug('    cmd: {}'.format(args))
        if stdin:
            stdin = open(stdin, 'r')
        else:
            stdin = subprocess.PIPE
        self.i = i
        self.args = args
        self.process = subprocess.Popen(args, stdout=subprocess.PIPE,
            stdin=stdin, stderr=subprocess.PIPE)

    def send(self, msg):
        logger.debug('writing to stdin of process {}: {}'.format(self.i, repr(msg)))
        self.process.stdin.write(msg)
        self.process.stdin.flush()

    def finish_input(self):
        logger.debug('closing stdin of process {}'.format(self.i))
        self.process.stdin.close()

    def readline_stdout(self):
        line = self.process.stdout.readline().rstrip()
        logger.debug('got line {} from process {}'.format(repr(line), self.i))
        #print('read', line)
        return line

    def send_signal(self, sig):
        logger.debug('sending signal {} to process {}'.format(sig, self.i))
        self.process.send_signal(sig)

    def diff_fd(self, fd, comparison):
        if fd == 1:
            f = self.process.stdout
        elif fd == 2:
            f = self.process.stderr
        fname = ['stdin', 'stdout', 'stderr'][fd]
        out = f.read().splitlines(1)
        with open(comparison, 'r') as compare:
            #d = difflib.Differ()
            #diff = list(d.compare(compare.read().splitlines(1), out))
            expected = compare.read().splitlines(1)
            diff = difflib.unified_diff(expected, out, comparison, '(actual)')
            diff = list(diff)
            if diff:
                logger.error('{} mismatch on process {} {}'.format(fname, self.i,
                    self.args))
                logger.warning('\n'+''.join(diff).rstrip())
            else:
                logger.info('{} matched on process {} to file {}'
                        .format(fname, self.i, repr(comparison)))

    signal_names = {getattr(signal, s): s for s in dir(signal)
            if s.startswith('SIG') and not s.startswith('SIG_')}

    @classmethod
    def _sig_name(cls, code):
        pcode = code if code >= 0 else -code
        s = 'signal ' if code < 0 else 'exit code '
        s += str(pcode)
        if code < 0 and pcode in cls.signal_names:
            s += ' ('+cls.signal_names[pcode]+')'
        return s


    def expect_exit(self, code):
        ret = self.process.wait()
        if ret != code:
            logger.error('process {} expected {} but got {}'
                    .format(self.i, self._sig_name(code), self._sig_name(ret)))
        else:
            logger.info('process {} exited correctly with {}'
                    .format(self.i, self._sig_name(code)))

    def assert_signalled(self, sig):
        self.expect_exit(-sig)

    def kill(self):
        logger.debug('killing process {}'.format(self.i))
        self.process.kill()


class TestCase(object):
    def __init__(self):
        self._i = -1

    def process(self, args, stdin=None):
        self._i += 1
        return TestProcess(self._i, args, stdin)
    
    def delay(self, t):
        logger.debug('sleeping for {} seconds'.format(t))
        time.sleep(t*delay)

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

def _catch_warnings(display):
    caught = []
    original = warnings.showwarning

    def showwarning(*args, **kwargs):
        caught.append(args[0])
        if display:
            return original(*args, **kwargs)
    warnings.showwarning = showwarning
    return caught

class CaptureHandler(logging.NullHandler):
    def __init__(self):
        super(logging.NullHandler, self).__init__()
        self.setFormatter(logging.Formatter('%(levelname)s %(funcName)s %(message)s'))
        self.records = []

    def handle(self, record):
        self.records.append(record)

def nonneg_float(s):
    x = float(s)
    if x < 0:
        raise argparse.ArgumentTypeError('value cannot be negative: ' + s)
    return x

def parse_args(args):
    parser = argparse.ArgumentParser(description='Test runner for CSSE2310. '
            +'Shim marks.py by Kenton Lam.')
    parser.add_argument('-v', '--verbose', action='count', default=0,
            help='verbosity, can be repeated up to 3 times. once prints diffs, twice prints successes, thrice prints all actions.')
    parser.add_argument('-d', '--delay', action='store', type=nonneg_float, default=1,
            help='delay time multiplier.')
    parser.add_argument('test', type=str, nargs='*', default=('*',),
            help='tests to run. can contain Bash-style wildcards. default: all.')
    return parser.parse_args(args)

def main():
    global delay
    tests = {}
    test_names = []
    for cls in TestCase.__subclasses__():
        for fn in dir(cls):
            fn = getattr(cls, fn)
            if not hasattr(fn, 'marks'): continue
            name = cls.__name__+'.'+fn.__name__
            tests[name] = (cls, fn)
            test_names.append(name)
    
    args = (parse_args(sys.argv[1:]))
    delay = args.delay
    verbose = min(args.verbose, 3)

    failure_level = logging.ERROR
    diff_level = logging.WARNING
    success_level = logging.INFO
    display_level = [logging.ERROR, logging.WARNING, logging.INFO,
            logging.DEBUG][verbose]

    level_names = {
        failure_level: Colour.WARNING+'failure',
        success_level: Colour.OKBLUE+'success',
        diff_level: 'diff',
        logging.CRITICAL: Colour.WARNING+Colour.REVERSE+'error'
    }

    handler = CaptureHandler()
    logger.setLevel(logging.DEBUG)
    logger.addHandler(handler)

    matched = []
    seen = set()
    for t in args.test:
        new_matches = [x for x in fnmatch.filter(test_names, '*'+t+'*')
                if x not in seen]
        matched.extend(new_matches)
        seen.update(new_matches)

    print('executing', str(len(matched)), 'tests:', matched)
    print()

    max_len = max(len(x) for x in matched) if matched else 0
    num_len = len(str(len(matched)))

    passed = 0
    failed = 0
    start = time.time()
    for name in matched:
        test_num = '[{}/{}]'.format(1+passed+failed, len(matched))
        test_num = test_num.rjust(2*num_len+3)
        print(Colour.MAGENTA+test_num+Colour.ENDC,
                name.ljust(max_len), '...', end=' ')
        del handler.records[:]

        cls, fn = tests[name]
        cls.__marks_options__ = {'working_dir': ''}
        try:
            cls.setup_class() # needed to set cls.prog
        except (NameError, OSError):
            pass
        c = cls()

        try:
            fn(c)
        except Exception as e:
            logger.critical(e.__class__.__name__+': '+str(e))

        if any(r for r in handler.records if r.levelno >= failure_level):
            failed += 1
            print(Colour.FAIL+'FAIL'+Colour.ENDC)
        else:
            passed += 1
            print(Colour.OKGREEN+'OK'+Colour.ENDC)
        for i, r in enumerate(handler.records):
            if r.levelno < display_level: continue
            name = r.levelname.lower()
            if r.levelno in level_names: name = level_names[r.levelno]
            print(name+':'+Colour.ENDC, r.msg)
    print('\nran', passed+failed, 'tests in', round(time.time()-start, 2), 'seconds.',
            'passed:', str(passed)+',', 'failed:', str(failed)+'.')

# uncompyle6 version 3.3.5
# Python bytecode 2.7 (62211)
# Decompiled from: Python 3.6.7 (default, Oct 22 2018, 11:32:17) 
# [GCC 8.2.0]
# Embedded file name: py_src/case.py
# Compiled at: 2016-09-30 19:23:47
from __future__ import print_function
import contextlib, sys, os, inspect, difflib, time
from .result import TestResult
from .util import strclass, safe_repr, coloured_text
from .process import Process, TimeoutProcess, TracedProcess
BUFFER_SIZE = 8 * 1024

def marks(category, mark=None, category_marks=None):
    """Assign marks to a test or suite of tests, grouped by a category."""

    def decorator(test_item):
        if mark is None and category_marks is None:
            raise ValueError('One of mark or category_marks must be defined')
        test_item.__marks_category__ = category
        test_item.__marks_mark__ = mark
        test_item.__marks_category_marks__ = category_marks
        return test_item

    return decorator


def ignore_result(test_item):
    """Mark a test as having its results ignored.
    Used for tests that add details but do not test functionality
    """
    test_item.__marks_ignore_result__ = True
    return test_item


class _TestWrapper(object):

    def __init__(self):
        self.success = True
        self.errors = []

    @contextlib.contextmanager
    def test_executer(self, test_case, is_test=False):
        old_success = self.success
        self.success = True
        try:
            yield
        except KeyboardInterrupt:
            raise
        except:
            exc_info = sys.exc_info()
            self.success = False
            self.errors.append((test_case, exc_info))
            exc_info = None
        finally:
            self.success = self.success and old_success

        return


class TestCase(object):
    failure_exception = AssertionError
    default_test_method = 'run_test'
    process_class = Process
    timeout = None

    def __init__(self, test_method_name='run_test', timeout=None):
        try:
            getattr(self, test_method_name)
        except AttributeError:
            if test_method_name != self.default_test_method:
                raise ValueError('test method %s does not exist in %s' % (
                 strclass(self.__class__), test_method_name))
        else:
            self._test_method = test_method_name

        if timeout:
            self.timeout = timeout
        if self.timeout and self.process_class == Process:
            self.process_class = TimeoutProcess
        self._process_count = 0
        self._processes = []
        self.__details = {}

    def setup(self):
        pass

    def tear_down(self):
        pass

    @classmethod
    def setup_class(cls):
        pass

    @classmethod
    def tear_down_class(cls):
        pass

    def id(self):
        return ('{0}.{1}').format(strclass(self.__class__), self._test_method)

    def doc(self):
        """Return the docstring of the current test method"""
        return inspect.getdoc(self.test_method)

    def __str__(self):
        return ('{0} ({1})').format(self._test_method, strclass(self.__class__))

    def __repr__(self):
        return ('<{0} test_method={1}>').format(strclass(self.__class__), self._test_method)

    def __call__(self, *args, **kwargs):
        return self.run(*args, **kwargs)

    def _process_errors(self, result, errors):
        for test_case, exc_info in errors:
            if exc_info is not None:
                if issubclass(exc_info[0], self.failure_exception):
                    result.add_failure(test_case, exc_info)
                else:
                    result.add_error(test_case, exc_info)

        return

    @property
    def test_method(self):
        return getattr(self, self._test_method)

    def _stdout_filename(self, p):
        """
        Generate filename for standard out (stdout) output from a process.
        """
        return ('{0}.{1}.out').format(self.id(), p.count)

    def _stderr_filename(self, p):
        """
        Generate filename for standard error (stderr) output from a process.
        """
        return ('{0}.{1}.err').format(self.id(), p.count)

    def option(self, option):
        """Retrieves the value of an option, or None if option not set."""
        return self.__marks_options__.get(option, None)

    def _print_coloured(self, text, fg=None, bg=None, attrs=None, **kwargs):
        stream = kwargs.get('file', sys.stdout)
        if stream.isatty():
            text = coloured_text(text, colour=fg, background=bg, attrs=attrs)
        print(text, **kwargs)

    def process(self, argv, input_file=None, *args, **kwargs):
        """Create a Process of the type specified for this test case"""
        if self.timeout:
            kwargs.setdefault('timeout', self.timeout)
        if kwargs.get('timeout') is not None and not isinstance(kwargs['timeout'], int):
            raise ValueError('Process timeout must be an integer.')
        if input_file is not None:
            kwargs['input_file'] = input_file
        if self.option('explain'):
            self.process_class = ExplainProcess
        p = self.process_class(argv, *args, **kwargs)
        p.count = self._process_count
        self._process_count += 1
        self._processes.append(p)
        if self.option('explain'):
            for i, arg in enumerate(argv):
                if arg == '' or any(c.isspace() for c in arg):
                    argv[i] = ('"{0}"').format(arg)
                argv[i] = argv[i].encode('unicode_escape')

            self._print_coloured(('Start Process {0}:').format(p.count), attrs=['bold'])
            print(('\t{0}').format((' ').join(argv)), end='')
            if input_file is not None:
                print((' < {0}').format(input_file), end='')
            print((' > {0} 2> {1}').format(self._stdout_filename(p), self._stderr_filename(p)))
        return p

    def _cleanup_processes(self):
        """Attempt to kill all processes started within a test"""
        if self.option('explain'):
            return
        for p in self._processes:
            try:
                p.kill()
            except RuntimeError:
                pass

    def run(self, result=None, **kwargs):
        original_result = result
        if result is None:
            result = TestResult()
            result.start_test_run()
        ignored = getattr(self.test_method, '__marks_ignore_result__', False)
        if not ignored:
            result.start_test(self)
        self._process_count = 0
        self._processes = []
        self.__details = {}
        wrapper = _TestWrapper()
        try:
            with wrapper.test_executer(self):
                self.setup()
            if wrapper.success:
                with wrapper.test_executer(self, is_test=True):
                    self.test_method()
                with wrapper.test_executer(self):
                    self.tear_down()
                self._cleanup_processes()
            self._process_details(result)
            if not ignored:
                self._process_errors(result, wrapper.errors)
                if wrapper.success:
                    result.add_success(self)
            return result
        except KeyboardInterrupt:
            self._cleanup_processes()
            raise
        finally:
            if not ignored:
                result.stop_test(self)
            if original_result is None:
                result.stop_test_run()

        return

    def _check_signal(self, process, msg):
        """Check if process was signalled, causing the current test to fail."""
        if process.check_signalled():
            msg += ('Process received unexpected signal: {0}').format(process.signal)
        self._check_timeout(process, msg)

    def _check_timeout(self, process, msg):
        """Check if process was timed out, causing the test to fail."""
        if self.option('update'):
            return
        if process.timeout:
            msg = 'Timeout occurred'
        if not self.option('explain'):
            process.kill()
        raise self.failure_exception(msg)

    def delay(self, secs):
        """Insert a delay into a test.
        Delay is in seconds, with fractions being acceptable.
        """
        if not self.option('explain'):
            time.sleep(secs)

    def fail(self, msg=None):
        """Fail immediately, with the given message."""
        if self.option('update'):
            return
        raise self.failure_exception(msg)

    def assert_stdout_matches_file(self, process, file_path, msg=None):
        """
        Assert that the standard output of the process matches the
        contents of the given file.
        """
        if self.option('explain'):
            self._print_coloured(('Compare stdout from Process {0}:').format(process.count), attrs=[
             'bold'])
            print(('\tdiff {0} {1}').format(self._stdout_filename(process), file_path))
            return
        else:
            if self.option('update') or self.option('save'):
                filename = file_path
                if self.option('save'):
                    filename = self._stdout_filename(process)
                with open(filename, 'wb') as (f):
                    while True:
                        line = process.readline_stdout()
                        f.write(line)
                        if line == '':
                            break

            if self.option('update'):
                print(('\tstandard output file updated: {0}').format(file_path))
                return
            msg = msg or 'stdout mismatch'
            result = None
            if self.option('save'):
                result = self._compare_files(self._stdout_filename(process), file_path, msg=msg)
            elif self.option('show_diff'):
                result = self._verbose_compare(process.readline_stdout, file_path, self._stdout_filename(process), msg)
            elif not process.expect_stdout_file(file_path):
                result = msg
            if result is not None:
                self._check_signal(process, result)
            return

    def assert_stderr_matches_file(self, process, file_path, msg=None):
        """
        Assert that the standard error of the process matches the
        contents of the given file.
        """
        if self.option('explain'):
            self._print_coloured(('Compare stderr from Process {0}:').format(process.count), attrs=[
             'bold'])
            print(('\tdiff {0} {1}').format(self._stderr_filename(process), file_path))
            return
        else:
            if self.option('update') or self.option('save'):
                filename = file_path
                if self.option('save'):
                    filename = self._stderr_filename(process)
                with open(filename, 'wb') as (f):
                    while True:
                        line = process.readline_stderr()
                        f.write(line)
                        if line == '':
                            break

            if self.option('update'):
                print(('\tstandard error file updated: {0}').format(file_path))
                return
            msg = msg or 'stderr mismatch'
            result = None
            if self.option('save'):
                result = self._compare_files(self._stderr_filename(process), file_path, msg=msg)
            elif self.option('show_diff'):
                result = self._verbose_compare(process.readline_stderr, file_path, self._stderr_filename(process), msg)
            elif not process.expect_stderr_file(file_path):
                result = msg
            if result is not None:
                self._check_signal(process, result)
            return

    def assert_stdout(self, process, output, msg=None):
        """
        Assert that the standard output of the process contains the given
        output.
        """
        if self.option('explain'):
            if output == '':
                self._print_coloured(('Expect end of file (Process {0} [stdout])').format(process.count), attrs=[
                 'bold'])
            else:
                self._print_coloured(('Expect output (Process {0} [stdout]): ').format(process.count), attrs=[
                 'bold'], end='')
                print(safe_repr(output))
            return
        if self.option('update'):
            print(('\tCheck assert_stdout({0})').format(safe_repr(output)))
            return
        if not process.expect_stdout(output):
            msg = msg or 'stdout mismatch'
            self._check_signal(process, msg)

    def assert_stderr(self, process, output, msg=None):
        """
        Assert that the standard error of the process contains the given
        output.
        """
        if self.option('explain'):
            if output == '':
                self._print_coloured(('Expect end of file (Process {0} [stderr])').format(process.count), attrs=[
                 'bold'])
            else:
                self._print_coloured(('Expect output (Process {0} [stderr]): ').format(process.count), attrs=[
                 'bold'], end='')
                print(safe_repr(output))
            return
        if self.option('update'):
            print(('\tCheck assert_stderr({0})').format(safe_repr(output)))
            return
        if not process.expect_stderr(output):
            msg = msg or 'stderr mismatch'
            self._check_signal(process, msg)

    def assert_exit_status(self, process, status, msg=None):
        """
        Assert that the exit status of the process matches the given status.
        """
        if self.option('explain'):
            self._print_coloured(('Expect exit status (Process {0}): ').format(process.count), attrs=[
             'bold'], end='')
            print(status)
            return
        if not process.assert_exit_status(status):
            msg = msg or ('exit status mismatch: expected {0}, got {1}').format(status, process.exit_status)
            self._check_signal(process, msg)

    def assert_signalled(self, process, msg=None):
        """
        Assert that the process received a signal.
        """
        if self.option('explain'):
            self._print_coloured(('Expect Process {0} to receive signal').format(process.count), attrs=[
             'bold'])
            return
        if not process.assert_signalled():
            msg = msg or 'program did not receive signal'
            self._check_timeout(process, msg)

    def assert_signal(self, process, signal, msg=None):
        """
        Assert that the signal of the process matches the given signal.
        """
        if self.option('explain'):
            self._print_coloured(('Expect signal (Process {0}): ').format(process.count), attrs=[
             'bold'], end='')
            print(signal)
            return
        if not process.assert_signal(signal):
            msg = msg or ('signal mismatch: expected {0}, got {1}').format(signal, process.signal)
            self._check_timeout(process, msg)

    def assert_files_equal(self, file1, file2, msg=None):
        """
        Assert that the given files contain exactly the same contents.
        """
        if self.option('explain'):
            self._print_coloured('Check files are the same:', attrs=['bold'])
            print(('\tdiff {0} {1}').format(file1, file2))
            return
        else:
            result = self._compare_files(file1, file2, msg=msg)
            if result is not None:
                raise self.failure_exception(result)
            return

    def _compare_files(self, file1, file2, msg1=None, msg2=None, msg=None):
        if not os.path.exists(file1):
            return msg1 or ('file missing: {0}').format(file1)
        if not os.path.exists(file2):
            return msg2 or ('file missing: {0}').format(file1)
        f1 = open(file1, 'rb')
        f2 = open(file2, 'rb')
        different = False
        while True:
            b1 = f1.read(BUFFER_SIZE)
            b2 = f2.read(BUFFER_SIZE)
            if b1 != b2:
                different = True
                break
            if not b1:
                break

        f1.close()
        f2.close()
        if different:
            msg = msg or 'file mismatch: contents do not exactly match'
            if self.option('show_diff'):
                f1 = open(file1, 'rb')
                f2 = open(file2, 'rb')
                diff = difflib.unified_diff(f1.readlines(), f2.readlines(), fromfile=file1, tofile=file2)
                msg += ('\nDiff leading to failure:\n{0}').format(('').join(diff))
                f1.close()
                f2.close()
            return msg

    def _verbose_compare(self, stream_readline, file_path, stream_name, msg):
        if not os.path.exists(file_path):
            return ('file missing: {0}').format(file_path)
        different = False
        p_history = []
        f_history = []
        with open(file_path, 'rb') as (f):
            while True:
                p_line = stream_readline()
                f_line = f.readline()
                p_history.append(p_line)
                f_history.append(f_line)
                if f_line != p_line:
                    different = True
                    break
                if not f_line:
                    break

        if different:
            diff = difflib.unified_diff(p_history, f_history, fromfile=stream_name, tofile=file_path)
            msg += '\nDiff leading to failure [truncated]:\n'
            msg += ('').join(diff)
            return msg

    def add_detail(self, name, data):
        """Record information related to the test"""
        self.__details[name] = data

    def _process_details(self, result):
        """Update details in the result with those stored from the test."""
        result.update_details(self.__details)

    def child_pids(self, parent):
        """Get the process IDs of the children of the given parent process."""
        pids = []
        if self.option('explain'):
            self._print_coloured(('Get IDs of child processes of Process {0}').format(parent.count), attrs=[
             'bold'])
        elif isinstance(parent, TracedProcess):
            pids = parent.child_pids()
        else:
            pgrep = self.process(['pgrep', '-P', str(parent.pid)])
            while True:
                pid = pgrep.readline_stdout()
                if pid == '':
                    break
                pid = int(pid.strip())
                pids.append(pid)

            pgrep.assert_exit_status(0)
            del pgrep
        return pids

    def signal_process(self, pid, sig, explain_process=None):
        """Send a signal to the process with the given ID."""
        if self.option('explain'):
            proc = explain_process or 'a process (determined at runtime)'
            msg = ('Send signal {0} to {1}').format(sig, proc)
            self._print_coloured(msg, attrs=['bold'])
        else:
            try:
                os.kill(pid, sig)
            except:
                self.fail(('Failed to send signal {0} process {1}').format(sig, pid))


class ExplainProcess(object):
    """A dummy process class, for use with the explain test functionality"""

    def __init__(self, argv, input_file=None, **kwargs):
        self.argv = argv
        self.input_file = input_file
        self._send_count = 0

    def _print_coloured(self, text, fg=None, bg=None, attrs=None, **kwargs):
        stream = kwargs.get('file', sys.stdout)
        if stream.isatty():
            text = coloured_text(text, colour=fg, background=bg, attrs=attrs)
        print(text, **kwargs)

    def finish_input(self):
        self._print_coloured(('Finish input to Process {0} (ie. Ctrl+D)').format(self.count), attrs=[
         'bold'])

    def kill(self):
        self._print_coloured(('Kill Process {0}').format(self.count), attrs=['bold'], end='')
        print(' (send SIGKILL to process group)')

    def readline_stderr(self):
        return ''

    def readline_stdout(self):
        return ''

    def send(self, message):
        if self._send_count < 11:
            self._print_coloured(('Send input to Process {0}: ').format(self.count), attrs=[
             'bold'], end='')
            print(safe_repr(message))
        elif self._send_count == 11:
            self._print_coloured(('Further input sent to Process {0} - see test case for details').format(self.count), attrs=[
             'bold'])
        self._send_count += 1

    def print_stdout(self):
        pass

    def print_stderr(self):
        pass

    def send_signal(self, signal):
        self._print_coloured(('Send signal to Process {0}: ').format(self.count), attrs=[
         'bold'], end='')
        print(signal)

    def send_signal_group(self, signal):
        self._print_coloured(('Send signal to Process {0} (incl. children): ').format(self.count), attrs=[
         'bold'], end='')
        print(signal)
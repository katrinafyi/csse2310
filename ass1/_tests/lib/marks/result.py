# uncompyle6 version 3.3.5
# Python bytecode 2.7 (62211)
# Decompiled from: Python 3.6.7 (default, Oct 22 2018, 11:32:17) 
# [GCC 8.2.0]
# Embedded file name: py_src/result.py
# Compiled at: 2017-08-23 13:50:41
from __future__ import division, print_function
import sys
from . import util
RESULT_TEMPLATE = 'Ran {0} tests: {1} success, {2} errors, {3} failures'
RESULT_ERROR = 0
RESULT_FAILURE = 0
RESULT_SUCCESS = 1

class TestResult(object):
    """Container of test result information."""

    def __init__(self):
        self.failures = []
        self.errors = []
        self.successes = []
        self.tests_run = 0
        self.details = {}
        self._test_class_setup = {}
        self._test_module_setup = {}

    def start_test_run(self):
        """Run before the testing starts."""
        pass

    def start_test(self, test):
        """Record a test as having started."""
        self.tests_run += 1

    def stop_test(self, test):
        """Record a test as having finished."""
        pass

    def stop_test_run(self):
        """Run after all of the testing is complete."""
        pass

    def add_module_setup(self, module_name, success=True):
        """Record a test module setup as complete."""
        self._test_module_setup[module_name] = success

    def module_setup_run(self, module_):
        """Check if setup for a module has occurred."""
        return module_ in self._test_module_setup

    def module_setup_failed(self, module_name):
        """Check if setup for a module failed."""
        return not self._test_module_setup.get(module_name, True)

    def test_modules(self):
        """Return list of modules that have been setup."""
        return self._test_module_setup.keys()

    def add_class_setup(self, class_, success=True):
        """Record a test class setup as complete."""
        self._test_class_setup[class_] = success

    def class_setup_run(self, class_):
        """Check if setup for a class has occurred."""
        return class_ in self._test_class_setup

    def class_setup_failed(self, class_):
        """Check if setup for a test class failed."""
        return not self._test_class_setup.get(class_, True)

    def test_classes(self):
        """Return list of test classes that have been setup."""
        return self._test_class_setup.keys()

    def add_failure(self, test, error):
        """Record a test failure."""
        self.failures.append((test, self._exc_info_pretty_print(error, test)))

    def add_error(self, test, error):
        """Record a test error."""
        self.errors.append((test, self._exc_info_pretty_print(error, test)))

    def add_success(self, test):
        """Record a test success."""
        self.successes.append((test, None))
        return

    def add_detail(self, name, data):
        """Record information about a test."""
        self.details[name] = data

    def update_details(self, details):
        """Update details from a test."""
        self.details.update(details)

    def get_details(self):
        """Retrieve all stored information about tests."""
        return self.details

    def _exc_info_pretty_print(self, exc_info, test):
        exc_type, value, tb = exc_info
        return str(value)

    def __repr__(self):
        return ('<{0} run={1} successes={2} errors={3} failures={4}>').format(util.strclass(self.__class__), self.tests_run, len(self.successes), len(self.errors), len(self.failures))

    def _print_coloured(self, text, fg=None, bg=None, attrs=None, **kwargs):
        stream = kwargs.get('file', sys.stdout)
        if stream.isatty():
            text = util.coloured_text(text, colour=fg, background=bg, attrs=attrs)
        print(text, **kwargs)

    def option(self, option):
        """Retrieves the value of an option, or None if option not set."""
        return self.__marks_options__.get(option, None)


class PrintedTestResult(TestResult):

    def start_test_run(self):
        super(PrintedTestResult, self).start_test_run()
        if not self.option('silent'):
            print('Running tests\n')

    def start_test(self, test):
        super(PrintedTestResult, self).start_test(test)
        if self.option('verbose'):
            print(('{0:60}').format(test.id()), end='')
            sys.stdout.flush()

    def stop_test(self, test):
        super(PrintedTestResult, self).stop_test(test)

    def stop_test_run(self):
        super(PrintedTestResult, self).stop_test_run()
        if self.option('verbose'):
            results = RESULT_TEMPLATE.format(self.tests_run, len(self.successes), len(self.errors), len(self.failures))
            print()
            print('-' * 70)
            print(results)

    def add_failure(self, test, error):
        super(PrintedTestResult, self).add_failure(test, error)
        if self.option('verbose'):
            self._print_coloured('FAIL', fg='yellow', attrs=['bold'])
            print(('\t{0}').format(self._exc_info_pretty_print(error, test)))

    def add_error(self, test, error):
        super(PrintedTestResult, self).add_error(test, error)
        if self.option('verbose'):
            self._print_coloured('ERROR', fg='cyan', attrs=['bold'])
            print(('\t{0}').format(self._exc_info_pretty_print(error, test)))

    def add_success(self, test):
        super(PrintedTestResult, self).add_success(test)
        if self.option('verbose'):
            self._print_coloured('OK', fg='green', attrs=['bold'])


class MarkingTestResult(PrintedTestResult):
    """Container of test result information."""

    def __init__(self):
        super(MarkingTestResult, self).__init__()
        self.tests = {}
        self.marks = {}
        self.tests_passed = 0
        self.total_tests = 0
        self.total_marks = 0
        self.received_marks = 0

    def _record_test(self, test, outcome):
        test_method = test.test_method
        category = getattr(test_method, '__marks_category__', '') or getattr(test.__class__, '__marks_category__', '')
        cat_marks = getattr(test_method, '__marks_category_marks__', 0) or getattr(test.__class__, '__marks_category_marks__', 0)
        mark = getattr(test_method, '__marks_mark__', 0) or getattr(test.__class__, '__marks_mark__', 0)
        r = self.marks.setdefault(category, {})
        r.setdefault('total_marks', 0)
        category_marks = r.setdefault('category_marks', cat_marks)
        if not cat_marks:
            r['total_marks'] += mark
        elif cat_marks != category_marks:
            raise ValueError(("Differing total marks for category '{0}' ({1} vs {2})").format(category, category_marks, cat_marks))
        r.setdefault('tests', []).append(outcome)
        if r['total_marks'] and cat_marks:
            raise ValueError(("Category '{0}' cannot have both category marks and individual test marks").format(category))
        self.tests[test.id()] = outcome

    def stop_test_run(self):
        """Run after all of the testing is complete."""
        for category in self.marks:
            info = self.marks[category]
            if info['category_marks']:
                passed = info['tests'].count(RESULT_SUCCESS)
                mark = passed / len(info['tests']) * info['category_marks']
                self.total_marks += info['category_marks']
            else:
                failed = info['tests'].count(RESULT_FAILURE) + info['tests'].count(RESULT_ERROR)
                passed = len(info['tests']) - failed
                mark = sum(info['tests'])
                self.total_marks += info['total_marks']
            info['passed'] = passed
            info['mark'] = mark
            self.tests_passed += passed
            self.total_tests += len(info['tests'])
            self.received_marks += mark

    def module_setup_failed(self, module_name):
        return False

    def class_setup_failed(self, class_):
        return False

    def add_failure(self, test, error):
        """Record a test failure."""
        super(MarkingTestResult, self).add_failure(test, error)
        self._record_test(test, RESULT_FAILURE)

    def add_error(self, test, error):
        """Record a test error."""
        super(MarkingTestResult, self).add_error(test, error)
        self._record_test(test, RESULT_ERROR)

    def add_success(self, test):
        """Record a test success."""
        super(MarkingTestResult, self).add_success(test)
        test_method = test.test_method
        mark = getattr(test_method, '__marks_mark__', 0) or getattr(test.__class__, '__marks_mark__', RESULT_SUCCESS)
        self._record_test(test, mark)

    def export(self):
        """Export the test results and marks as a dictionary"""
        results = {'tests': self.tests, 
           'results': {'failures': [ test.id() for test, err in self.failures ], 'errors': [ test.id() for test, err in self.errors ], 'successes': [ test.id() for test, err in self.successes ]}, 'marks': self.marks, 
           'totals': {'passed': self.tests_passed, 
                      'test_count': self.total_tests, 
                      'received_marks': self.received_marks, 
                      'total_marks': self.total_marks}, 
           'details': self.get_details()}
        return results


class UpdateTestResult(TestResult):

    def start_test_run(self):
        super(UpdateTestResult, self).start_test_run()
        print('Updating tests\n')

    def start_test(self, test):
        super(UpdateTestResult, self).start_test(test)
        self._print_coloured(('==> {0}:').format(test.id()), attrs=['bold'])

    def stop_test(self, test):
        print()


class ExplainTestResult(TestResult):

    def start_test_run(self):
        super(ExplainTestResult, self).start_test_run()
        print('Showing explanation for tests')
        message = 'NOTE: THIS IS AN EXPLANATION ONLY. NO TESTS ARE RUN.'
        self._print_coloured(message, fg='yellow', attrs=['bold'])
        print('To replicate a test, all given commands must be executed.\n')

    def start_test(self, test):
        super(ExplainTestResult, self).start_test(test)
        self._print_coloured(('==> {0}:').format(test.id()), fg='green', attrs=['bold'])
        doc = test.doc()
        if doc:
            self._print_coloured('About the test:', attrs=['bold'])
            print(doc)
        self._print_coloured('What the test runs and checks:', attrs=['bold'])

    def stop_test(self, test):
        super(ExplainTestResult, self).stop_test(test)
        print()

    def stop_test_run(self):
        super(ExplainTestResult, self).stop_test_run()
        print(('Explained {0} tests.').format(self.tests_run))
        message = 'NOTE: THIS IS AN EXPLANATION ONLY. NO TESTS WERE RUN.'
        self._print_coloured(message, fg='yellow', attrs=['bold'])
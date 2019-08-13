from __future__ import division, print_function
import os, errno, tempfile, shutil
from . import result
from .case import TestCase
from .process import TracedProcess
TEMP_PREFIX = 'testres'
DEFAULT_CLEANUP = True
DEFAULT_SILENT = False

class BasicTestRunner(object):
    result_class = result.PrintedTestResult

    def __init__(self, result_class=None, **kwargs):
        self.options = kwargs
        if result_class is not None:
            self.result_class = result_class
        self.options['working_dir'] = os.getcwd()
        return

    def _apply_options(self, obj):
        """Apply the appropriate options to the object"""
        obj.__marks_options__ = self.options

    def setup_environment(self):
        """Setup the environment before running the tests.

        Create a temporary directory to run tests from. Store location
        in options, so test cases can use this information.
        """
        working_dir = self.options['working_dir']
        if self.options.get('explain', False):
            temp_dir = working_dir
        elif not self.options.get('silent', DEFAULT_SILENT):
            print('Setting up environment...')
            prefix = self.options.get('temp_prefix', TEMP_PREFIX)
            temp_dir = tempfile.mkdtemp(dir=working_dir, prefix=prefix)
            if not self.options.get('cleanup', DEFAULT_CLEANUP):
                print('Test output in', temp_dir)
                print('Remember to clean up your test result folders.\n')
            os.chdir(temp_dir)

        self.options['temp_dir'] = temp_dir

    def tear_down_environment(self):
        """Tear down the environment after running the tests"""
        if not self.options.get('explain', False):
            if not self.options.get('silent', DEFAULT_SILENT):
                print('Tearing down environment...')
            os.chdir(self.options['working_dir'])
            if self.options.get('cleanup', DEFAULT_CLEANUP):
                try:
                    shutil.rmtree(self.options['temp_dir'])
                except OSError as e:
                    if e.errno != errno.ENOENT:
                        raise

    def run(self, test):
        self.setup_environment()
        self._apply_options(test)
        result = self.result_class()
        self._apply_options(result)
        result.start_test_run()
        test.run(result)
        result.stop_test_run()
        self.tear_down_environment()
        return result


class MarkingTestRunner(BasicTestRunner):
    result_class = result.MarkingTestResult

    def run(self, test):
        self.setup_environment()
        self._apply_options(test)
        TestCase.process_class = TracedProcess
        result = self.result_class()
        self._apply_options(result)
        result.start_test_run()
        test.run(result)
        result.stop_test_run()
        if not self.options.get('silent', False):
            if self.options.get('verbose', False):
                print()
            print('Marking Results')
            print(('{0:30}{1:15}{2}').format('Category', 'Passed', 'Mark'))
            for category in result.marks:
                info = result.marks[category]
                total = info['total_marks'] or info['category_marks']
                if total == 0:
                    fraction = 0
                else:
                    fraction = info['mark'] / total
                print(('{0:30}{1:15}{2:.2f}/{3:.2f} ({4:.2%})').format(category, ('{0}/{1}').format(info['passed'], len(info['tests'])), info['mark'], total, fraction))

            print(('\n{0:30}{1:15}{2:.2f}/{3:.2f} ({4:.2%})').format('Total', ('{0}/{1}').format(result.tests_passed, result.total_tests), result.received_marks, result.total_marks, result.received_marks / result.total_marks))
        self.tear_down_environment()
        return result


CONFIRMATION_MESSAGE = '\nPlease confirm that you want to update the output files in this test suite.\n\nNOTE: THIS PROCESS WILL MODIFY EXISTING FILES.\n      PLEASE ENSURE YOU HAVE A BACKUP BEFORE PROCEEDING.\n'

class UpdateTestRunner(BasicTestRunner):
    result_class = result.UpdateTestResult

    def run(self, test):
        print(CONFIRMATION_MESSAGE)
        confirm = raw_input('Are you sure you want to update the files? (y/N)')
        if confirm == 'y':
            self.options['update'] = True
            super(UpdateTestRunner, self).run(test)
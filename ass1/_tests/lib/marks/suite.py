# uncompyle6 version 3.3.5
# Python bytecode 2.7 (62211)
# Decompiled from: Python 3.6.7 (default, Oct 22 2018, 11:32:17) 
# [GCC 8.2.0]
# Embedded file name: py_src/suite.py
# Compiled at: 2014-11-09 15:38:48
import sys
from .case import _TestWrapper
from .result import TestResult
from .util import strclass

class TestSuite(object):

    def __init__(self, tests=()):
        self._tests = []
        self.add_tests(tests)

    def __repr__(self):
        return ('<{0} tests={1}>').format(strclass(self.__class__), list(self))

    def __iter__(self):
        return iter(self._tests)

    def __call__(self, *args, **kwargs):
        return self.run(*args, **kwargs)

    def add_test(self, test):
        if not hasattr(test, '__call__'):
            raise TypeError(('Test {0} is not callable').format(repr(test)))
        self._tests.append(test)

    def add_tests(self, tests):
        if isinstance(tests, basestring):
            raise TypeError('tests must be an iterable of tests, not a string')
        for test in tests:
            self.add_test(test)

    def run(self, result=None, child=False):
        original_result = result
        if result is None:
            result = TestResult()
            result.start_test_run()
        try:
            for test in self:
                self._setup_module(test, result)
                self._setup_class(test, result)
                module_name = test.__class__.__module__
                if not (result.module_setup_failed(module_name) or result.class_setup_failed(test.__class__)):
                    test.run(result, child=True)

            if not child:
                self._tear_down_classes(result)
                self._tear_down_modules(result)
            return result
        finally:
            if original_result is None:
                result.stop_test_run()

        return

    def _setup_module(self, test, result):
        module_name = test.__class__.__module__
        if module_name == __name__:
            return
        else:
            if result.module_setup_run(module_name):
                return
            try:
                module = sys.modules[module_name]
            except KeyError:
                return

            wrapper = _TestWrapper()
            if getattr(module, 'setup_module', None):
                with wrapper.test_executer(self):
                    options = getattr(self, '__marks_options__', {})
                    module.setup_module(options)
            result.add_module_setup(module_name, wrapper.success)
            return

    def _tear_down_module(self, module_name, result):
        try:
            module = sys.modules[module_name]
        except KeyError:
            return

        wrapper = _TestWrapper()
        if getattr(module, 'tear_down_module', None):
            with wrapper.test_executer(self):
                options = getattr(self, '__marks_options__', {})
                module.tear_down_module(options)
        return

    def _tear_down_modules(self, result):
        for module_name in result.test_modules():
            self._tear_down_module(module_name, result)

    def _setup_class(self, test, result):
        class_ = test.__class__
        self._apply_options(class_)
        if result.class_setup_run(class_):
            return
        else:
            if result.module_setup_failed(class_.__module__):
                return
            wrapper = _TestWrapper()
            if getattr(class_, 'setup_class', None):
                with wrapper.test_executer(self):
                    class_.setup_class()
            result.add_class_setup(class_, wrapper.success)
            return

    def _tear_down_class(self, class_, result):
        wrapper = _TestWrapper()
        if getattr(class_, 'tear_down_class', None):
            with wrapper.test_executer(self):
                class_.tear_down_class()
        return

    def _tear_down_classes(self, result):
        for class_ in result.test_classes():
            self._tear_down_class(class_, result)

    def _apply_options(self, class_):
        """Apply the appropriate options to the test class"""
        class_.__marks_options__ = getattr(self, '__marks_options__', {})
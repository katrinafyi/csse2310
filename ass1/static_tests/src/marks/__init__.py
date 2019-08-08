# uncompyle6 version 3.3.5
# Python bytecode 2.7 (62211)
# Decompiled from: Python 3.6.7 (default, Oct 22 2018, 11:32:17) 
# [GCC 8.2.0]
# Embedded file name: py_src/__init__.py
# Compiled at: 2014-10-09 21:25:22
"""
MARKS - systems programming testing and marking framework.

Based on Python unittest framework.
"""
__all__ = [
 '__version__', 'ignore_result', 'main', 'marks', 'Process', 'TestCase',
 'TestSuite', 'TestLoader', 'default_test_loader', 'TestResult',
 'set_ld_preload', 'get_ld_preload']
from ._version import get_version
from .process import Process, set_ld_preload, get_ld_preload
from .case import TestCase, marks, ignore_result
from .suite import TestSuite
from .loader import TestLoader, default_test_loader
from .result import TestResult
from .main import main
__version__ = get_version()
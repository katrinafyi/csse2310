# uncompyle6 version 3.3.5
# Python bytecode 2.7 (62211)
# Decompiled from: Python 3.6.7 (default, Oct 22 2018, 11:32:17) 
# [GCC 8.2.0]
# Embedded file name: py_src/_version.py
# Compiled at: 2014-11-09 15:38:48
"""Version Information

Used from Django:
    https://github.com/django/django/blob/master/django/utils/version.py
"""
VERSION = (0, 5, 0, 'alpha', 8)

def get_version(version=None):
    """Returns a PEP 386-compliant version number from VERSION."""
    version = get_complete_version(version)
    major = get_major_version(version)
    sub = ''
    if version[3] == 'alpha' and version[4] == 0:
        pass
    elif version[3] != 'final':
        mapping = {'alpha': 'a', 'beta': 'b', 'rc': 'c'}
        sub = mapping[version[3]] + str(version[4])
    return str(major + sub)


def get_major_version(version=None):
    """Returns major version from VERSION."""
    version = get_complete_version(version)
    parts = 2 if version[2] == 0 else 3
    major = ('.').join(str(x) for x in version[:parts])
    return major


def get_complete_version(version=None):
    """Returns a tuple of the version. If version argument is non-empty,
    then checks for correctness of the tuple provided.
    """
    if version is None:
        version = VERSION
    else:
        assert len(version) == 5
        assert version[3] in ('alpha', 'beta', 'rc', 'final')
    return version
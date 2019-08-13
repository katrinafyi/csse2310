# uncompyle6 version 3.3.5
# Python bytecode 2.7 (62211)
# Decompiled from: Python 3.6.7 (default, Oct 22 2018, 11:32:17) 
# [GCC 8.2.0]
# Embedded file name: py_src/util.py
# Compiled at: 2014-10-07 16:44:01
import os
_MAX_LENGTH = 80

def strclass(cls):
    """Generate a class name string, including module path.

    Remove module '__main__' from the ID, as it is not useful in most cases.
    """
    if cls.__module__ == '__main__':
        return cls.__name__
    return ('{0}.{1}').format(cls.__module__, cls.__name__)


def safe_repr(obj, length=None):
    """Safely generate a string representation of an object."""
    if length is None:
        length = _MAX_LENGTH
    try:
        result = repr(obj)
    except Exception:
        result = object.__repr__(obj)

    if len(result) <= length:
        return result
    else:
        return result[:length] + ' [truncated]...'


COLOUR_FORMAT = '\x1b[{0}m{1}'
COLOURS = ('grey', 'red', 'green', 'yellow', 'blue', 'magenta', 'cyan', 'white')
BACKGROUND = dict(list(zip(COLOURS, list(range(40, 48)))))
FOREGROUND = dict(list(zip(COLOURS, list(range(30, 38)))))
ATTRIBUTES = dict(list(zip(('bold', 'dark', '', 'underline', 'blink', '', 'reverse',
                            'concealed'), list(range(1, 9)))))
del ATTRIBUTES['']
RESET = '\x1b[0m'

def coloured_text(text, colour=None, background=None, attrs=None):
    """Add ANSI colours and attributes to text.

    Available foreground and background colors:
        red, green, yellow, blue, magenta, cyan, white.

    Available attributes:
        bold, dark, underline, blink, reverse, concealed.
    """
    if os.getenv('ANSI_COLORS_DISABLED') is None:
        if colour is not None:
            text = COLOUR_FORMAT.format(FOREGROUND[colour], text)
        if background is not None:
            text = COLOUR_FORMAT.format(BACKGROUND[background], text)
        if attrs is not None:
            for attr in attrs:
                text = COLOUR_FORMAT.format(ATTRIBUTES[attr], text)

        text += RESET
    return text
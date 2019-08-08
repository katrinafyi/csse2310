# uncompyle6 version 3.3.5
# Python bytecode 2.7 (62211)
# Decompiled from: Python 3.6.7 (default, Oct 22 2018, 11:32:17) 
# [GCC 8.2.0]
# Embedded file name: py_src/marking.py
# Compiled at: 2015-12-05 17:35:47
from __future__ import division, print_function
import os, traceback, copy, multiprocessing as mp
from multiprocessing.pool import Pool
import csv, json
from .runner import BasicTestRunner, MarkingTestRunner
NUM_PROCESSES = 4

class LogException(object):
    """Log any exception raised in a Pool worker"""

    def __init__(self, callable):
        self.__callable = callable

    def __call__(self, *args, **kwargs):
        try:
            result = self.__callable(*args, **kwargs)
        except Exception:
            self._error(traceback.format_exc())
            raise

        return result

    def _error(self, msg, *args):
        return mp.get_logger().error(msg, *args)


class LoggingPool(Pool):

    def apply_async(self, func, args=(), kwds={}, callback=None):
        return Pool.apply_async(self, LogException(func), args, kwds, callback)


def mark_submission(path, test, options):
    """Mark a single submission"""
    try:
        os.chdir(path)
        submission = os.path.basename(os.path.normpath(path))
        options['submission'] = submission
        print('-> Start marking submission:', submission)
        options['silent'] = True
        options['cleanup'] = True
        if options.get('marking_setup', False):
            options['marking_setup'](options)
        runner = MarkingTestRunner(**options)
        result = runner.run(test)
        if options.get('marking_tear_down', False):
            options['marking_tear_down'](options)
        details = result.export()
        details['submission'] = submission
        with open('results.json', 'w') as (f):
            json.dump(details, f, indent=4)
        print(('-> Finished marking submission: {0} ({1})').format(submission, details['totals']['received_marks']))
        return details
    except KeyboardInterrupt:
        return


class MarkingRunner(BasicTestRunner):

    def _list_dirs(self):
        """List all directories within the target directory"""
        dirs = []
        root = self.options.get('directory')
        for f in os.listdir(root):
            if os.path.isdir(os.path.join(root, f)):
                dirs.append(f)

        return dirs

    def _submissions(self, test):
        """Generator returning path, test and options"""
        for folder in self._list_dirs():
            full_path = os.path.join(self.options['directory'], folder)
            yield (full_path, test, copy.deepcopy(self.options))

    def _set_protection(self):
        import marks
        preload = self.options.get('ld_preload', _default_protection())
        marks.set_ld_preload(preload)

    def run(self, test):
        self._set_protection()
        processes = self.options.get('processes', NUM_PROCESSES)
        count = {'submissions': len(self._list_dirs()), 
           'marked': 0}
        print('Starting marking:', count['submissions'], 'submissions')
        mp.log_to_stderr()
        pool = LoggingPool(processes=processes, maxtasksperchild=1)
        results = []

        def complete(result):
            count['marked'] += 1
            print(('Marked {0}/{1} ({2:.2%})').format(count['marked'], count['submissions'], count['marked'] / count['submissions']))
            if result:
                results.append(result)

        try:
            for s in self._submissions(test):
                pool.apply_async(mark_submission, args=s, callback=complete)

            pool.close()
            pool.join()
        except KeyboardInterrupt:
            pool.terminate()
            pool.join()

        directory = self.options.get('directory')
        results_json_default = os.path.join(directory, 'overall_results.json')
        results_json = self.options.get('overall_results_json', results_json_default)
        results_csv_default = os.path.join(directory, 'marking_results.csv')
        results_csv = self.options.get('overall_results_csv', results_csv_default)
        with open(results_json, 'w') as (f):
            json.dump(results, f)
        if results:
            header = [
             'submission']
            header.extend(results[0]['tests'].keys())
            header.extend(results[0]['details'].keys())
            header.append('mark')
            with open(results_csv, 'w') as (f):
                dw = csv.DictWriter(f, fieldnames=header)
                dw.writerow(dict((fn, fn) for fn in header))
                for res in sorted(results, key=lambda x: x['submission']):
                    info = {'submission': res['submission'], 'mark': res['totals']['received_marks']}
                    info.update(res['tests'])
                    info.update(res['details'])
                    dw.writerow(info)


def _default_protection():
    """Get the path to the included protection library"""
    import marks
    return os.path.join(os.path.dirname(os.path.abspath(marks.__file__)), 'libprotect.so')
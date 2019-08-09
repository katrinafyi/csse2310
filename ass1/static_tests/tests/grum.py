#!/usr/bin/env python
from __future__ import print_function
import sys
import os

# Get correct path to files, based on platform
import platform
host = platform.node().split('.')[0]

if host=="moss":
   sys.path[0:0]=['/local/courses/csse2310/lib']
   TEST_LOCATION = '/home/users/uqjfenw1/public/2019/ptesta1'
else:
	sys.path[0:0] = [ 
	    '/home/joel/marks',
	]
import marks

COMPILE = "make"
 
 
class Assignment1(marks.TestCase):
  timeout = 12 
  @classmethod
  def setup_class(cls):
        # Store original location
        options = getattr(cls, '__marks_options__', {})
        cls.prog = os.path.join(options['working_dir'], './bark')
        cls._compile_warnings = 0
        cls._compile_errors = 0
 
        # Create symlink to tests in working dir
        os.chdir(options['working_dir'])
        try:
            os.symlink(TEST_LOCATION, 'tests')
        except OSError:
            pass
        os.chdir(options['temp_dir'])

        # Modify test environment when running tests (excl. explain mode).
        if not options.get('explain', False):
            # Compile program
            os.chdir(options['working_dir'])
            p = marks.Process(COMPILE.split())
            os.chdir(options['temp_dir'])
 
            # Count warnings and errors
            warnings = 0
            errors = 0
            while True:
                line = p.readline_stderr()
                if line == '':
                    break
                if 'warning:' in line:
                    warnings += 1
                if 'error:' in line:
                    errors += 1
                print(line, end='')

            # Do not run tests if compilation failed.
            assert p.assert_exit_status(0)
 
            # Create symlink to tests within temp folder
            try:
                os.symlink(TEST_LOCATION, 'tests')
            except OSError:
                pass 

  @marks.marks('numargs', category_marks=1)
  def test_usage1(self):
    """ """
    #1 empty empty usage.err - - 
    p = self.process([self.prog])
    p.finish_input()
    self.assert_stdout_matches_file(p, 'tests/usage1.out')
    self.assert_stderr_matches_file(p, 'tests/usage1.err')
    self.assert_exit_status(p, 1)

  @marks.marks('numargs', category_marks=1)
  def test_usage2(self):
    """ """
    #1 empty empty usage.err - - 
    p = self.process([self.prog]+["",""])
    p.finish_input()
    self.assert_stdout_matches_file(p, 'tests/usage2.out')
    self.assert_stderr_matches_file(p, 'tests/usage2.err')
    self.assert_exit_status(p, 1)

  @marks.marks('typeargs', category_marks=2)
  def test_bad_type1(self):
    """ """
    #1 empty empty usage.err - - 
    p = self.process([self.prog]+["xx", "", ""])
    p.finish_input()
    self.assert_stdout_matches_file(p, 'tests/empty')
    self.assert_stderr_matches_file(p, 'tests/type.err')
    self.assert_exit_status(p, 2)

  @marks.marks('typeargs', category_marks=2)
  def test_bad_type2(self):
    """ """
    #1 empty empty usage.err - - 
    p = self.process([self.prog]+["tests/blarg", "x4", "5", "h", "a"])
    p.finish_input()
    self.assert_stdout_matches_file(p, 'tests/empty')
    self.assert_stderr_matches_file(p, 'tests/type.err')
    self.assert_exit_status(p, 2)

  @marks.marks('baddeck', category_marks=2)
  def test_bad_deck1(self):
    """ """
    #1 empty empty usage.err - - 
    p = self.process([self.prog]+["tests/wally", "5", "5", "h", "h"])
    p.finish_input()
    self.assert_stdout_matches_file(p, 'tests/empty')
    self.assert_stderr_matches_file(p, 'tests/deck.err')
    self.assert_exit_status(p, 3)

  @marks.marks('baddeck', category_marks=2)
  def test_bad_deck2(self):
    """ """
    #1 empty empty usage.err - - 
    p = self.process([self.prog]+["tests/bd3", "5", "5", "h", "h"])
    p.finish_input()
    self.assert_stdout_matches_file(p, 'tests/empty')
    self.assert_stderr_matches_file(p, 'tests/deck.err')
    self.assert_exit_status(p, 3)

  @marks.marks('BadSave', category_marks=2)
  def test_bad_sf1(self):
    """ """
    #1 empty empty usage.err - - 
    p = self.process([self.prog]+["tests/s1", "h", "h"])
    p.finish_input()
    self.assert_stdout_matches_file(p, 'tests/empty')
    self.assert_stderr_matches_file(p, 'tests/sfx.err')
    self.assert_exit_status(p, 3)

  @marks.marks('BadSave', category_marks=2)
  def test_bad_sf2(self):
    """ """
    #1 empty empty usage.err - - 
    p = self.process([self.prog]+["tests/s3", "h", "h"])
    p.finish_input()
    self.assert_stdout_matches_file(p, 'tests/empty')
    self.assert_stderr_matches_file(p, 'tests/sf.err')
    self.assert_exit_status(p, 4)

  @marks.marks('initboard', category_marks=2)
  def test_init1(self):
    """ """
    #1 empty empty usage.err - - 
    p = self.process([self.prog]+["tests/d1", "4", "3", "h", "h"])
    p.finish_input()
    self.assert_stdout_matches_file(p, 'tests/init1.out')
    self.assert_stderr_matches_file(p, 'tests/eoi.err')
    self.assert_exit_status(p, 7)

  @marks.marks('initboard', category_marks=2)
  def test_init2(self):
    """ """
    #1 empty empty usage.err - - 
    p = self.process([self.prog]+["tests/d1", "3", "4", "h", "h"])
    p.finish_input()
    self.assert_stdout_matches_file(p, 'tests/init2.out')
    self.assert_stderr_matches_file(p, 'tests/eoi.err')
    self.assert_exit_status(p, 7)

  @marks.marks('humfirstmove', category_marks=2)
  def test_1hum2(self):
    """ """
    #1 empty empty usage.err - - 
    p = self.process([self.prog]+["tests/d1", "7", "4", "h", "h"], 'tests/1h2.in')
    p.finish_input()
    self.assert_stdout_matches_file(p, 'tests/1h2.out')
    self.assert_stderr_matches_file(p, 'tests/eoi.err')
    self.assert_exit_status(p, 7)

  @marks.marks('humsecondmove', category_marks=2)
  def test_2hum2(self):
    """ """
    #1 empty empty usage.err - - 
    p = self.process([self.prog]+["tests/d1", "5", "5", "h", "h"], 'tests/2h2.in')
    p.finish_input()
    self.assert_stdout_matches_file(p, 'tests/2h2.out')
    self.assert_stderr_matches_file(p, 'tests/eoi.err')
    self.assert_exit_status(p, 7)

  @marks.marks('humsavegame', category_marks=2)
  def test_hs1(self):
    """ """
    #1 empty empty usage.err - - 
    p = self.process([self.prog]+["tests/d4", "6", "4", "h", "h"], 'tests/hs1.in')
    p.finish_input()
    self.assert_stdout_matches_file(p, 'tests/hs1.out')
    self.assert_stderr_matches_file(p, 'tests/eoi.err')
    self.assert_files_equal('tsave.1', 'tests/tsave.1')
    self.assert_exit_status(p, 7)

  @marks.marks('humsavegame', category_marks=2)
  def test_hs2(self):
    """ """
    #1 empty empty usage.err - - 
    p = self.process([self.prog]+["tests/d4", "3", "4", "h", "h"], 'tests/hs2.in')
    p.finish_input()
    self.assert_stdout_matches_file(p, 'tests/hs2.out')
    self.assert_stderr_matches_file(p, 'tests/eoi.err')
    self.assert_files_equal('tsave.2', 'tests/tsave.2')
    self.assert_exit_status(p, 7)

  @marks.marks('1auto', category_marks=2)
  def test_1auto1(self):
    """ """
    #1 empty empty usage.err - - 
    p = self.process([self.prog]+["tests/d6", "3", "4", "a", "h"])
    p.finish_input()
    self.assert_stdout_matches_file(p, 'tests/1a1.out')
    self.assert_stderr_matches_file(p, 'tests/eoi.err')
    self.assert_exit_status(p, 7)

  @marks.marks('1auto', category_marks=2)
  def test_1auto2(self):
    """ """
    #1 empty empty usage.err - - 
    p = self.process([self.prog]+["tests/d6", "5", "5", "h", "a"], "tests/1a2.in")
    p.finish_input()
    self.assert_stdout_matches_file(p, 'tests/1a2.out')
    self.assert_stderr_matches_file(p, 'tests/eoi.err')
    self.assert_exit_status(p, 7)

  @marks.marks('2auto', category_marks=2)
  def test_2auto1(self):
    """ """
    #1 empty empty usage.err - - 
    p = self.process([self.prog]+["tests/d6", "5", "5", "a", "h"], "tests/2a1.in")
    p.finish_input()
    self.assert_stdout_matches_file(p, 'tests/2a1.out')
    self.assert_stderr_matches_file(p, 'tests/eoi.err')
    self.assert_exit_status(p, 7)

  @marks.marks('loaddisplay', category_marks=2)
  def test_loaddisplay1(self):
    """ """
    #1 empty empty usage.err - - 
    p = self.process([self.prog]+["tests/disp1.save", "h", "a"])
    p.finish_input()
    self.assert_stdout_matches_file(p, 'tests/ld1.out')
    self.assert_stderr_matches_file(p, 'tests/eoi.err')
    self.assert_exit_status(p, 7)

  @marks.marks('loadfirst', category_marks=2)
  def test_loadfirst1(self):
    """ """
    #1 empty empty usage.err - - 
    p = self.process([self.prog]+["tests/load1.save", "a", "h"], "tests/lf1.in")
    p.finish_input()
    self.assert_stdout_matches_file(p, 'tests/lf1.out')
    self.assert_stderr_matches_file(p, 'tests/eoi.err')
    self.assert_exit_status(p, 7)

  @marks.marks('humgames', category_marks=3)
  def test_humgames1(self):
    """ """
    #1 empty empty usage.err - - 
    p = self.process([self.prog]+["tests/d1", "4", "4", "h", "h"], "tests/hg1.in")
    p.finish_input()
    self.assert_stdout_matches_file(p, 'tests/hg1.out')
    self.assert_stderr_matches_file(p, 'tests/empty')
    self.assert_exit_status(p, 0)

  @marks.marks('humgames', category_marks=3)
  def test_humgames2(self):
    """ """
    #1 empty empty usage.err - -
    p = self.process([self.prog]+["tests/d4", "4", "4", "h", "h"], "tests/hg2.in")
    p.finish_input()
    self.assert_stdout_matches_file(p, 'tests/hg2.out')
    self.assert_stderr_matches_file(p, 'tests/empty')
    self.assert_exit_status(p, 0)

  @marks.marks('huma', category_marks=6)
  def test_huma2(self):
    """ """
    #1 empty empty usage.err - -
    p = self.process([self.prog]+["tests/d1", "4", "3", "h", "a"], "tests/ha2.in")
    p.finish_input()
    self.assert_stdout_matches_file(p, 'tests/ha2.out')
    self.assert_stderr_matches_file(p, 'tests/empty')
    self.assert_exit_status(p, 0)

  @marks.marks('any', category_marks=6)
  def test_any1(self):
    """ """
    #1 empty empty usage.err - -
    p = self.process([self.prog]+["tests/d6", "9", "9", "a", "a"])
    p.finish_input()
    self.assert_stdout_matches_file(p, 'tests/any1.out')
    self.assert_stderr_matches_file(p, 'tests/empty')
    self.assert_exit_status(p, 0)


  @marks.marks('any', category_marks=6)
  def test_any2(self):
    """ """
    #1 empty empty usage.err - -
    p = self.process([self.prog]+["tests/d1", "7", "12", "a", "a"])
    p.finish_input()
    self.assert_stdout_matches_file(p, 'tests/any2.out')
    self.assert_stderr_matches_file(p, 'tests/empty')
    self.assert_exit_status(p, 0)

  def test_savefile_empty_deck(self):
    p = self.process([self.prog] + ['tests/save_3x3', 'h', 'h'])
    p.finish_input()
    self.assert_stdout_matches_file(p, 'tests/out_3x3')
    self.assert_stderr_matches_file(p, 'tests/eof')
    self.assert_exit_status(p, 7)

if __name__ == '__main__':
    marks.main()


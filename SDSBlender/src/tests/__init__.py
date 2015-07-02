# Copyright 2010  Bart Veldstra
# 
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
# 3. The name of the author may not be used to endorse or promote products 
#    derived from this software without specific prior written permission.
# 
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
# IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
# OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
# IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
# NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
# THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

import unittest, os, imp, re, sys

class TestCollector:
    def __init__(self, testloader=None):
        self.testloader = testloader
        if self.testloader is None:
            self.testloader = unittest.defaultTestLoader

    def find_tests(self):
        """Find all tests in the current directory tree."""
        return self._load_tests_from_dir(os.getcwd())

    def _is_test_candidate(self, entry):
        """Determine if a file qualifies as a test."""
        #Ignore blender tests
        if re.search("blender_tests?.py$", entry):
            return False
        return re.search("_tests?.py$", entry)

    def _load_module(self, path, entry):
        """Load a file as a module."""
        sys.path.append(path)
        entry = os.path.splitext(entry)[0]
        file, mpath, desc = imp.find_module(entry, [path])
        module = None
        try:
            module = imp.load_module(entry, file, mpath, desc)
        except ImportError as i:
            print("Import failed for module '%s': %s" % (entry, i))
        finally:
            file.close()
            sys.path.remove(path)
        return module
            

    def _load_tests_from_dir(self, current_dir):
        """Load all tests in a given directory tree."""
        dirsuite = unittest.TestSuite()
        for entry in os.listdir(current_dir):
            try:
                dir = os.path.join(current_dir, entry)
                suite = self._load_tests_from_dir(dir)
                dirsuite.addTest(suite)
            except OSError:
                if self._is_test_candidate(entry):
                    module = self._load_module(current_dir, entry)
                    if module:
                        suite = self.testloader.loadTestsFromModule(module)
                        dirsuite.addTest(suite)
        return dirsuite

class TestRunner(unittest.TextTestRunner):
    def _makeTestResult(self):
        return unittest.TextTestRunner._makeTestResult()


def run_all_tests():
    """Run all tests in the current directory tree."""
    collector = TestCollector()
    suite = collector.find_tests()
    runner = TestRunner()
    runner.run(suite)

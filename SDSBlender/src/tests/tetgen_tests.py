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

import os, unittest, io

from sds import tetgen as mod

class TestTetrahedralizeError(unittest.TestCase):
    default_mesh = "MockMeshObject"
    default_quality = 1.1
    default_area = 1.1
    default_inner = BaseException()
    default_message = "Error message"

    def setUp(self):
        self.exception = mod.TetrahedralizeError(
                                                self.default_mesh, 
                                                self.default_quality, 
                                                self.default_area, 
                                                self.default_inner,
                                                self.default_message
                                            )

    def tearDown(self):
        self.exception = None

    def test_constructor(self):
        assert self.exception.mesh == self.default_mesh
        assert self.exception.quality == self.default_quality
        assert self.exception.area == self.default_area
        assert self.exception.inner == self.default_inner
        assert self.exception.message == self.default_message

class TestTetrahedralizedMesh(unittest.TestCase):
    def test_commonprefix(self):
        tetra = mod.TetrahedralizedMesh('file.ext')
        assert tetra.commonprefix == 'file.1'

    def test_destroy(self):
        tetra = mod.TetrahedralizedMesh('tetgen_test_file.ext')
        created = []
        for ext in 'node', 'ele', 'neigh', 'face':
            create_me = 'tetgen_test_file.1.' + ext
            file = open(create_me, 'w')
            file.write("should be removed")
            file.close()
            created.append(create_me)
        tetra.destroy()
        files_in_cwd = os.listdir(os.getcwd())
        for file in created:
            assert file not in files_in_cwd

    def test_total_vertices(self):
        tetra = mod.TetrahedralizedMesh('tetgen_test_file.ext')
        file = open('tetgen_test_file.1.node', 'w')
        file.write('%d 3 0 0' % 42)
        file.write('this_file_should_be_deleted')
        file.close()
        assert tetra.total_vertices is 42
        os.remove(file.name)

    def test_total_vertices_fails_with_zero_vertices(self):
        tetra = mod.TetrahedralizedMesh('tetgen_test_file.ext')
        file = open('tetgen_test_file.1.node', 'w')
        file.write('%d 3 0 0' % 0)
        file.write('this_file_should_be_deleted')
        file.close()
        try:
            success = True
            tetra.total_vertices
            success = False
        except AssertionError:
            pass
        os.remove(file.name)
        assert success
        
class TestTetrahedralize(unittest.TestCase):
    class MockFS:
        class OutputString(io.StringIO):
            def close(self):
                pass

        def __init__(self):
            self.files = dict()
            self.removed = []
        def open(self, filename, mode='r'):
            assert mode == 'w', "Mock supports only writing"
            output = self.OutputString()
            self.files[filename] = output
            return output
        def remove(self, filename):
            assert filename in self.files, "File does not exist"
            self.removed.append(filename)

    class MockRunner:
        def __init__(self):
            self.last_run = (None, None)
            self.fail = False

        def tetgen(self, arguments):
            self.last_run = arguments
            if self.fail:
                raise OSError()

    def setUp(self):
        self.input = io.StringIO()
        self.fs = self.MockFS()
        self.runner = self.MockRunner()
        self.tetgen = mod.Tetgen(self.fs, self.runner)

    def test_writes_stream_to_file(self):
        contents = "File contents\nRow 1\nRow 2\n"
        input = io.StringIO(contents)
        self.tetgen.from_file(input)
        written = (v.getvalue() for v in self.fs.files.values())
        self.assertSameElements([contents], written)

    def test_runs_tetgen_executable(self):
        self.tetgen.from_file(self.input, 1.3)
        stored_file = list(self.fs.files.keys())[0]
        expected = (stored_file, '-n', '-p', '-q1.3', '-Q', '-Y')
        arguments = self.runner.last_run
        self.assertSameElements(expected, arguments)

    def test_creates_mesh_with_filename(self):
        mesh = self.tetgen.from_file(self.input)
        stored_file = list(self.fs.files.keys())[0]
        self.assertEquals(os.path.splitext(stored_file)[0] + '.1', 
                          mesh.commonprefix)

    def test_takes_quality_as_float(self):
        self.tetgen.from_file(self.input, quality=1.3)
        arguments = self.runner.last_run
        self.assertIn('-q1.3', arguments)

    def test_takes_area_as_float(self):
        self.tetgen.from_file(self.input, area=1.3)
        arguments = self.runner.last_run
        self.assertIn('-a1.3', arguments)

    def test_defaults_to_no_quality(self):
        self.tetgen.from_file(self.input)
        arguments = [arg for arg in self.runner.last_run 
                     if arg.startswith('-q')]
        self.assertListEqual(arguments, [])

    def test_throws_error_when_execution_fails(self):
        self.runner.fail = True
        self.assertRaises(mod.TetrahedralizeError, self.tetgen.from_file, 
                          self.input)

    def test_throws_error_when_stream_is_closed(self):
        self.input.close()
        self.assertRaises(mod.TetrahedralizeError, self.tetgen.from_file, 
                          self.input)

    def test_exception_still_removes_file(self):
        self.runner.fail = True
        self.assertRaises(mod.TetrahedralizeError, self.tetgen.from_file, 
                          self.input)
        self.assertEquals(1, len(self.fs.removed))


class SmeshStreamTests(unittest.TestCase):
    def test_empty_readline_returns_zeros(self):
        stream = mod.SmeshStream([], [])
        self.assertEquals('0 3 0 0\n', stream.readline())
        for expected in ['0 0\n', '0\n', '0\n', '']:
            self.assertEquals(expected, stream.readline())

    def test_vertices(self):
        vertex_1 = (1,2,3)
        vertex_2 = (4,5,6)
        stream = mod.SmeshStream([vertex_1, vertex_2], [])
        for expected in ['2 3 0 0\n', '0 1 2 3\n', '1 4 5 6\n']:
            self.assertEquals(expected, stream.readline())
        for expected in ['0 0\n', '0\n', '0\n', '']:
            self.assertEquals(expected, stream.readline())

    def test_faces(self):
        face_1 = (1, 2, 3)
        face_2 = (4, 5, 6)
        stream = mod.SmeshStream([], [face_1, face_2])
        self.assertEquals('0 3 0 0\n', stream.readline())
        for expected in ['2 0\n', '3 1 2 3\n', '3 4 5 6\n']:
            self.assertEquals(expected, stream.readline())
        for expected in ['0\n', '0\n', '']:
            self.assertEquals(expected, stream.readline())

    def test_quads(self):
        quad = (1, 2, 3, 4)
        stream = mod.SmeshStream([], [quad])
        self.assertEquals('0 3 0 0\n', stream.readline())
        for expected in ['1 0\n', '4 1 2 3 4\n']:
            self.assertEquals(expected, stream.readline())
        for expected in ['0\n', '0\n', '']:
            self.assertEquals(expected, stream.readline())

    def test_iterable(self):
        stream = mod.SmeshStream([], [])
        lines = [line for line in stream]
        expected = ['0 3 0 0\n', '0 0\n', '0\n', '0\n']
        self.assertListEqual(expected, lines)

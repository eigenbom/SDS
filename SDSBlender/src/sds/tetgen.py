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

from os import path
import datetime, os, io, tempfile

from sds import core

__all__ = ["TetrahedralizeError", "TetrahedralizedMesh", "Tetgen", 
           "SmeshStream"]

class TetrahedralizeError(Exception):
    @property
    def inner(self):
        """The exception that caused the error."""
        return self._inner

    @property
    def message(self):
        """The error message."""
        return self._message

    @property
    def quality(self):
        """The desired tetrahedra quality."""
        return self._quality

    @property
    def area(self):
        """The desired tetrahedra area."""
        return self._area

    @property
    def mesh(self):
        """The mesh to be tetrahedralized."""
        return self._mesh

    def __init__(self, mesh,  quality=None, area=None, e=None, message=None):
        """Create a exception object for a specific mesh."""
        if e and not message:
            message = str(e)
        if not message:
            message = "Error occured while generating tetrahedralized mesh."
        self._mesh = mesh
        self._quality = quality
        self._area = area
        self._inner = e
        self._message = message

    def __str__(self):
        return self._message

class TetrahedralizedMesh():
    """Handle for the tetgen output files.

    The mesh contains the filename prefix all the output files of a tetgen run.
    """
    def __init__(self, filename):
        """Create a handler.

        filename:   string with the prefix including the full path

        Note that .1 is appended by this method.
        """
        self._filename = path.splitext(filename)[0] + ".1"
        self._total_vertices = None

    @property
    def commonprefix(self):
        """The prefix shared by all files."""
        return self._filename

    @property
    def total_vertices(self):
        """The total amount of vertices in the mesh."""
        if self._total_vertices is None:
            self._total_vertices = self._calculate_total_vertices()
        return self._total_vertices

    def destroy(self):
        """Remove all files from the file system.

        This method is called to properly destroy the object.
        """
        extensions = ('node', 'neigh', 'ele', 'face')
        for ext in extensions:
            os.remove("%s.%s" % (self._filename, ext))

    def _calculate_total_vertices(self):
        try:
            node_file = open(self._filename + ".node")
            total_vertices = int(node_file.readline().split()[0])
            assert total_vertices > 0
        finally:
            node_file.close()
        return total_vertices

class Tetgen:
    """Factory for creating tetrahedralized meshes from file streams.

    This class depends on tetgen being installed and available through
    the utilrunner. Currently only Smesh model files are supported.
    """

    class _TetgenFS:
        """Facade for file system operations."""
        open = io.open
        remove = os.remove

    def __init__(self, fs_operators=None, runner=None):
        """Create a new instance of the tetgen util.

        fs_operators:   util with file system operation methods
        runner:         util for running programs

        Both parameters default to python implementations.
        """
        if fs_operators is None:
            fs_operators = self._TetgenFS()
        if runner is None:
            runner = core.SDSUtil
        self.fs = fs_operators
        self.runner = runner

    def _write_stream(self, stream, filename):
        with self.fs.open(filename, mode='w') as output:
            for line in stream:
                output.write(line)

    def _run_tetgen(self, filename, quality, area):
        arguments = [filename,
                     '-p', 
                     '-n',
                     '-Y',
                     '-Q']
        if quality:
            arguments.append('-q%s' % quality)
        if area:
            arguments.append('-a%s' % area)
        self.runner.tetgen(arguments)

    def _temp_filename(self, suffix):
        now = datetime.datetime.today()
        return now.strftime('simulation-%Y%m%d-%H%M' + suffix)

    def from_file(self, stream, quality=None, area=None):
        """Convert a Smesh stream into a tetrahedralized mesh.

        stream:     iterator over every line in the model format
        quality:    float containing the quality coefficient being passed to
                    tetgen or None
        area:       float containing the maximum tetrahedra area or None
        """
        tempfile_name = self._temp_filename('.smesh')
        tempfile_dir = tempfile.gettempdir()
        tempfile_abs = os.path.join(tempfile_dir, tempfile_name)
        try:
            self._write_stream(stream, tempfile_abs)
            self._run_tetgen(tempfile_abs, quality, area)
        except ValueError as v:
            raise TetrahedralizeError(stream, quality, area, e=v)
        except OSError as ose:
            raise TetrahedralizeError(stream, quality, area, e=ose, 
                                      message="Failed to run tetgen executable")
        finally:
            self.fs.remove(tempfile_abs)
        mesh = TetrahedralizedMesh(tempfile_abs)
        return mesh

class SmeshStream:
    """A smesh model stream, compatible with tetgen.

    This is an implementation of the format described on: 
    http://tetgen.berlios.de/fformats.smesh.html
    """
    def __init__(self, vertices, faces):
        """Create a new smesh stream.

        Both vertices as faces must be lists of tuples with their respective
        XYZ-coordinates and vertices.
        """
        vertices = self._vertex_generator(vertices)
        faces = self._face_generator(faces)
        appendices = self._zero_generator(2)
        self._parts = [vertices, faces, appendices]

    def _zero_generator(self, count):
        for i in range(count):
            yield '0'

    def _vertex_generator(self, vertices):
        yield '{} 3 0 0'.format(len(vertices))
        for index, vertex in enumerate(vertices):
            rounded_coordinates = [round(co, 6) for co in vertex]
            yield '{} {} {} {}'.format(index, *rounded_coordinates)

    def _face_generator(self, faces):
        yield '{} 0'.format(len(faces))
        for face in faces:
            vertices = ' '.join(str(v) for v in face)
            yield '{} {}'.format(len(face), vertices)

    def _readline(self):
        try:
            if self._parts:
                return next(self._parts[0]) + '\n'
            else:
                return None
        except StopIteration:
            self._parts.pop(0)
        return self.readline()

    def readline(self):
        """Read one line."""
        line = self._readline()
        return line if line else ''

    def __iter__(self):
        """Get a line iterator.

        This iterator will return all lines making up the stream. Note that
        this method returns the object instance, so the iterator is available
        only once.
        """
        return self

    def __next__(self):
        """Get the next line.

        This will raise a StopIteration exception when no more lines can be
        read.
        """
        line = self._readline()
        if not line:
            raise StopIteration()
        return line

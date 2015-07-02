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

import unittest

try:
    import bpy
    _noblender = False
except ImportError:
    _noblender = True

from blender.blender250.utils import PaintInspector

class PaintInspectorTests(unittest.TestCase):
    def assertVertexPaint(self, painted_vertices, paint_inspector, precision=None):
        for vertex in painted_vertices:
            expected = painted_vertices[vertex]
            actual = paint_inspector[vertex]
            if not precision:
                self.assertTupleEqual(expected, actual,
                                      "Paint for '%s' does not match: %s, "
                                      "expected: %s" % (vertex, actual, 
                                                        expected))
            else:
                for e, a in zip(expected, actual):
                    self.assertAlmostEquals(e, a, places=precision)

    def create_paint_inspector(self, paint_values, faces):
        vertex_names = [str(i) for i in range(len(paint_values))]
        vertices = list(vertex_names)
        colored_faces = []
        for face in faces:
            color_face = [paint_values[i] for i in face]
            colored_faces.append(color_face)
        painted_vertices = dict(zip(vertex_names, paint_values))
        return painted_vertices, PaintInspector(vertices, faces, colored_faces)

    def test_map_vertex_to_color_two_separate_triangles(self):
        painted_vertex_i = [(0,0,0), (0.1,0.2,0.3), (0.3,0.1,0.2), 
                            (1,1,1), (0.5,0.5,0.5), (0.6,0.6,0.6)]
        faces = [[0, 1, 2], [3, 4, 5]]
        painted, pi = self.create_paint_inspector(painted_vertex_i, faces)
        self.assertVertexPaint(painted, pi)

    def test_map_vertex_to_color_two_joined_triangles(self):
        painted_vertex_i = [(0,0,0), (0.1,0.2,0.3), (0.5,0.5,0.5), (1,1,1)]
        faces = [[0, 1, 2], [0, 2, 3]]
        painted, pi = self.create_paint_inspector(painted_vertex_i, faces)
        self.assertVertexPaint(painted, pi)

    def test_map_same_vertex_different_values_takes_highest(self):
        vertices = ['a', 'b', 'c', 'd']
        faces = [[0, 1, 2], [0, 2, 3]]
        paint_faces = [[(0.1,0.1,0.1), (0.2,0.2,0.2), (0.7,0.7,0.7)],
                       [(0.0,0.1,0.3), (0.7,0.7,0.7), (1.0,1.0,1.0)]]
        painted = {'a':(0.1,0.1,0.3), 'b':(0.2,0.2,0.2), 'c':(0.7,0.7,0.7),
                   'd':(1.0,1.0,1.0)}
        pi = PaintInspector(vertices, faces, paint_faces)
        self.assertVertexPaint(painted, pi)

    def test_map_isolated_vertex_contains_no_morphogens(self):
        vertices = ['a', 'b', 'c', 'd']
        faces = [[0, 2, 3]]
        paint_faces = [[(0.1,0.1,0.1), (0.2,0.2,0.2), (0.7,0.7,0.7)]]
        painted = {'a':(0.1,0.1,0.1), 'b':(0,0,0), 'c':(0.2,0.2,0.2),
                   'd':(0.7,0.7,0.7)}
        pi = PaintInspector(vertices, faces, paint_faces)
        self.assertVertexPaint(painted, pi)

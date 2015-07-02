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

from sds import geometry as mod

class MockVector:
    def __init__(self, components=(0,0,0)):
        self._components = components
    def x(self):
        return self._components[0]
    def y(self):
        return self._components[1]
    def z(self):
        return self._components[2]

class VertexTests(unittest.TestCase):
    class MockVertex:
        def __init__(self, position=None, surface=False):
            self._x = MockVector() if position is None else position
            self._surface = surface
        def x(self):
            return self._x
        def surface(self):
            return self._surface

    def tests_create_copies_position(self):
        mockvector = MockVector((1,2,3))
        mockvertex = self.MockVertex(mockvector)
        vertex = mod.Vertex.create(mockvertex)
        self.assertTupleEqual((1,2,3), vertex.position)

    def tests_create_copies_surface(self):
        mockvertex = self.MockVertex(surface=True)
        vertex = mod.Vertex.create(mockvertex)
        self.assertTrue(vertex.on_meshsurface)

class FaceTests(unittest.TestCase):
    class MockFace:
        def __init__(self, vertices):
            self._vertices = vertices
        def cv(self, index):
            return self._vertices[index]
    def tests_create_replaces_vertices_with_indices(self):
        vertex_dict = {'verta':0, 'vertb':1, 'vertc':2}
        core_face = self.MockFace(['vertc', 'verta', 'vertb'])
        face = mod.Face.create(core_face, vertex_dict)
        self.assertTupleEqual((2, 0, 1), face.vertices)

class SurfaceMeshBuilderTests(unittest.TestCase):
    class MockMesh:
        def __init__(self, vertices=None, faces=None):
            self._verts = vertices
            self._faces = faces
        def outerFaces(self):
            return self._faces
        def vertices(self):
            return self._verts
    class MockVertexFactory:
        def __init__(self):
            self.verts = []
            self.output = []
        def create(self, vertex):
            self.verts.append(vertex)
            return self.output.pop(0)
    class MockFaceFactory:
        def __init__(self):
            self.faces = []
            self.vertexlist = []
        def create(self, face, vertexlist):
            self.faces.append(face)
            self.vertexlist.append(vertexlist)
            return str(face)
    class MockVertex:
        def __init__(self, onsurface):
            self.on_meshsurface = onsurface
    def setUp(self):
        self.vert_fact = self.MockVertexFactory()
        self.face_fact = self.MockFaceFactory()
        self.surf_fact = mod.SurfaceMeshBuilder(self.vert_fact, self.face_fact)
        self.vertices_core = ['a', 'b', 'c', 'd']
        self.vertices_real = [self.MockVertex(True), 
                              self.MockVertex(False), 
                              self.MockVertex(True), 
                              self.MockVertex(False)]
        self.vert_fact.output = list(self.vertices_real)
        self.faces_core = [1,2,3]
        self.mock_mesh = self.MockMesh(self.vertices_core, self.faces_core)
        self.mesh = self.surf_fact.create(self.mock_mesh)
        
    def tests_create_only_adds_surface_vertex(self):
        self.assertListEqual(self.vertices_core, self.vert_fact.verts)
        surface_verts = [v for v in self.vertices_real if v.on_meshsurface]
        self.assertListEqual(surface_verts, self.mesh.vertices)

    def test_create_uses_surface_vertices_as_list_for_face(self):
        vertexmap = {'a':0, 'c':1}
        for l in self.face_fact.vertexlist:
            self.assertDictEqual(vertexmap, l)

    def test_create_adds_outer_faces(self):
        faces = [str(f) for f in self.faces_core]
        self.assertListEqual(faces, self.mesh.faces)

class CoreMeshFactoryTests(unittest.TestCase):
    class MockMeshTools:
        def Load(self, tetra_mesh):
            self.input = tetra_mesh
            return "MESH"
    class MockTetraMesh:
        def __init__(self, prefix):
            self.commonprefix = prefix
    def test_create_returns_core_mesh(self):
        mmt = self.MockMeshTools()
        mf = mod.CoreMeshFactory(mmt)
        input = self.MockTetraMesh("INPUT")
        mesh = mf.create_from_tetramesh(input)
        self.assertEquals(mmt.input, "INPUT")
        self.assertEquals(mesh, "MESH")

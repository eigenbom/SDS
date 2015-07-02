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

from sds import core

class Vertex:
    @staticmethod
    def create(vertex):
        """Create a vertex from a sds vertex."""
        core_vector = vertex.x()
        position = (core_vector.x(), core_vector.y(), core_vector.z())
        surface = vertex.surface()
        vertex = Vertex(position, surface)
        return vertex

    @property
    def position(self):
        """Position of the vertex in 3D."""
        return self._position

    @property
    def on_meshsurface(self):
        """Is the vertex on the surface of the mesh."""
        return self._on_meshsurface

    def __init__(self, position, surface):
        self._position = position
        self._on_meshsurface = surface

class Face:
    @staticmethod
    def create(face, vertexmap):
        """Create a face from a sds face.

        Uses a vertex map to convert the face vertices to indices.
        """
        core_vertices = (face.cv(i) for i in range(3))
        vertices = [vertexmap[v] for v in core_vertices]
        return Face(vertices)   

    @property
    def vertices(self):
        """Indices of the vertices spanning the surface."""
        return self._vertices

    def __init__(self, vertices):
        self._vertices = tuple(vertices)

class SurfaceMeshBuilder:
    def __init__(self, vertex_factory=Vertex, face_factory=Face):
        self.vertex_factory = vertex_factory
        self.face_factory = face_factory
    def create(self, core_mesh):
        """Create a mesh from a sds mesh.
        
        This method uses only the surface vertices and faces to construct the
        mesh.
        """
        mesh = Mesh()
        core_surface_vertices = {}
        for core_vertex in core_mesh.vertices():
            vertex = self.vertex_factory.create(core_vertex)
            if vertex.on_meshsurface:
                mesh.vertices.append(vertex)
                core_surface_vertices[core_vertex] = len(core_surface_vertices)
        for core_face in core_mesh.outerFaces():
            face = self.face_factory.create(core_face, core_surface_vertices)
            mesh.faces.append(face)
        return mesh

class TetrahedralMeshBuilder:
    def __init__(self, vertex_factory=Vertex, face_factory=Face):
        self.vertex_factory = vertex_factory
        self.face_factory = face_factory

    def create(self, core_mesh):
        """Create a mesh from a sds mesh.
        
        This method creates all the individual tetrahedra in the mesh.
        """
        mesh = Mesh()
        core_vertices = []
        for core_tetra in core_mesh.tetras():
            current = len(mesh.vertices)
            core_vertices = (core_tetra.cv(i) for i in range(4))
            vertices = [self.vertex_factory.create(v) for v in core_vertices]
            mesh.vertices.extend(vertices)
            facemapping = [[0,2,1], [0,1,3], [1,2,3], [2,0,3]]
            for facemap in facemapping:
                vertexindices = [vind + current for vind in facemap]
                face = Face(vertexindices)
                mesh.faces.append(face)
        return mesh

class Mesh:
    @property
    def vertices(self):
        """List of mesh vertices."""
        return self._vertices
    @property
    def faces(self):
        """List of mesh faces."""
        return self._faces
    def __init__(self):
        self._vertices = list()
        self._faces = list()

class CoreMeshFactory:
    @staticmethod
    def create():
        return CoreMeshFactory(core.MeshTools)

    def __init__(self, meshtools):
        self._meshtools = meshtools

    def create_from_tetramesh(self, tetramesh):
        """Create a sds mesh from a tetrahedralized mesh."""
        prefix = tetramesh.commonprefix
        mesh = self._meshtools.Load(prefix)
        return mesh

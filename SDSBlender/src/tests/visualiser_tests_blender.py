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

from nose.tools import with_setup, assert_almost_equal
import mox
import decimal
from sds import core

try:
    import bpy, visualiser as v
except:
    # Only runnable from within Blender
    __test__ = False


def _setup_one_tetrahedra():
    global organism, group
    organism = core.OrganismTools.loadOneTet()
    v.visualise_organism(organism)
    group = bpy.data.groups['organism']


def _destroy_one_tetrahedra():
    global organism, group
    _destroy_group(group)
    organism = None
    group = None


def _destroy_group(group):
    for object in group.objects:
        mesh = bpy.data.meshes[object.name]
        bpy.data.scenes.active.objects.unlink(object)
        object.name = "destroyable"
        mesh.name = "destroyable"
    bpy.data.groups.unlink(group)


def _from_opengl(organism_vertex):
    position = organism_vertex.x()
    return (-position.x(),position.z(),position.y())


def test_visualise_organism_without_organism():
    v.visualise_organism(None, group="test_organism")
    group = bpy.data.groups['test_organism']
    assert len(group.objects) == 0
    _destroy_group(group)


def test_visualise_organism_default_group():
    v.visualise_organism(None)
    group = bpy.data.groups['organism']
    _destroy_group(group)


def test_visualise_organism_without_tetrahedrae():
    organism = core.Organism()
    v.visualise_organism(organism)
    group = bpy.data.groups['organism']
    assert len(group.objects) == 0
    _destroy_group(group)


@with_setup(_setup_one_tetrahedra, _destroy_one_tetrahedra)
def test_visualise_organism_with_one_tetrahedra_name():
    global group
    assert len(group.objects) == 1, "No objects in group"
    object = group.objects[0]
    assert object.name == "organism_0", (
           "Object name differs: %s, expected: %s" % 
            (object.name, "organism_0"))


@with_setup(_setup_one_tetrahedra, _destroy_one_tetrahedra)
def test_visualise_organism_with_one_tetrahedra_vertices():
    global group, organism
    mesh = group.objects[0].getData()
    for index, vertex in enumerate(_tetra_verts(organism.mesh().tetras()[0])):
        assert_almost_equal(vertex[0], mesh.verts[index].co[0])
        assert_almost_equal(vertex[1], mesh.verts[index].co[1])
        assert_almost_equal(vertex[2], mesh.verts[index].co[2])


def _tetra_verts(tetra):
    for i in range(4):
        yield _from_opengl(tetra.cv(i))


def _tetra_faces(tetra):
    vertices = list(_tetra_verts(tetra))
    faces = {1:(0,2,1), # bottom face
             2:(0,1,3), # left face
             3:(1,2,3), # right face
             4:(2,0,3)} # back face
    for face in faces:
        indices = faces[face]
        yield [vertices[i] for i in indices]


def _same_float(float_a, float_b):
    a = float(str(float_a)[:7])
    b = float(str(float_b)[:7])
    return a == b


def _same_vertex(vertex_a, vertex_b):
    return  (_same_float(vertex_a[0], vertex_b[0]) and
             _same_float(vertex_a[1], vertex_b[1]) and
             _same_float(vertex_a[2], vertex_b[2]))


def _same_face(tetra_face, mesh_face):
    return (_same_vertex(tetra_face[0], mesh_face[0].co) and
            _same_vertex(tetra_face[1], mesh_face[1].co) and
            _same_vertex(tetra_face[2], mesh_face[2].co))


@with_setup(_setup_one_tetrahedra, _destroy_one_tetrahedra)
def test_visualise_organism_with_one_tetrahedra_faces():
    global group, organism
    mesh = group.objects[0].getData(mesh=True)
    tetra = organism.mesh().tetras()[0]
    tetra_faces = [face for face in _tetra_faces(tetra)]
    mesh_faces = [face.verts for face in mesh.faces]
    for t_face in tetra_faces:
        match = [m_face for m_face in mesh_faces if _same_face(t_face, m_face)]
        assert match
        mesh_faces.remove(match[0])


class MockOrganism(core.Organism):
    class mock_mesh(core.Mesh):
        def __init__(self):
            real_organism = core.OrganismTools.loadOneTet()
            self._tetras = [real_organism.mesh().tetras()[0],
                            real_organism.mesh().tetras()[0]]

        def tetras(self):
            return self._tetras

    def __init__(self):
        self._mesh = self.mock_mesh()

    def mesh(self):
        return self._mesh


def test_visualise_organism_with_two_tetrahedra_objects_with_name():
    mock_organism = MockOrganism()
    v.visualise_organism(mock_organism)
    group = bpy.data.groups['organism']
    tetra_1 = group.objects[0]
    tetra_2 = group.objects[1]
    assert tetra_1.name == "organism_0", "Unexpected name: '%s'" % tetra_1.name
    assert tetra_2.name == "organism_1", "Unexpected name: '%s'" % tetra_2.name
    _destroy_group(group)


def test_visualise_organism_with_two_tetrahedra_objects_vertices():
    mock_organism = MockOrganism()
    v.visualise_organism(mock_organism)
    group = bpy.data.groups['organism']
    for index, tetra in enumerate(mock_organism.mesh().tetras()):
        mesh = group.objects[index].getData(mesh=True)
        for index, vertex in enumerate(_tetra_verts(tetra)):
            assert_almost_equal(vertex[0], mesh.verts[index].co[0])
            assert_almost_equal(vertex[1], mesh.verts[index].co[1])
            assert_almost_equal(vertex[2], mesh.verts[index].co[2])
    _destroy_group(group)


def test_visualise_organism_with_two_tetrahedra_objects_faces():
    mock_organism = MockOrganism()
    v.visualise_organism(mock_organism)
    group = bpy.data.groups['organism']
    for index, tetra in enumerate(mock_organism.mesh().tetras()):
        mesh = group.objects[index].getData(mesh=True)
        tetra_faces = [face for face in _tetra_faces(tetra)]
        mesh_faces = [face.verts for face in mesh.faces]
        for t_face in tetra_faces:
            match = [m_face for m_face in mesh_faces if _same_face(t_face, m_face)]
            assert match
            mesh_faces.remove(match[0])
    _destroy_group(group)

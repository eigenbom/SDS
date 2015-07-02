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

import Blender, bpy

def _from_opengl(organism_vertex):
    """Converts a vertex to blender coordinates."""
    position = organism_vertex.x()
    return (-position.x(),position.z(),position.y())


def _create_mesh_in_group(group):
    """Creates an empty mesh object in a group."""
    name = group.name
    number = len(group.objects)
    mesh_name = '%s_%d' % (name, number)
    mesh = Blender.Mesh.New(mesh_name)
    object = bpy.data.scenes.active.objects.new(mesh, mesh_name)
    group.objects.link(object)
    return mesh


def _add_tetra_to_mesh(tetra, mesh):
    """Adds a tetrahedra to a mesh object"""
    vertices = [_from_opengl(tetra.cv(i)) for i in range(4)]
    mesh.verts.extend(vertices)
    faces = [[0,2,1], [0,1,3], [1,2,3], [2,0,3]]
    mesh.faces.extend(faces)

def _visualise_organism_mesh(mesh, group):
    for tetra in mesh.tetras():
        mesh = _create_mesh_in_group(group)
        _add_tetra_to_mesh(tetra, mesh)


def visualise_organism(organism, group="organism"):
    """Renders all the individual tetrahedrae of a given organism."""
    container = Blender.Group.New(group)
    if organism and organism.mesh():
        mesh = organism.mesh()
        _visualise_organism_mesh(mesh, container)

from sds import core

if __name__ == '__main__':
    assert False, "Simulation file needed"
    loader = core.SimulationLoader('simulation-20091209-1209.1.cfg')
    m = None

    assert loader.loadFrameData()
    assert loader.hasSegment("mesh")

    m = core.Mesh()
    ml = core.MeshSegmentIO(m)
    loader.initialiseSegmentLoader(ml)
    loader.loadSegment(ml)

    container = Blender.Group.New("organism")
    _visualise_organism_mesh(m, container)

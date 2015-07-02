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

import bpy
import os, tempfile, contextlib

from sdsio import IMesh, IOrganism, IOutput

from sds.simulation import MeshFrameBuilder
from sds.geometry import SurfaceMeshBuilder
from sds.tetgen import Tetgen

from .utils import Utils, PaintInspector

class BlenderMesh(IMesh):
    @staticmethod
    def create_from_object(object, quality=None, area=None):
        """Create a new Blender mesh.

        This factory method will also apply the transformation matrix and
        Blender-to-OpenGL-axis matrix to the resulting mesh.
        """
        converted_data = object.data.copy()
        converted_data.transform(object.matrix)
        Utils.apply_opengl_conversion(converted_data)
        return BlenderMesh(converted_data, quality, area)

    def __init__(self, mesh, quality=None, area=None):
        self._mesh = mesh
        self._quality = quality
        self._area = area

    @contextlib.contextmanager
    def get_tetgen_format(self):
        """Convert mesh object to tetgen format."""
        stream = Utils.mesh_to_smesh(self._mesh)
        mesh = Tetgen().from_file(stream, self._quality, self._area)
        yield mesh
        mesh.destroy()


class Model(IOrganism):
    def __init__(self, organism, scene):
        self._organism = organism
        self._scene = scene

    def get_process_model_configuration(self):
        index = int(self._organism.data.sds_processmodel_data)
        if index == 0:
            return None
        name = Utils.text_list()[index]
        lines = (l.line for l in bpy.data.texts[name].lines)
        string = '\n'.join(lines)
        return string 

    def get_mesh(self):
        mesh = self._organism.data
        quality = mesh.sds_tetgen_quality
        apply_area = mesh.sds_tetgen_restrict_area 
        area = mesh.sds_tetgen_area if apply_area else None
        return BlenderMesh.create_from_object(self._organism, quality, area)

    def get_index_paint(self):
        """Gets the paint values.

        The key is the vertex index, the value a tuple with the RGB values.
        Returns None when not vertex paint is applied.
        """
        if not self._organism.data.vertex_colors:
            return None
        pi = PaintInspector.load(self._organism.data)
        values = dict()
        for index, vertex in enumerate(self._organism.data.verts):
            values[index] = pi[vertex]
        return values
    

class TemplatedFrameBuilder(MeshFrameBuilder):
    def __init__(self, factory, materials):
        MeshFrameBuilder.__init__(self, factory)
        self._materials = materials

    def create_frame(self, simulation):
        """Create a frame based on the current simulation state.

        Every frame will store a copy of the templated materials.
        """
        frame = MeshFrameBuilder.create_frame(self, simulation)
        frame.materials = self._materials
        return frame


class BlenderOutput(IOutput):
    def __init__(self, organisms, scene):
        self._organism_bases = organisms
        self._scene = scene

    def _build_mesh(self, mesh, frame):
        mesh.add_geometry(len(frame.vertices), 0, len(frame.faces))
        for blender_v,sds_v  in zip(mesh.verts, frame.vertices):
            blender_v.co = sds_v.position
        for blender_f, sds_f in zip(mesh.faces, frame.faces):
            blender_f.verts = sds_f.vertices
        Utils.apply_opengl_conversion(mesh)
        mesh.update()

    def _animate_object(self, object, frame_num):
        for frame, hidden in zip((1, frame_num, frame_num + 1), 
                                (True, False, True)):
            self._scene.set_frame(frame)
            object.restrict_render = hidden
            object.restrict_view = hidden
            object.keyframe_insert("restrict_render")
            object.keyframe_insert("restrict_view")

    def render_mesh(self, number, frame, materials):
        name = "SimFrame{:03}".format(number)
        mesh = bpy.data.meshes.new(name)
        object = bpy.data.objects.new(name, mesh)
        object.data = mesh
        self._scene.objects.link(object)
        self._build_mesh(mesh, frame)
        self._animate_object(object, number)
        for name in materials:
            material = bpy.data.materials[name]
            mesh.add_material(material)
        # At this point, the object will have a rotation set (unclear why)
        # This line of code will reset this
        object.rotation_quaternion = [1, 0, 0, 0]

    def render_frame(self, frame):
        """Render a frame in Blender."""
        for mesh, materials in zip(frame.meshes, frame.materials):
            self.render_mesh(frame.number, mesh, materials)

    def create_frame_builder(self):
        """Return a frame builder."""
        mesh_factory = SurfaceMeshBuilder()
        materials = [[m.name for m in o.data.materials] 
                     for o in self._organism_bases]
        return TemplatedFrameBuilder(mesh_factory, materials)

    def log_error(self, error_msg):
        print(error_msg)

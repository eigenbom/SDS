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

import bpy, Mathutils

from sds.tetgen import SmeshStream

class Utils:
    @staticmethod
    def apply_opengl_conversion(mesh):
        matrix_1 = [-1, 0, 0]
        matrix_2 = [ 0, 0, 1]
        matrix_3 = [ 0, 1, 0]
        matrix = Mathutils.Matrix(matrix_1, matrix_2, matrix_3)
        matrix.resize4x4()
        mesh.transform(matrix)

    @staticmethod
    def _generate_enumerable_list(bpy_collection, criteria=None):
        if not criteria:
            criteria = lambda o: True
        objects = (o for o in bpy_collection if criteria(o))
        all_objects = [(index, objects.name)
                      for index, objects in enumerate(objects, 1)]
        all_objects.append((0, "None"))
        return dict(all_objects)

    @staticmethod
    def mesh_list():
        criteria = lambda o: o.type == 'MESH'
        return Utils._generate_enumerable_list(bpy.data.objects, criteria)

    @staticmethod
    def text_list():
        return Utils._generate_enumerable_list(bpy.data.texts)

    @staticmethod
    def mesh_to_smesh(mesh):
        vertices = [tuple(v.co) for v in mesh.verts]
        faces = [tuple(f.verts) for f in mesh.faces]
        return SmeshStream(vertices, faces)


class PaintInspector:
    @staticmethod
    def load(mesh):
        vertices = list(mesh.verts)
        faces = [list(face.verts) for face in mesh.faces]
        paint_faces_raw = mesh.vertex_colors[0].data
        paint_faces = [(tuple(f.color1), tuple(f.color2), 
                        tuple(f.color3), tuple(f.color4)) 
                       for f in paint_faces_raw]
        pi = PaintInspector(vertices, faces, paint_faces)
        return pi

    def _merge_colors(self, update):
        for index in update:
            if index in self._colors:
                old = self._colors[index]
                new = update[index]
                merge = tuple(max(o, n) for o,n in zip(old, new))
                update[index] = merge
        self._colors.update(update)

    def __init__(self, vertices, faces, colored_faces):
        self._keys = vertices
        self._colors = dict()
        for mesh_face, paint_face in zip(faces, colored_faces):
            update = dict(zip(mesh_face, paint_face))
            self._merge_colors(update)

    def keys(self):
        return list(self._keys)

    def __getitem__(self, index):
        try:			
            vert_index = self._keys.index(index)
            return self._colors[vert_index]
        except KeyError:
            return (0, 0, 0)


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

import tempfile, os
from os import path

from nose.tools import *

try:
    import Blender, bpy, ply_animation
except:
    # Only runnable from within Blender
    __test__ = False


def generate_tetrahedra(filename):
    filename = os.path.split(filename)[1]
    name = os.path.splitext(filename)[0]
    vertices = [[-1, -1, -1], [1, -1, -1], [0, 1, -1], [0, 0, 1]]
    faces = [[2,1,0], [0,1,3], [1,2,3], [2, 0, 3]]
    tetra = bpy.data.meshes.new(name)
    tetra.verts.extend(vertices)
    tetra.faces.extend(faces)
    scn = bpy.data.scenes.active
    scn.objects.new(tetra)
    

def replace_mesh_load():
    """Replaces the mesh load function with a stub"""
    global __ply_backup
    __ply_backup = ply_animation.load_mesh
    ply_animation.load_mesh = generate_tetrahedra
    scn = bpy.data.scenes.active
    for ob in scn.objects:
        scn.objects.unlink(ob)


def restore_mesh_load():
    """Restores the mesh load function with a stub"""
    global __ply_backup
    ply_animation.load_mesh = __ply_backup


def write_ply_files():
    """Write several files with the extension ply to dir"""
    global __ply_files
    __ply_files = []
    for i in range(25):
        filename = "%02d.ply" % i
        __ply_files.append("%02d" % i)
        file = open(filename, 'w')
        file.write('\n')
        file.close()
    file = open('non_ply.blend', 'w')
    file.write('\n')
    file.close()


def remove_ply_files():
    """Write several files with the extension ply to dir"""
    global __ply_files
    for file in __ply_files:
        os.remove("%s.ply" % file)
    __ply_files = None
    os.remove('non_ply.blend')
        

@with_setup(write_ply_files, remove_ply_files)
def test_list_ply_returns_ply_files():
    global __ply_files

    files_in_dir = ply_animation.list_ply(os.getcwd())
    for ply_file in map(lambda f: "%s.ply" % f, __ply_files):
        assert ply_file in files_in_dir, ("File %s not found in dir %s" % 
                                           (ply_file, files_in_dir))
    assert len(files_in_dir) == len(__ply_files), ("Unequal amount of ply "
                                                   "found: %d, expected %d" %
                                                    (len(files_in_dir), 
                                                     len(__ply_files)))


@with_setup(replace_mesh_load, restore_mesh_load)
def test_mesh_loads_replaced_bits():
    ply_animation.load_mesh('filename')
    try:
        bpy.data.meshes['filename']
    except:
        assert False, "'filename' not found in meshes"


@with_setup(replace_mesh_load, restore_mesh_load)
@with_setup(write_ply_files, remove_ply_files)
def test_load_calls_mesh_load_for_every_file():
    global __ply_files
    ply_animation.load(os.getcwd())
    for file in __ply_files:
        try:
            bpy.data.meshes[file]
        except:
            assert False, "'%s' not found in meshes" % file

@with_setup(replace_mesh_load, restore_mesh_load)
@with_setup(write_ply_files, remove_ply_files)
def test_load_returns_list_with_new_objects():
    global __ply_files
    new_objs = ply_animation.load(os.getcwd())
    generated = __ply_files
    for object in new_objs:
        assert object.name in generated, ("%s not in %s" % 
                                             object.name, __ply_files)
        generated.remove(object.name)
    assert len(generated) == 0, "Not every object imported: %s" % generated


@with_setup(replace_mesh_load, restore_mesh_load)
@with_setup(write_ply_files, remove_ply_files)
def test_load_attaches_ipo_which_moves_between_layers():
    global __ply_files
    ply_animation.load(os.getcwd())
    for name in __ply_files:
        ob = bpy.data.objects[name]
        assert ob.getIpo() is not None, "No Ipo assigned to %s" % file
        ipo = ob.getIpo()
        assert ipo[Blender.Ipo.OB_LAYER] is not None, (
               "No ipo layer curve assigned to %s" % name) 
        points = ipo[Blender.Ipo.OB_LAYER].bezierPoints
        assert len(points) == 3, ("Knot point count invalid for %s: %d" % 
                                   (file, len(points)))
        assert points[0].pt == [0, 1]
        assert points[1].pt[1] == 2
        assert points[2].pt[1] == 1
        assert points[1].pt[0] == points[2].pt[0] - 1

@with_setup(replace_mesh_load, restore_mesh_load)
@with_setup(write_ply_files, remove_ply_files)
def test_load_places_objects_alternating_between_layers():
    global __ply_files
    ply_animation.load(os.getcwd())
    for name, key in zip(__ply_files, range(len(__ply_files))):
        key += 1
        ob = bpy.data.objects[name]
        ipo = ob.getIpo()
        points = ipo[Blender.Ipo.OB_LAYER].bezierPoints
        assert points[1].pt[0] == key, ("%s is in layer 2 at wrong time: %d, " 
                                        "expected %d" % 
                                         (name, points[1].pt[0], key)) 
        assert points[1].pt[0] == key, ("%s returns in layer 1 at wrong time: "
                                        "%d, expected %d" % 
                                         (name, points[1].pt[0], key)) 

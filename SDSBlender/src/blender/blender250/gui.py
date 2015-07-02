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

from .utils import Utils
from .io import BlenderOutput, Model, BlenderMesh

# Needed to pass some data between the View and the Operator
_callback = None
_scene_properties = None
_mesh_properties = None

class View:
    def __init__(self):
        self._build_gui()

    @property
    def callback(self):
        """Callback function to be called when simulation is requested."""
        global _callback
        return _callback

    @callback.setter
    def callback(self, function):
        global _callback
        _callback = function

    def _build_gui(self):
        global _scene_properties, _mesh_properties
        _scene_properties = self._register_scene_properties()
        _mesh_properties = self._register_mesh_properties()
        bpy.types.register(OBJECT_OT_sds)
        bpy.types.register(OBJECT_PT_sds_world)
        bpy.types.register(OBJECT_PT_sds)
        bpy.types.register(OBJECT_PT_sds_processmodel)
        bpy.types.register(OBJECT_PT_sds_tetrahedralise)

    def _log_property(self, container, func):
        def new_func(**kwargs):
            container.append(kwargs['attr'][4:])
            func(**kwargs)
        return new_func
        
    def _register_scene_properties(self):
        log = list()
        IntProperty = self._log_property(log, bpy.types.Scene.IntProperty)
        FloatProperty = self._log_property(log, bpy.types.Scene.FloatProperty)
        BoolProperty = self._log_property(log, bpy.types.Scene.BoolProperty)
        StringProperty = self._log_property(log, bpy.types.Scene.StringProperty)
        IntProperty(attr="sds_frame_step", name="Step",
                    description="Simulation steps per frame",
                    default=30, min=1)
        IntProperty(attr="sds_frame_total", name="Total",
                    description="Total number of frames to simulate",
                    default=100, min=1)
        FloatProperty(attr="sds_delta_t", name="dt",
                      description="Time per simulation step",
                      precision=3,
                      default=0.002, min=0.001)
        BoolProperty(attr="sds_collision",
                     name="Collision",
                     description="Perform collision detection")
        FloatProperty(attr="sds_gravity", name="Gravity",
                      description="Amount of gravity during simulation",
                      default=0)
        FloatProperty(attr="sds_density", name="Density",
                      description="Density of surroundings",
                      default=1)
        FloatProperty(attr="sds_viscosity", name="Viscosity",
                      description="Viscosity of fluid environment",
                      default=.1,
                      precision=3)										
        FloatProperty(attr="sds_kd", name="kD",
                      description="Edge spring stiffness",
                      default=2.5)
        FloatProperty(attr="sds_ksm", name="kSM",
                      description="Surface edge spring stiffness multiplier",
                      default=1.2)
        FloatProperty(attr="sds_kv", name="kV",
                      description="Tetrahedra spring stiffness",
                      default=3.0)
        FloatProperty(attr="sds_kdamp", name="kDamp",
                      description="Spring dampening",
                      default=0.5,
                      precision=3)										
        StringProperty(attr="sds_comments", name="Comments",
                       description="Additional comments about the simulation")
        BoolProperty(attr="sds_dump_initial", name="DEBUG: Output Initial",
                     description="Output initial state to disk (out.cfg/out.bin)"
                                 "for debugging without simulating")
        BoolProperty(attr="sds_dirty_mode", name="Dirty mode?", 
                     description="Let the simulation ignore some errors.")
        return log

    def _register_mesh_properties(self):
        log = list()
        EnumProperty = self._log_property(log, bpy.types.Mesh.EnumProperty)
        FloatProperty = self._log_property(log, bpy.types.Mesh.FloatProperty)
        BoolProperty = self._log_property(log, bpy.types.Mesh.BoolProperty)
        BoolProperty(attr="sds_active", name="Active object", default=False,
                      description="Involved in a SDS simulation.")
        EnumProperty(attr="sds_role", name="Role", 
                     items=[("0", "Static mesh", "0"),
                            ("1", "Organism", "1")],
                     description="The role played in a SDS simulation.")
        FloatProperty(attr="sds_tetgen_quality",
                      name="Tetgen quality", default=1.1,
                      description="The quality of the tetrahedralized mesh.")
        BoolProperty(attr="sds_tetgen_restrict_area",
                      name="Restrict tetrahedra volume", default=False,
                      description="Restrict the volume of the individual "
                                  "tetrahedras.")
        FloatProperty(attr="sds_tetgen_area",
                      name="Maximum tetrahedra area", default=1.1,
                      description="The maximum area of the individual "
                                  "tetrahedras in the organism.")
        return log
    

class SourceNotPresentError(Exception):
    @property
    def source(self):
        return self._source

    def __init__(self, source, message=None):
        if not message:
            message = source
        Exception.__init__(self, message)
        self._source = source
        

class GuiSettings:
    """Dictionary adapter for accessing Blender SDS settings.

    The adapter exposes the properties of a Scene and Mesh through the python
    dictionary interface.
    """
    @property
    def mesh_attributes(self):
        """Keys related to the Mesh object."""
        return self._mesh_attributes

    @property
    def scene_attributes(self):
        """Keys related to the Scene object."""
        return self._scene_attributes

    def __init__(self):
        self.mesh = None
        self.scene = None
        self._mesh_attributes = []
        self._scene_attributes = []

    def __getitem__(self, name):
        """Access the property value.

        The value name will be prefixed with 'sds_'.

        This method determines the owner of the requested property and returns
        its value. If the property cannot be found, an AttributeError is 
        raised. If the owner of the property cannot be found, a
        SourceNotPresentError is raised.
        """
        target = None
        if name in self.mesh_attributes:
            if not self.mesh:
                raise SourceNotPresentError("mesh")
            target = self.mesh
        if name in self.scene_attributes:
            if not self.scene:
                raise SourceNotPresentError("scene")
            target = self.scene
        if not target:
            raise AttributeError('Attribute \"%s\" not assigned to mesh or scene'%(name))
        return getattr(target, "sds_%s" % name)

    def get_world_bounds(self):
        index = int(self.scene.sds_world_bounds)
        if index == 0:
            return None
        name = Utils.mesh_list()[index]
        object = bpy.data.objects[name]
        mesh = object.data.copy()
        mesh.transform(object.matrix)
        Utils.apply_opengl_conversion(mesh)
        if len(mesh.verts) == 0:
            return None
        minimum = mesh.verts[0].co
        maximum = mesh.verts[0].co
        for vertex in mesh.verts:
            vertex = vertex.co
            minimum = [min(vertex[0], minimum[0]), 
                       min(vertex[1], minimum[1]), 
                       min(vertex[2], minimum[2])] 
            maximum = [max(vertex[0], maximum[0]), 
                       max(vertex[1], maximum[1]), 
                       max(vertex[2], maximum[2])] 
        return (minimum, maximum)

    def __str__(self):
        """Textual representation of the class."""
        object = "GuiSettings"
        return "<%s: mesh_attr=%s, scene_attr=%s>" % (
                object, str(self.mesh_attributes), str(self.scene_attributes))


class SceneButtonsPanel(bpy.types.Panel):
    bl_space_type = "PROPERTIES"
    bl_region_type = "WINDOW"
    bl_context = "scene"


class MeshButtonsPanel(bpy.types.Panel):
    bl_space_type = "PROPERTIES"
    bl_region_type = "WINDOW"
    bl_context = "data"

    def poll(self, context):
        return context.active_object and context.active_object.type == 'MESH'


class SDSMeshButtonsPanel(MeshButtonsPanel):
    def poll(self, context):
        if MeshButtonsPanel.poll(self, context):
            return context.active_object.data.sds_active
        else:
            return False


class OBJECT_PT_sds(MeshButtonsPanel):
    bl_label = "SDS"

    properties = ["role"]

    def draw_header(self, context):
        self.layout.prop(context.active_object.data, "sds_active", text="")

    def draw(self, context):
        mesh = context.active_object.data
        layout = self.layout
        for property in self.properties:
            text = property.capitalize()
            layout.prop(mesh, "sds_%s" % property, text=text)


class OBJECT_PT_sds_tetrahedralise(SDSMeshButtonsPanel):
    bl_label = "SDS Tetrahedralise"

    def draw_tetgen(self, layout, container):
        col = layout.column(align=True)
        col.prop(container, "sds_tetgen_restrict_area", text="Limit area")
        col.prop(container, "sds_tetgen_area", text="Area")
        col.prop(container, "sds_tetgen_quality", text="Quality")

    def poll(self, context):
        if SDSMeshButtonsPanel.poll(self, context):
            return context.active_object.data.sds_role == "1"
        else:
            return False

    def draw(self, context):
        mesh = context.active_object.data
        layout = self.layout
        self.draw_tetgen(layout, mesh)
        
class OBJECT_PT_sds_processmodel(SDSMeshButtonsPanel):
    bl_label = "SDS Process Model"

    def poll(self, context):
        if SDSMeshButtonsPanel.poll(self, context):
            return context.active_object.data.sds_role == "1"
        else:
            return False

    def draw(self, context):
        self.retrieve_texts(context)
        mesh = context.active_object.data
        layout = self.layout
        layout.prop(mesh, "sds_processmodel_data")

    def _create_enum(self, collection, attr, name, description, type):
        all = [(str(index), collection[index], str(index))
               for index in range(len(collection))]
        type.EnumProperty(attr=attr, name=name, description=description, 
                          items = all, default = '0')

    def retrieve_texts(self, context):
        texts = Utils.text_list()
        self._create_enum(texts, attr="sds_processmodel_data",
                          name="ProcessModel",
                          description="Choose a text region to set the process"
                                      " model",
                          type=bpy.types.Mesh)

class OBJECT_PT_sds_world(SceneButtonsPanel):
    bl_label = "SDS World"

    def draw(self, context):
        self.retrieve_meshes(context)
        self.retrieve_texts(context)
        layout = self.layout
        rd = context.scene
        layout.prop(rd, "sds_world_bounds")
        self.draw_time(layout, rd)
        layout.separator()
        self.draw_physics(layout, rd)
        layout.separator()
        layout.label(text="Debug:")
        layout.prop(rd, "sds_dump_initial")		
        layout.prop(rd, "sds_dirty_mode")		
        layout.separator()
        layout.operator("OBJECT_OT_sds", text="Simulate")

    def draw_time(self, layout, container):
        layout.label(text="Frames:")
        col = layout.column(align=True)
        col.prop(container, "sds_frame_total", text="Total")
        col.prop(container, "sds_frame_step", text="Step")
        col.prop(container, "sds_delta_t", text="dt")

    def draw_physics(self, layout, container):
        sub = layout.split()
        col = sub.column()
        col.label(text="Environment:")
        col.prop(container, "sds_collision")
        col.prop(container, "sds_gravity")
        col.prop(container, "sds_density")
        col.prop(container, "sds_viscosity")
        col = sub.column(align=True)
        col.label(text="Springs:")
        col.prop(container, "sds_kd")
        col.prop(container, "sds_ksm")
        col.prop(container, "sds_kv")
        col.prop(container, "sds_kdamp")

    def _create_enum(self, collection, attr, name, description, type):
        all = [(str(index), collection[index], str(index))
               for index in range(len(collection))]
        type.EnumProperty(attr=attr, name=name, description=description, 
                          items = all, default = '0')

    def retrieve_meshes(self, context):
        meshes = Utils.mesh_list()
        self._create_enum(meshes, attr="sds_world_bounds", 
                          name="Bounding mesh",
                          description="Choose a mesh to set the world "
                                      "boundaries",
                          type=bpy.types.Scene)
        
    def retrieve_texts(self, context):
        texts = Utils.text_list()
        self._create_enum(texts, attr="sds_processmodel_data",
                          name="ProcessModel",
                          description="Choose a text region to set the process"
                                      " model",
                          type=bpy.types.Mesh)


class OBJECT_OT_sds(bpy.types.Operator):
    bl_idname = "OBJECT_OT_sds"
    bl_label = "Run SDS Simulation"
    __doc__ = "Run SDS simulation with active object as organism"

    def get_settings(self, scene):
        global _scene_properties
        settings = GuiSettings()
        settings.scene_attributes.extend(_scene_properties)
        settings.scene = scene
        return settings

    def get_involved_meshes(self, objects):
        return (o for o in objects if o.type == 'MESH' and o.data.sds_active)

    def get_organisms(self, context):
        objects = [o for o in self.get_involved_meshes(context.main.objects)
                   if o.data.sds_role == "1"]
        scene = context.scene
        organisms = [Model(o, scene) for o in objects]
        output = BlenderOutput(objects, scene)
        return organisms, output

    def get_static_meshes(self, context):
        objects = (o for o in self.get_involved_meshes(context.main.objects)
                   if o.data.sds_role == "0")
        static_meshes = [BlenderMesh.create_from_object(o) for o in objects]
        return static_meshes

    def invoke(self, context, event):
        global _callback
        organisms, output = self.get_organisms(context)
        static_meshes = self.get_static_meshes(context)
        scene = context.scene
        settings = self.get_settings(scene)
        status = _callback(organisms, static_meshes, settings, output)
        if status.error:
            self.report('ERROR', status.message)
        return 'FINISHED'

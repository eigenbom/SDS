__author__= "Ben Porter"
__version__= "0.1"

__bpydoc__= """\
This script imports an SDS simulation into blender.
"""

import os, math, time
import Mathutils
import Geometry
from bpy.props import *

# HACK:For testing purposes
import sys
sys.path.append('.')

import blender
from sds import data, util, simulation, geometry, serialisation, core
import gc

from blender.blender250.io import *
from sdsio import IFrameBuilder
from sds.geometry import *
from sds.simulation import *

# custom frame builder for importing morphogen values as uv coordinates

class CustomMeshFrameBuilder(IFrameBuilder):
	def __init__(self, factory):
		self._meshfactory = factory
		self._frame_counter = 0
	def create_frame(self, simulation):
		"""Create a frame based on the current simulation state."""
		self._frame_counter += 1
		number = self._frame_counter		
		orgs = [self._meshfactory.create(o) for o in simulation.organisms]
		return Frame(number, orgs)
	   
class CustomSurfaceMeshBuilder:
	def __init__(self, vertex_factory=Vertex, face_factory=Face):
		self.vertex_factory = vertex_factory
		self.face_factory = face_factory
		
	def create(self, core_organism):
		"""Create a mesh from a sds organism.
		
		This method uses the surface vertices, morphogens, and faces to construct the
		mesh.
		"""
		core_mesh = core_organism.mesh()
		mesh = Mesh()
		core_surface_vertices = {}
		for core_cell in core_organism.cells():
			core_vertex = core_cell.v()
			cc = core_cell.getCellContents()
			m = cc.getMorphogen(cc.numMorphogens()-1) / core_cell.vol()
			#print(core_cell, m, core_cell.vol())
			vertex = self.vertex_factory.create(core_vertex)
			vertex.m = m
			if vertex.on_meshsurface:
				mesh.vertices.append(vertex)
				core_surface_vertices[core_vertex] = len(core_surface_vertices)
		for core_face in core_mesh.outerFaces():
			face = self.face_factory.create(core_face, core_surface_vertices)
			mesh.faces.append(face)
		return mesh
	   
	   
class CustomTemplatedFrameBuilder(CustomMeshFrameBuilder):
	def __init__(self, factory, materials):
		CustomMeshFrameBuilder.__init__(self, factory)
		self._materials = materials

	def create_frame(self, simulation):
		"""Create a frame based on the current simulation state.

		Every frame will store a copy of the templated materials.
		"""
		frame = CustomMeshFrameBuilder.create_frame(self, simulation)
		frame.materials = self._materials
		return frame
	   
class CustomBlenderOutput(IOutput):
	def __init__(self, organisms, scene):
		self._organism_bases = organisms
		self._scene = scene
		
	@staticmethod
	def set_uv(mesh,v,uv):
		vi = mesh.verts[:].index(v)
		faces = mesh.uv_textures[0].data
		for f,fi in zip(mesh.faces,range(len(mesh.faces))):		 
			if vi in f.verts:
				vfi = f.verts[:].index(vi)
				f2 = faces[fi]
				all = [f2.uv1,f2.uv2,f2.uv3][vfi]
				all[0] = uv[0]
				all[1] = uv[1]

	def _build_mesh(self, mesh, frame):
		mesh.add_geometry(len(frame.vertices), 0, len(frame.faces))
		mesh.add_uv_texture()
		for blender_v,sds_v in zip(mesh.verts, frame.vertices):
			blender_v.co = sds_v.position
		for blender_f, sds_f in zip(mesh.faces, frame.faces):
			blender_f.verts = sds_f.vertices
		for blender_v,sds_v in zip(mesh.verts, frame.vertices):
			CustomBlenderOutput.set_uv(mesh,blender_v,[sds_v.m,sds_v.m])
			
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
		mesh_factory = CustomSurfaceMeshBuilder()
		materials = [[m.name for m in o.data.materials] 
					 for o in self._organism_bases]
		return CustomTemplatedFrameBuilder(mesh_factory, materials)

	def log_error(self, error_msg):
		print(error_msg)

stasis = []

class simwrapper(object):
	def __init__(self, simulation):
		self.simulation = simulation
		
	def current_frame(self):
		print("current_frame")
		o = self.simulation.world().organism()
		if o is None:
			print("NONE")
			return None
		else:
			return o 
		
	@property
	def organisms(self):
		o = self.current_frame()
		if o is not None:
			return [o]
		else:
			print("simwrapper.organisms() = None!")
			return None

def load_sds(filepath, triple, context):
	#gc.set_debug(gc.DEBUG_LEAK)
	first,last,delta = triple
	print("loading: \"" + filepath + "\"")
	# try to load the sds file
	# iterate through and dump all the frames, one by one, using the framebuilder
	
	try:
		loader = core.SimulationLoader(filepath)
	except Exception as e:
		print(e)

	hdr = loader.simulationHeader()
	if not (loader.hasSegment("mesh") and loader.hasSegment("organism") and loader.hasSegment("processinfo")):	
		print("Simulation file does not have required segments.")
		return
	
	# frame_builder = output.create_frame_builder()
		
	#bo = BlenderOutput([bpy.data.add_object('MESH', 'foo')],context.scene)
	obj = bpy.context.active_object
	if obj is None:	
		bpy.ops.object.add(type='MESH')
		obj = bpy.context.active_object
	bo = CustomBlenderOutput([obj],context.scene)
	frame_builder = bo.create_frame_builder()
	
	#m = core.Mesh()
	#o = core.Organism(m)
					
	if loader.loadData():
		mTotalNumberOfFrames = loader.countFrames()
		if last==-1 or last>mTotalNumberOfFrames:
			last = mTotalNumberOfFrames		
		for frame in range(first,last+1,1+delta):			
			#loadFrame(context, loader, i, bo, frame_builder, hdr, hdr.processModel, m, o)			
			print("Frame %d: "%frame)
			fb = frame_builder
			pm = hdr.processModel
			if not loader.setFrame(frame):
				print("Cannot load frame %d"%frame)				
				return False
			else:		
				print("loading frame %d"%frame)
				frame_hdr = loader.currentFrame()
				
				#simulation.cleanup()
				print("clearing old data")
				print("clearing mesh")
				#m.clear();
				print("clearing org")
				#o.clear();
				m = core.Mesh()
				m.thisown = False
				o = core.Organism(m)
				o.thisown = False
								
				# load new data 
				print("loading mesh")		
				ml = core.MeshSegmentIO(m)
				loader.initialiseSegmentLoader(ml)
				loader.loadSegment(ml)		
		
				print("loading org")
				o.setMesh(m)
				oio = core.OrganismSegmentIO(o)
				loader.initialiseSegmentLoader(oio)
				loader.loadSegment(oio)
				print("loaded")		
						
				print("setting processmodel")
				o.setProcessModel(pm)
				print("set")
				
				if pm:
					print("loading pm")
					pml = core.ProcessModelSegmentIO(o)
					if (loader.initialiseSegmentLoader(pml)):
						loader.loadSegment(pml)
						print("loaded")
					else:
						print("Couldn't load process info!");
						return
		
				print("loading oworld")
				ow = core.OWorld(True)
				ow.thisown = False
				
				print("adding org")
				ow.addOrganism(o)
				ow.setBounds(hdr.worldBounds)
				
				print("setting world")		
				simulation = core.SDSSimulation()
				simulation.setWorld(ow)
				simwrap = simwrapper(simulation)
		
				# dump the frame into blender
				# output = blender.get_view()			
				bo.render_frame(fb.create_frame(simwrap))
				
				# add subsurf
				# TODO: This currently doesn't work as expected...
				# Use Mesh_mod instead...I think?
				#obj = bpy.context.active_object
				#bpy.ops.object.modifier_add(type='SUBSURF')
				#mod = obj.modifiers[-1]
				#mod.levels = 1
				#mod.render_levels = 3
				
				# TODO: fix this, objects should be deleted after a frame is created
				#stasis.append(m)
				#stasis.append(o)
				#stasis.append(ow)
				
				# disown process model
				o.setProcessModel(None)
				# cleanup will delete oworld, organism and mesh
				simulation.cleanup()
				
				#print("gc %d %d %d"%gc.get_count())				
		print("done")

class IMPORT_OT_sds(bpy.types.Operator):
	'''Import an SDS simulation'''
	bl_idname = "import_scene.sds"
	bl_label = "Import SDS"
	
	path = StringProperty(name="File Path", description="File path used for importing the SDS file", maxlen= 1024, default="")
	first_frame = IntProperty(name="First Frame", description="First frame to import", default=0)
	last_frame = IntProperty(name="Last Frame", description="Last frame to import (-1=last)", default=-1)
	delta_frames = IntProperty(name="Frame Skip", description="Frames to skip over", default=0)	
	
	#CREATE_SMOOTH_GROUPS = BoolProperty(name="Smooth Groups", description="Surround smooth groups by sharp edges", default= True)
	#CREATE_FGONS = BoolProperty(name="NGons as FGons", description="Import faces with more then 4 verts as fgons", default= True)
	
	def execute(self, context):
		load_sds(self.properties.path, (self.properties.first_frame,self.properties.last_frame,self.properties.delta_frames), context)
		return {'FINISHED'}	
	
	def invoke(self, context, event):	
		wm = context.manager
		wm.add_fileselect(self)
		return {'RUNNING_MODAL'}

bpy.types.register(IMPORT_OT_sds)
#bpy.ops.add(IMPORT_OT_sds) 

menu_func = lambda self, context: self.layout.operator(IMPORT_OT_sds.bl_idname, text="SDS (.cfg)...")
bpy.types.INFO_MT_file_import.append(menu_func)
#menu_item = dynamic_menu.add(bpy.types.INFO_MT_file_import, menu_func)

if __name__ == "__main__":	
	#load_sds("test.cfg", None)
	pass

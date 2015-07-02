import bpy

material = bpy.data.materials['Material']


def meshes():
    return [m for m in bpy.data.meshes if m.name.startswith('SimFrame')]

def makeSmooth():
    for mesh in meshes():
        for face in mesh.faces:
            face.smooth = 1

def applyMat():    
    for mesh in meshes():
        mesh.materials += [material]

def applySubsurf(levels = 1, render_levels = 2):
    #bpy.ops.object.select_pattern(pattern="SimFrame???")
    for o in bpy.data.objects:
        o.selected = False

    first_obj = None
    for obj in bpy.data.objects:
        if obj.type=="MESH" and obj.name.startswith('SimFrame'):
            if first_obj is None: 
                first_obj = obj
            obj.selected = True

    # add modifier
    try:
        bpy.ops.object.modifier_add(type='SUBSURF')
        mod = first_obj.modifiers[-1]
        mod.levels = levels
        mod.render_levels = render_levels
    except TypeError:
        pass

    for o in bpy.data.objects:
        o.selected = False


def applySubsurf2(levels = 1, render_levels = 2):
    #bpy.ops.object.select_pattern(pattern="SimFrame???")

    for obj in bpy.data.objects:
        if obj.type=="MESH" and obj.name.startswith('SimFrame'):
            frame_no = int(obj.name[-3:])
            #if (frame_no > 4): return
            print("frame_no: %d"%frame_no)
            bpy.context.scene.set_frame(frame_no)
            #bpy.ops.object.select_name(name=obj.name)
            #print(bpy.context.selected_objects)
            #continue            
            bpy.context.scene.objects.active = obj
        
            # add modifier
            try:
                bpy.ops.object.modifier_add(type='SUBSURF')
                mod = obj.modifiers[-1]
                mod.levels = levels
                mod.render_levels = render_levels
            except TypeError:
                pass
    
            #bpy.ops.object.select_all(action='DESELECT')

def redrawAll():
    for obj in bpy.data.objects:
        if obj.type=="MESH":
            obj.make_display_list() # Needed to apply the modifier

#makeSmooth()
#applyMat()
applySubsurf2(1,3)
redrawAll() 

#Window.RedrawAll() # View the change
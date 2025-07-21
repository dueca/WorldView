import os
import sys
import time
import bpy
from bpy import context

print(sys.argv)

# this converts a blender file to a "baked" model with the same base name
if len(sys.argv) < 5 or not sys.argv[4].endswith('.blend') or \
   (len(sys.argv) >= 6 and sys.argv[5] not in ('collada', 'glb', 'gltf')):
    print("""Usage:
    bake-a-blend file [collada|glb]""", file=sys.stderr)
    sys.exit(0)

infile = sys.argv[4]
base = os.path.basename(infile)[:-6]

def writeglb():
    bpy.ops.export_scene.gltf(filepath=f'{base}.glb', export_animations=False,
                              use_selection=True, export_apply=True,
                              export_yup=True,
                              export_texcoords=True, export_normals=True,
                              export_lights=True)

def writegltf():
    bpy.ops.export_scene.gltf(filepath=f'{base}.gltf',
                              export_format='GLTF_EMBEDDED',
                              export_animations=False,
                              use_selection=True, export_apply=True,
                              export_yup=True, filter_glob="*.gltf",
                              export_texcoords=True, export_normals=True,
                              export_lights=True)
  
def writecollada():
    bpy.ops.wm.collada_export(filepath=f'{base}.dae', selected=True)

writers = dict(glb=writeglb, collada=writecollada, gltf=writegltf)

if len(sys.argv) >= 6:
    writer = writers[sys.argv[5]]
else:
    writer = writeglb

# had problems with non-unicode????
def neuter(word):
    return ''.join([i < 128 and chr(i) or '#' for i in map(ord, word)])

# function to apply any remaining modifiers & create meshes
def apply_modifiers(obj):
    print(f"Checking modifiers for {obj.name}")

    # make the object active, otherwise nothing happens
    bpy.context.view_layer.objects.active = obj

    # run through the modifiers
    for _, m in enumerate(obj.modifiers):
        try:
            mname = neuter(m.name)
            oname = neuter(obj.name)
            bpy.ops.object.modifier_apply(modifier=mname)
            print(f"Applied {mname} to {oname}")
        except RuntimeError as e:
            print(f"Error applying {mname} to {oname}, removing it instead.")

    for m in obj.modifiers:
        print(f"Removing remaining modifier {m.name}")
        obj.modifiers.remove(m)

def convert_to_mesh(obj):
    
    if obj.type in ("CURVE", "METABALL", "SURFACE", "TEXT"):
        print(f"Converting {obj.name} from {obj.type} to mesh")

        # make the object active, otherwise nothing happens
        bpy.ops.object.select_all(action='DESELECT')
        obj.select_set(True)
        # bpy.context.view_layer.objects.active = obj
        bpy.ops.object.convert(target='MESH')
        
# load the file
bpy.ops.wm.open_mainfile(filepath=sys.argv[4])

# work in a context override (why? dunno)
window = context.window_manager.windows[0]
with context.temp_override(window=window):

    # object mode?
    bpy.ops.object.mode_set(mode = 'OBJECT')

    # each object, apply any remaining modifiers
    for o in bpy.context.scene.objects:

        # apply remaining modifiers
        apply_modifiers(o)

        # convert incompatible objects to mesh
        convert_to_mesh(o)

    # select everything, except main light
    for o in bpy.context.scene.objects:
        o.select_set(o.name != 'Light')

    # save the file
    writer()

    # exit blender
    bpy.ops.wm.quit_blender()

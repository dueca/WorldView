import os
import sys
import time
import bpy
from bpy import context

# this converts a blender file to a "baked" model with the same base name
# the arguments after '--' are for the script

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

try:
    idb = sys.argv.index('--')
    if not idb:
        raise ValueError("Give conversion arguments after '--'")

    bakeargs = sys.argv[idb+1:]
    if len(bakeargs) < 1 or not bakeargs[0].endswith(".blend"):
        raise ValueError("Need blender filename argument")

    infile = bakeargs[0]
    base = os.path.basename(infile)[:-6]

    if len(bakeargs) >= 2:
        try:
            writer = writers[bakeargs[1]]
        except KeyError:
            raise ValueError(f"Cannot convert to type {bakeargs[1]}")
    else:
        writer = writeglb

except ValueError as e:
    print(f"""Usage:
    bake-a-blend file [collada|glb|gltf]

    Error: {e}""", file=sys.stderr)
    sys.exit(1)


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
bpy.ops.wm.open_mainfile(filepath=infile)

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

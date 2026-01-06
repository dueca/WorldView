# VSGViewer

## Introduction

Viewer back-end that uses the new VulkanSceneGraph code to produce a
view on a world with controlled objects. The WorldView module will accept
information on the observer's (ego) motion and on motion and presence of
other entities in a visual scene. VSGViewer object will translate this into
graphics created with Vulkan and VulkanSceneGraph.

## Design

### Nodes and scene graph tree

VulkanSceneGraph (vsg), takes a minimalist, light-weight approach to scene
graphs. I hope to do right by it in this module. The current implementation
offers an xml-based way of configuring a scene, and specifying how simple
graphical objects can be attached to and driven by DUECA channel data.

There are two main nodes in the scene graph; "root", which is simply the root
node, and "observer", which mimics the motion of an observer. You can attach
your models to the observer node, and that means that they will be "carried"
by the observer, as an example, take your vehicle interior, if you want to
show it in VR.

### Simple fog

The current code loads the standard VSG pbr shaders. When you specify fog,
the code will use custom shaders with the fog values.

Visual models can be specified in several different ways. A convenient and
fairly portable way is to use gltf / glb files. It is also possible (and
quicker to load) to use files converted to vsg with vsgconvd. However, this
may have the suprising effect that fog doesn´t work for these objects; the
objects are supplied with the standard shader, and fog doesn´t affect the
visualization. Objects stored in other formats are converted on the fly, and
then processed by the shader specified in the program.

### XML format

The VSGViewer is supported by a simple XML loader, that takes a simple xml
file, and converts the data to instructions to create visual models,
translations, lights or the like.

The following objects are defined by default:

| Object | parameter | type | description |
|--------|-----------|------|--------------|
| ambient-light | color | 3 float | scaled 0..1, comma-separated |
|               | intensity | float |                            |
| directional-light | color | 3 float |                          |
|               | intensity | float |                            |
|               | direction | 3 float | direction vector, AE convention |
| point-light   |  color | 3 float |                             |
|               | intensity | float |                            |
|               | xyz      | 3 float | location of the light     |
|               | span     | float | range of the light          |
| spot-light    |  color | 3 float |                             |
|               | intensity | float |                            |
|               | xyz      | 3 float | location of the light     |
|               | span     | float | range of the light          |
|               | innerAngle | float |                           |
| static-transform | xyz   | 3 float |                           |
|               | phithetapsi | 3 float |  degrees               |
|               | scalexyz | 3 float |                           |
| centered-transform | xyz   | 3 float | xy attached to observer  |
|               | phithetapsi | 3 float |  degrees               |
|               | scalexyz | 3 float |                           |
| tiled-transform | tilesizexyz   | 3 float | jumps with tile size  |
|                | xyz   | 3 float |         |
|               | phithetapsi | 3 float |  degrees               |
|               | scalexyz | 3 float |                           |
| transform     | scalexyz | 3 float | pos+angles controlled by channel entry  |
| model         | filename | str     |                           |
| static-model  | filename | str     | pos+angles controlled by channel entry  |
| switch        | selected_child | int    | initially active child   |

The model and static-model take a file name for the visual model file.

The transform and model are to be linked to a channel entry, and controlled
from the entry position and attitude. The "model" type directly attaches the
model in the file to the given position. If an offset is needed, a transform
can be used. This transform describes the offset from the position and
orientation controlled by the channel, and a static-model (or several) may be
supplied as child to the transform.

Files loaded by the xml loader follow the format in `vsgworld.xsd`. In summary
a graphic object can be defined/manipulated as:

- `template`. The configuration given to a template can be used for a later
  definition which lists the template. Alternatively, it can be used to
  define visual objects created dynamically by the appearance of entries in
  the channel with visible entries. The key attribute of the template
  determines how it can be accessed.

  To re-use the template in a static
  definition, specify the key as the template attribute. For dynamic objects, linked to a channel, the key is used as the match to find the
  right template. The matching process is further described in the documentation for the SpecificationBase class.

- `static`. A static object does not move by itself, it is not controlled
  by a channel.

- `remove`. With a remove entry, the named object is removed.

- `modify`. With a modify entry, an existing named object can be modified
  (if possible). This is usually limited to addition of children, and
  changes in parameters.

### Extension of the XML format

The WorldDataSpec dco object serves as an intermediate vessel for the
information in the xml file. The "filename" and "coordinates" lists of
strings and floats respectively, are (mis-)used to transfer the relevant
data. The `vsgobjects.xml` file shows these bindings for the standard
VSG objects available. By reading additional files, additional graphical
types in the factory can be configured. The configuration there defines
how named variables are linked to the coordinates array.

### Extension

The VSGViewer uses a factory to link object types to coded classes. To extend
this factory, include the "VSGObjectFactory.hxx" header, and define a maker
object in static text. linking the type name to your code. See the various
VSGObject-derived classes.

## References

A project using Vulkan with a lot of experimental steps [Flower](https://github.com/qiutangu/Flower)

(VulkanSceneGraph code)[https://github.com/vsg-dev/VulkanSceneGraph)

(Example applications)[https://github.com/vsg-dev/vsgExamples)

(Vulkan Guide)[https://vkguide.dev/]

(Learn Vulkan)[https://www.vulkan.org/learn]

(Vulkan Examples)[https://github.com/SaschaWillems/Vulkan]

(Godot as alternative)[https://github.com/Calinou/godot-reflection]

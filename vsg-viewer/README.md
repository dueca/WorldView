# VSGViewer

## Introduction

Viewer back-end that uses the new VulkanSceneGraph code to produce a
view on a world with controlled objects.

## Design

### Nodes and scene graph tree

Lights are -- depending on their type -- with included positioning coordinates. 

To move around models, we use transform nodes as intermediate

### Adding simple fog?

The current code loads the standard VSG pbr shaders. To add fog or the like, custom shaders should be used? Shader example code in vsgExamples/data/shaders.


## References

A project using Vulkan with a lot of experimental steps [Flower](https://github.com/qiutangu/Flower)

(VulkanSceneGraph code)[https://github.com/vsg-dev/VulkanSceneGraph)

(Example applications)[https://github.com/vsg-dev/vsgExamples)

(Vulkan Guide)[https://vkguide.dev/]

(Learn Vulkan)[https://www.vulkan.org/learn]

(Vulkan Examples)[https://github.com/SaschaWillems/Vulkan]

(Godot as alternative)[https://github.com/Calinou/godot-reflection]

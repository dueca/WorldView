# WorldView, A DUECA Project

## Introduction

This is a project that uses [DUECA
middleware](https://github.com/dueca/dueca). It provides a general
interface to 3D graphics back-ends, through the "world-view" module,
and a number of back-ends to scene graph or other means of 3D graphics
generation:

- OgreViewer, interaction with Ogre 1.9
- PlibViewer, with plib, a very simple scene graph library
- OSGViewer, interacting with OpenSceneGraph
- FlightGearViewer, interacting with FlightGear
- VSGViewer, interacting with the new VulkanSceneGraph

## Application

Back-end 3D view. There are some facilities for testing the 3D
views. Using `dueca-gproject`, borrow the following:

    WorldView/WorldView
	WorldView/SpecificationBase
	
and one of the backends. The new 'VSGViewer' back-end currently
gets most testing, after that the `OSGViewer` and `OgreViewer`
back-ends, ymmv on the other backends.

The FlightGearViewer needs a running flightgear, to be started
together with DUECA; check the run folder. It is also able to,
provided youhave the visual models and configuration, show other 
vehicles (like aircraft and ground vehicles).

## Author(s)

René van Paassen, Mark Mulder, Olaf Stroosma, Herman Damveld

## LICENSE

EUPL-1.2

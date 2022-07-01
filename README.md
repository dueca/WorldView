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

## Application

Back-end 3D view. There are some facilities for testing the 3D
views. Using `dueca-gproject`, borrow the following:

    WorldView/WorldView
	WorldView/SpecificationBase
	
and one of the backends. Note that the `OSGViewer` and `OgreViewer`
back-ends had most testing, ymmv on the other backends.

## Author(s)

René van Paassen, Mark Mulder, Olaf Stroosma, Herman Damveld

## LICENSE

EUPL-1.2

This implements the WorldView module, for creating 3D visualizations.

By itself this module does not do much, it needs a helper for the
specific 3D library used. PlibViewer is an example of such a helper. 

The viewers should all roughly present the same minimum functionality:

- A world with static objects in it

- motion of the viewpoint through the world

- multiple views on the world, for simulation set-ups with multiple
  displays/projectors.

- Dynamic objects in the world driven by the MultiStreamChannel with
  data on the other objects. 

- Option for overlays, static (for masks) and controlled (for HUD)

- Buffer swap and blocking on the vertical retrace (where hardware
  permits)

One particularly strong point of this set-up is that the visualization
may run synchronized with the display rate; the ObjectMotion objects
used to communicate with the simulation incorporate extrapolation of
the position and orientation, and may be used to implement compensated
asynchronous updates of display and simulation. 

TODO:

- PlibViewer currently automatically creates new objects based on the
  klass name in the MultiStreamChannel with ObjectMotion
  objects. These objects are not removed when they disappear again
  from the channel. 

- The OgreViewer is currently behind on the PlibViewer, it needs to be
  revised and updated.

- Define an environment interface, for atmosphere effects; time of
  day, fog, etc.

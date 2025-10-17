# Pseudo module HMILabViewFiles

## What is a pseudo-module

In DUECA, a pseudo module does not contain code, and DUECA will not
attempt to compile and include the code files in the module folder.
This is flagged in the `modules.xml` files.

Pseudo modules are typically used to store data files that are or
could be shared among different projects, such as files for graphic
interfaces.

## HMILabViewFiles

This pseudo-module contains some code to calculate the frustum
parameters for one of our simulation laboratories. The HMI lab uses
three COTS DLP projectors that project on a segmented screen covering
three sides of the lab. The frustumcalc.py file contains the
information on start and end points of the screens, and calculates
the frustum parameters from those, for two "eye" positions, the left
seat (car driving set-up) and the right seat (aircraft and manual
control tasks with a side stick).

To account for inaccuracies where screens connect, the projections
are extended 20 pixels beyond the screen sizes, and with masks (svg
files created in inkscape, and then blurred with imagemagick), the
edges are adjusted. The overlaps can be tweaked by using test
versions of the masks, which have sharp transitions.

Your lab of course may be different, but these files may give you
some inspiration on setting up masks for your applications.

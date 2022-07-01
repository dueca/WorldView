This is a viewer helper object for the WorldView module. It makes no
sense to use it without that module, so in your project borrow both
the WorldView and this module, from the WorldView project. Look at the
test application in the project WorldView for guidance on how to configure
this. 

To add to your project Makefile:

PLIBLIBS =	-lplibssg -lplibsl -lplibfnt -lplibssgaux -lplibsg \
	 	-lplibul

Add $(PLIBLIBS) to the EXTRALIBS line

You can optionally add hmilib to the DCOMPONENTS line, to get the
GLRateInfo object to compile as well:

		--hmi --hmi-glfont --hmi-ftgl 

If you use the rate information display, create a hmilib fontmanager
in your dueca.mod, if you do not yet have that, for example:

(define font-manager
  (make-font-manager "ftgl"
    'set-drawing-method "polygon+outline"
    'add-font (make-font "/usr/share/HMIlib/fonts/Vera.ttf")
    'add-font (make-font "/usr/share/HMIlib/fonts/Monaco.ttf")
    'add-font (make-font "/usr/share/HMIlib/fonts/btype.ttf")
  )
)


TODO: 

- Add fog specification, and use the ssgaSky in a PlibObject

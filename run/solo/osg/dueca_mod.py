## -*-python-*-
## this is an example dueca_mod.py file, for you to start out with and adapt
## according to your needs. Note that you need a dueca_mod.py file only for the
## node with number 0

## in general, it is a good idea to clearly document your set up
## this is an excellent place.
claim_graphics_thread = True
f = 4

## node set-up
ecs_node = 0   # dutmms1, send order 3
#aux_node = 1   # dutmms3, send order 1
#pfd_node = 2   # dutmms5, send order 2
#cl_node = 3    # dutmms4, send order 0

## priority set-up
# normal nodes: 0 administration
#               1 hdf5 logging
#               2 simulation, unpackers
#               3 communication
#               4 ticker

# administration priority. Run the interface and logging here
admin_priority = dueca.PrioritySpec(0, 0)

# logging prio. Keep this free from time-critical other processes
log_priority = dueca.PrioritySpec(1, 0)

# priority of simulation, just above log
sim_priority = dueca.PrioritySpec(2, 0)

# nodes with a different priority scheme
# control loading node has 0, 1, 2 and 3 as above and furthermore
#               4 stick priority
#               5 ticker priority
# priority of the stick. Higher than prio of communication
# stick_priority = dueca.PrioritySpec(4, 0)

# timing set-up
# timing of the stick calculations. Assuming 100 usec ticks, this gives 2500 Hz
# stick_timing = dueca.TimeSpec(0, 4)

# this is normally 100, giving 100 Hz timing
sim_timing = dueca.TimeSpec(0, 100)

## for now, display on 50 Hz
display_timing = dueca.TimeSpec(0, 200)

## log a bit more economical, 25 Hz
log_timing = dueca.TimeSpec(0, 400)

if claim_graphics_thread:
    graphics_priority = log_priority
else:
    graphics_priority = admin_priority

## ---------------------------------------------------------------------
### the modules needed for dueca itself
if this_node_id == ecs_node:

    # create a list of modules:
    DUECA_mods = []
    DUECA_mods.append(dueca.Module("dusime", "", admin_priority))
    DUECA_mods.append(dueca.Module("dueca-view", "", admin_priority))
    DUECA_mods.append(dueca.Module("activity-view", "", admin_priority))
    DUECA_mods.append(dueca.Module("timing-view", "", admin_priority))
    DUECA_mods.append(dueca.Module("log-view", "", admin_priority))
    DUECA_mods.append(dueca.Module("channel-view", "", admin_priority))
    # uncomment for web-based graph, see DUECA documentation
    # DUECA_mods.append(dueca.Module("config-storage", "", admin_priority))

    if no_of_nodes > 1 and not classic_ip:
        DUECA_mods.append(dueca.Module("net-view", "", admin_priority))

    # create the DUECA entity with that list
    DUECA_entity = dueca.Entity("dueca", DUECA_mods)

## ---------------------------------------------------------------------
# modules for your project (example)
mymods = []
drivemods = []

if this_node_id == ecs_node:
    mymods.append(dueca.Module(
        "world-view", "", graphics_priority).param(
            set_timing = display_timing,
            check_timing = (10000, 20000),
            claim_thread = claim_graphics_thread,
            set_viewer =
            dueca.OSGViewer().param(

                # Windows and viewports set-up

                # Front window
                ('add-window', "front"),
                ('window-size+pos', (int(1920/f), int(1080/f), 0, 0)),
                               
                ## one viewport for the front window
                ('add-viewport', "front"),
                ('viewport-window', "front"),
                ('viewport-pos+size', (int(62/f), 0, int(1796/f), int(1080/f))),
                ('set-frustum',
                 (1.0, 10000.0,
                  -1.171378,  0.724228,
                  -0.479275,  0.660622)),
                ('eye-offset',
                 (0.0, 0.0, 0.0, 0.0, 0.0, -0.000000)),
                ('viewport-overlay', "HMILabOverlays/MaskFrontSharp"),

                ## window with side views
                ('add-window', "sides"),
                ('window-size+pos', (
                    int(3840/f), int(1080/f), 0, 0)),
                
                ('add-viewport', "left side"),
                ('viewport-window', "sides"),
                ('viewport-pos+size', (
                    int(121/f), 0, int(1719/f), int(1080/f))),
                 ('set-frustum',
                  (1.0, 10000.0,
                   -0.750118,  0.675156,
                   -0.376500,  0.518960)),
                 ('eye-offset',
                  (0.0, 0.0, 0.0, 0.0, 0.0, -82.361594)),
                ('viewport-overlay', "HMILabOverlays/MaskLeftSideSharp"),
                
                ('add-viewport', "right side"),
                ('viewport-window', "sides"),
                ('viewport-pos+size', (
                    int(2175/f), 0, int(1345/f), int(1080/f))),
                ('set-frustum',
                 (1.0, 10000.0,
                  -1.218198,  0.588489,
                  -0.609963,  0.840760)),
                ('eye-offset',
                 (0.0, 0.0, 0.0, 0.0, 0.0, 85.112990)),
                ('viewport-overlay', "HMILabOverlays/MaskRightSideSharp"),

                # add visual objects (classes, then instantiation)
                ('add-object-class-data',
                 ("static:sunlight", "sunlight", "static-light")),

                ('add-object-class-coordinates',
                 (0.48, 0.48, 0.48, 1,   # ambient
                  0.48, 0.48, 0.48, 1,   # diffuse
                  0.0, 0.0, 0.0, 1,      # specular
                  0.4, 0.0, 1.0, 0,      # south??
                  0, 0, 0,               # direction not used
                  0.2, 0, 0)),           # no attenuation for sun
                ('add-object-class-data',
                 ("static:terrain", "terrain", "static", "terrain.obj")),
                ('add-object-class-data',
                 ("centered:skydome", "skydome", "centered", "skydome.obj")),
                ('add-object-class-coordinates',
                 (0.0, 0.0, 50.0)),

                # make the objects
                ('static-object', ('static:sunlight', 'sunlight')),
                ('static-object', ('static:terrain', 'terrain')),
                ('static-object', ('centered:skydome', 'skydome'))
                
            ).complete(),
            initial_camera = ( 0, 0, -30, 0, 0, 0)
        ))

    mymods.append(dueca.Module(
        "visual-test-drive", "", admin_priority).param(
            ('set-timing', sim_timing),
	    ('check-timing', (10000, 20000)),
	    ('add-motion', "myself"),
	    ('position', (-80, 0, -3)),
	    ('orientation', (0, 0, 0)),
	    ('speed', (1.0, 0, 0)),
	    ('dt', 0.1),
	    ('rotation', (0, 0, 0.4)),
	    ('add-motion', "houseX"),
	    ('position', (0, 0, -30)),
	    ('rotation', (1, 0, 0)),
	    ('dt', 0.1),
	    ('add-motion', "head"),
	    ('position', (160, 40, -40)),
	    ('orientation', (0, 0, 0)),
	    ('rotation', (1, 1, 10)),
	    ('dt', 0.1)
        ))

    # add a filer in this node for replay support
    # filer = dueca.ReplayFiler("PLHLAB")

# then combine in an entity (one "copy" per node)
if drivemods:
    driveentity = dueca.Entity("drive", drivemods)
    
if mymods:
    myentity = dueca.Entity("ogre", mymods)

## -*-python-*-
## this is an example dueca_mod.py file, for you to start out with and adapt
## according to your needs. Note that you need a dueca_mod.py file only for the
## node with number 0

## in general, it is a good idea to clearly document your set up
## this is an excellent place.
claim_graphics_thread = True

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

# scale factor, for testing (integer!)
f = 4

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

    # remove the quotes to enable DUSIME initial condition recording and
    # setting, and simulation recording and replay
    '''
    for e in ("PHLAB",):
        DUECA_mods.append(
            dueca.Module("initials-inventory", e, admin_priority).param(
                reference_file=f"initials-{e}.toml",
                store_file=f"initials-{e}-%Y%m%d_%H%M.toml"))
        DUECA_mods.append(
            dueca.Module("replay-master", e, admin_priority).param(
                reference_files=f"recordings-{e}.ddff",
                store_files=f"recordings-{e}-%Y%m%d_%H%M%S.ddff"))
    '''

    # create the DUECA entity with that list
    DUECA_entity = dueca.Entity("dueca", DUECA_mods)

## ---------------------------------------------------------------------
# modules for your project (example)
mymods = []

if this_node_id == ecs_node:
    mymods.append(dueca.Module(
        "world-view", "", graphics_priority).param(
            set_timing = display_timing,
            claim_thread = claim_graphics_thread,
            check_timing = (10000, 20000),
            set_viewer =
            dueca.OSGViewer().param(
                set_resourcepath='.',
                keep_cursor=True).param(
                    ('add-object-class-data',
                     ('static:world', 'world', 'static', 'terrain.obj')),
                    ('static-object', ('static:world', )),
                    ('add-object-class-data',
                     ("static:sunlight", "sunlight", "static-light")),
                    ('add-object-class-coordinates',
                     (0.8, 0.8, 0.8, 1,           # ambient
                      0.8, 0.8, 0.8, 1,           # diffuse
                      0.0, 0.0, 0.0, 1,           # specular
                      0.4, 0.0, 1.0, 0,           # south??
                      0, 0, 0,                    # direction not used
                      0.2, 0, 0)),                  # no attenuation for sun
                    ('static-object', ("static:sunlight",)),
                    ('add-object-class-data',
                     ("centered:skydome", "skydome", "centered", "skydome.obj")),
                    ('add-object-class-coordinates',
                     (0.0, 0.0, 50.0)),
                    ('static-object', ('centered:skydome', 'skydome')),
                    ('add-window', 'main'),
                    ('window-size+pos', (1920//f, 1080//f, 0, 0)),
                    ('add-viewport', 'portfront'),
                    ('viewport-window', 'main'),
                    ('viewport-pos+size', (62 // f, 0, 1796 // f, 1080 // f)),
                    ('eye-offset', (0, 0, 0, 0, 0, 0)),
                    ('set-frustum',
                     ( 1.0, 10000.0,
                       -1.17137785454, 0.724227595471,
                       -0.479274611399, 0.660621761658)),
                    ('add-window', 'left'),
                    ('window-size+pos', (1920//f, 1080//f, 0, 0)),
                    ('add-viewport', 'portleft'),
                    ('viewport-window', 'left'),
                    ('viewport-pos+size', (121//f, 0, 1719//f, 1080//f)),
                    ('eye-offset', (0, 0, 0, 0, 0, -82.362)),
                    ('set-frustum', (
                        1.0, 10000.0,
                        -0.75011761385, 0.675155946186,
                        -0.376500155079, 0.51895967321)),
                    ('add-window', 'right'),
                    ('window-size+pos', (1920//f, 1080//f, 0, 0)),
                    ('add-viewport', 'portright'),
                    ('viewport-window', 'right'),
                    ('viewport-pos+size', (255//f, 0, 1345//f, 1080//f)),
                    ('eye-offset', ( 0, 0, 0, 0, 0, 85.113)),
                    ('set-frustum', (
                        1.0, 10000,
                        -1.21819795407, 0.588489166461,
                        -0.60996297175, 0.840759771872)),

                    # mask, first as class
                    ('add-object-class-data',
                     ('mask:left', 'mask-left', 'overlay',
                      'hmilabmasklefttest.png', 'left', 'portleft')),
                    ('add-object-class-data',
                     ('mask:front', 'mask-front', 'overlay',
                      'hmilabmaskfronttest.png', 'main', 'portfront')),
                    ('add-object-class-data',
                     ('mask:right', 'mask-right', 'overlay',
                      'hmilabmaskrighttest.png', 'right', 'portright')),

                    # now instantiate
                    ('static-object', ("mask:left",)),
                    ('static-object', ("mask:front",)),
                    ('static-object', ("mask:right",)),
                ).complete()
        ))

    mymods.append(dueca.Module(
        "visual-test-drive", "", admin_priority).param(
            ('set-timing', sim_timing),
            ('check-timing', (10000, 20000)),
            ('add-motion', "myself"),
            ('position', (-20, 0, -3)),
            ('orientation', (0, 0, 0)),
            ('speed', (1.0, 0, 0)),
            ('dt', 0.1),
            ('rotation', (0, 0, 0.4))
            ))

    # add a filer in this node for replay support
    # filer = dueca.ReplayFiler("PLHLAB")

# then combine in an entity (one "copy" per node)
if mymods:
    myentity = dueca.Entity("PHLAB", mymods)

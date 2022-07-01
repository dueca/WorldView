## -*-python-*-
## this is an example dueca_mod.py file, for you to start out with and adapt 
## according to your needs. Note that you need a dueca_mod.py file only for the 
## node with number 0

## in general, it is a good idea to clearly document your set up
## this is an excellent place. 

# set up switches
claim_visual_thread = True

## node set-up
ecs_node = 0   # dutmms1, send order 0
ig_node_front = 1 # dutmms3_0
ig_node_sides = 2 # dutmms3_1

## priority set-up
# normal nodes: 0 administration
#               1 simulation, unpackers
#               2 communication
#               3 ticker

# administration priority. Run the interface and logging here
admin_priority = dueca.PrioritySpec(0, 0)

# priority of simulation, just above adiminstration
if this_node_id == ecs_node:
    sim_priority = dueca.PrioritySpec(1, 0)
else:
    sim_priority = dueca.PrioritySpec(2, 0)

if claim_visual_thread:
    gr_priority = dueca.PrioritySpec(1, 0)
else:
    gr_priority = dueca.PrioritySpec(0, 0)

# nodes with a different priority scheme
# control loading node has 0, 1 and 2 as above and furthermore
#               3 stick priority
#               4 ticker priority
# priority of the stick. Higher than prio of communication
# stick_priority = dueca.PrioritySpec(3, 0)

# timing set-up
# timing of the stick calculations. Assuming 100 usec ticks, this gives 2500 Hz
# stick_timing = dueca.TimeSpec(0, 4)

# this is normally 100, giving 100 Hz timing
sim_timing = dueca.TimeSpec(0, 100)

## for now, display on 50 Hz
display_timing = dueca.TimeSpec(0, 200)

## log a bit more economical, 25 Hz
log_timing = dueca.TimeSpec(0, 400)

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

    # create the entity with that list
    DUECA_entity = dueca.Entity("dueca", DUECA_mods)

## ---------------------------------------------------------------------
# modules for your project (example)
mymods = []

if this_node_id == ecs_node:
    mymods.append(dueca.Module(
        "visual-test-drive", "c", sim_priority).param(
            set_timing = sim_timing,
            check_timing = (10000, 20000)).param(
            add_motion = "myself").param(
            speed = [0.6, 0, 0], 
            dt = 0.1,
            rotation = [ 0, 0, 0.4]).param(
            add_motion = "house").param(
            set_class = "houseX",
            position = [30.5, 0, 0], 
            rotation = [1, 0, 0], 
            dt = 0.1)
    )
    mymods.append(dueca.Module(
        "control-view", "", admin_priority))
    mymods.append(dueca.Module(
        "configure-view", "", admin_priority))

if this_node_id == ig_node_front:
    mymods.append(dueca.Module(
        "world-view", "front", gr_priority).param(
            claim_thread = claim_visual_thread,
            set_timing = display_timing,
            check_timing = [ 10000, 20000 ],
            set_viewer = dueca.OgreViewer().param(
                add_window = "front").param(
                window_size_pos = [ 1800, 1080, 60, 0 ]).param(
                add_viewport = "front").param(
                viewport_window = "front").param(
                viewport_pos_size = [62, 0, 1796, 1080],
                set_frustum = [ 1.0, 10000, 
                                -0.4209, 0.8658, -0.3507, 0.4432],
                eye_offset = [0, 0, 0, 0, 0, 0 ]).param(
                add_resource_location = [ ".", "FileSystem", "local" ],
 	        load_scene = [ "./sample2.scene", "local" ],
		add_object_class = [ "houseX", "Barrel.mesh"]).complete()))

if this_node_id == ig_node_sides:
    mymods.append(dueca.Module(
        "world-view", "sides", gr_priority).param(
            claim_thread = claim_visual_thread,
            set_timing = display_timing,
            check_timing = [ 10000, 20000 ],
            set_viewer = dueca.OgreViewer().param(
                add_window = "sides").param(
                window_size_pos = [ 3840, 1080, 0, 0 ]).param(
                add_viewport = "right").param(
                viewport_window = "sides").param(
                viewport_pos_size = [2175, 0, 1345, 1080],
                set_frustum = [ 1.0, 10000, 
                                -0.8730, 0.1779, -0.3740, 0.4726],
                eye_offset = [0, 0, 0, 0, 0, 85.113 ],
                viewport_overlay = "HMILabOverlays/MaskRightSide").param(
                add_viewport = "left").param(
                viewport_window = "sides").param(
                viewport_pos_size = [ 121, 0, 1719, 1080],
                set_frustum = [ 1.0, 10000, -0.2796, 1.9294, -0.6376, 0.8057],
                eye_offset = [0, 0, 0, 0, 0, -82.362 ],
                viewport_overlay = "HMILabOverlays/MaskLeftSide").param(
                add_resource_location = [ ".", "FileSystem", "local" ],
 	        load_scene = [ "./sample2.scene", "local" ],
		add_object_class = [ "houseX", "Barrel.mesh"]).complete()))
    
# etc, each node can have modules in its mymods list

# then combine in an entity (one "copy" per node)
if mymods:
    myentity = dueca.Entity("ogre", mymods)


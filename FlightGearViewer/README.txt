Some notes on the state of this viewer.

The FlightGearViewer is an insert for the WorldView module. It can
pack the eye position data, and send it over a UDP connection to a
running FlightGear. With the proper configuration, the out-of-the
window view is generated. 

The current version sends the eye point data, and it does adds the
data from other objects in the ObjectMotion multistream channel,
sending that data to an fgms multiplayer server.

Use the example configuration file for the mp server:

fgms -c fgms.conf -v 3


To enable the view of other a/c, the flightgear multiplayer server can
be used. The file mpmessages.hxx shows the message structure for the
communication. Messages consist of a header (T_MsgHdr, a position
message T_PositionMsg, which apparently is ECEF, with the attitude
quaternion truncated to the vector part, and property parts,
xdr_data_t. The struct uses XDR_encode. Properties are signified by an
uint32 for the property id, see the list in multiplaymgr.cxx and data
as float, 32 bit int, or string (4 byte length, + the string padded to
4 bytes, XDR_encode_int8. This requires running a multiplayer server,
which looks like no problem. A selection should be made of the
properties that need to be sent over. 


Regarding synchronizing with opengl, I don't have a good clue
yet. Currently, the message reading is non-blocking, which does not
help, and I have been looking into the main loop, which includes a
calback from osg. 


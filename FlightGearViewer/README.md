# Flightgear as back-end

## Introduction

The FlightGearViewer is an insert for the WorldView module. It can
pack the eye position data, and send it over a UDP connection to a
running FlightGear. With the proper configuration, the out-of-the
window view is generated.

The current version has the following capabilities:

- Coordinates from a local carthesian reference system are
  converted to lat-lon-alt / psi-theta-phi before transmission
  to FlightGear.

- Use FLightGear's multiplayer input (directly, without intervening
  multiplayer server) to show other aircraft or vehicles.

- Using FlightGear camera configuration files, the view offset and
  angle can be controlled.

- The height above terrain can be reported back.

- Additional multiplayer model data can be encoded and sent, e.g.,
  livery, control surface movements, etc., if the DCO type in the multiplayer channel has been so equipped.

## Translating ObjectMotion data to other entities

To match the data in the ObjectMotion://world channels and the like to Flightgear, the viewer can be supplied with model-table entries. These link the entry in the channel (based on DCO type and label), with an object factory (defining how the data is to be converted), and a flightgear model and livery choice. See the generated documentation on the viewer:

~~~~{.bash}
DUECA_SCRIPTINSTRUCTIONS=FlightGearViewer ./dueca_run.x
~~~~

For use of the `fgprops` factory, the DCO type that you are using needs to have  an encodeProperties method that can encode the additional properties into a multiplayer message. The FGBaseAircraftMotion DCO object has a standard set of these properties already.
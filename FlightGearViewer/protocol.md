# FlightGear multiplayer

## Notes on the protocol

Consists of message header and position message, using xdr, described in

    flightgear/src/MultiPlayer/mpmessages.hxx

Additional data, e.g., liveries, control surface position,
gear extension, gear strut compression etc, can be send as
coded properties from the FlightGear property list. This list is in:

    flightgear/src/MultiPlayer/multiplaymgr.cxx

This list has been re-coded in the `MultiPlayerEncode.cxx` file, which uses the standard xdr library, currently, the MultiPlayerEncode can encode messages, and provide you with a data dump; current set-up in the
FlightGearViewer is one way, sending aircraft positions only.

To add static default values for the messages, you can define these in a
JSON file, and give the name for that file as an additional parameter when configuring the model table in the FlightGearViewer object.

An example JSON file for the A380:

    [
        { "code": 13000, "value": 357 },
        { "code": 1101, "value": "QFA" }
    ]

The 13000 property is the fallback model, see the list in the FlightGear data folder:

    /usr/share/games/flightgear/AI/Aircraft/fallback_models.xml

If the requested livery is not available, the fallback model will be
displayed.

## Configuring multiplay

The multiplay functionality in FlightGearViewer is direct, i.e., it directly sends multiplay messages to your flightgear process. In the `fgfs` start, add multiplay communication, you only need input:

    -multiplay=in,100,127.0.0.1,5001

In this example, communication is on the local host.

For your `dueca_mod.py` file, add `model_table` entries for the
`FlightGearViewer` object. These entries describe how data in the
"world" channel are mapped to aircraft in the FlightGear world.

Entries in the table like:

~~~~{.py}
                ('model-table',
                 ('ObjectMotion:c172', 'Aircraft/A380/XML/A380.xml', 'a380.json')),
                ('model-table',
                 ('ObjectMotion:ufo', 'Aircraft/ufo/Models/ufo.xml', '')),
~~~~

Describe how a channel entry in the "ObjectMotion://world" channel, (or any relevant other channel as configured in your `world-view` module), are mapped to aircraft sent to FlightGear. The `c172` or `ufo` would match to entries of type `ObjectMotion`, or a subclass of that type, with a label `c172` or `ufo` respectively.

It would also match to labels like `c172|PH-COZ`, which will select a c172 model, but name the aircraft identification as `PH-COZ`. See the documentation for the `SpecificationBase` class for other options.
#!/bin/bash

# This nifty command gets the dir name we are running in
DIRECTORY=${PWD##*/}

# Default for all non-configured platforms
LOGLEVEL=warn
LOGCLASS=all
CS=PHTEST

# default location FG_ROOT?
if test -z "${FG_ROOT}"; then
    test -d /usr/share/flightgear && FG_ROOT=/usr/share/flightgear
    test -d /usr/share/games/flightgear && FG_ROOT=/usr/share/games/flightgear
fi

# Check that protocol is installed (for local FG only)
if diff ../../../../WorldView/FlightGearViewer/duecavis.xml \
	${FG_ROOT}/Protocol/duecavis.xml; then
    echo "Using protocol duecavis.xml"
else
    echo "Please install or update duecavis.xml in ${FG_ROOT}/Protocol"
    exit 1
fi

# reception of data
UDP_PORT=5501
# feedback of height above terrain
UDP2_PORT=5502
# reception of multiplay
UDP_MULTIPLAY=5001
UDP2_MULTIPLAY=5002

# start flightgear
fgfs \
    --generic=socket,in,100,127.0.0.1,${UDP_PORT},udp,duecavis \
    --generic=socket,out,100,127.0.0.1,${UDP2_PORT},udp,duecavis \
    --callsign=${CS} \
    --multiplay=in,100,127.0.0.1,${UDP_MULTIPLAY} \
    --multiplay=out,100,127.0.0.1,${UDP2_MULTIPLAY} \
    --prop:int:/sim/multiplay/debug-level=0 \
    --airport=EHAM \
    --aircraft=pc12  \
    --fdm=external \
    --disable-real-weather-fetch \
    --start-date-lat=2024:08:23:14:00:00 \
    --enable-clouds3d \
    --prop:bool:/sim/menubar/visibility=false \
    --enable-splash-screen \
    --enable-terrasync \
    --disable-random-objects \
    --disable-random-buildings \
    --disable-random-vegetation \
    --disable-sound \
    --disable-rembrandt \
    --disable-distance-attenuation \
    --fog-nicest \
    --enable-specular-highlight \
    --prop:/local-weather/autostart=0 \
    --prop:bool:/sim/menubar/visibility=false \
    --disable-mouse-pointer \
    --disable-save-on-exit \
    --disable-splash-screen  \
    --log-level=$LOGLEVEL \
    --log-class=$LOGCLASS \
    --log-dir="." \
    --enable-ai-models \
    --disable-ai-traffic &

if [ "$1" = 'nodueca' ]; then
    echo "not starting dueca, one hour of testing" 
   sleep 3600
else
    ./dueca_run.x
fi

killall fgfs

# --metar="XXXX 012345Z 15001KT 0800 BKN02 OVC005 OVC020 08/06 Q0990" \

#!/bin/bash

# This nifty command gets the dir name we are running in
DIRECTORY=${PWD##*/}

# Default for all non-configured platforms
CAMERA_CONFIG=test_camera.xml
LOGLEVEL=info
LOGCLASS=all
CS=test

# If local, check that the protocol is installed
if test -z "${FG_ROOT}"; then
    test -d /usr/share/flightgear && FG_ROOT=/usr/share/flightgear
    test -d /usr/share/games/flightgear && FG_ROOT=/usr/share/games/flightgear
fi

# Check that protocol is installed (works for local FG only)
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
# reception and feedback of multiplay
UDP_MULTIPLAY=5001
UDP2_MULTIPLAY=5002

# On the lab, cameras/projectors all have different FOV/frustum and offset
if [ "$DIRECTORY" == "dutmms14" ]; then
    CAMERA_CONFIG=hmilab-camera-front.xml
    LOGLEVEL=alert
    CS=mms14
fi

if [ "$DIRECTORY" == "dutmms15" ]; then
    CAMERA_CONFIG=hmilab-camera-left.xml
    LOGLEVEL=alert
    CS=mms15
fi

if [ "$DIRECTORY" == "dutmms16" ]; then
    CAMERA_CONFIG=hmilab-camera-right.xml
    LOGLEVEL=alert
    CS=mms16
fi

UDP_PORT=5501

fgfs \
    --generic=socket,in,100,127.0.0.1,${UDP_PORT},udp,duecavis \
    --generic=socket,out,100,127.0.0.1,${UDP2_PORT},udp,duecavis \
    --config=${CAMERA_CONFIG} \
    --callsign=${CS} \
    --multiplay=in,100,127.0.0.1,${UDP_MULTIPLAY} \
    --multiplay=out,100,127.0.0.1,${UDP2_MULTIPLAY} \
    --prop:int:/sim/multiplay/debug-level=0 \
    --airport=EHAM \
    --fdm=external \
    --aircraft=ufo \
    --disable-real-weather-fetch \
    --enable-clouds3d \
    --start-date-lat=2024:08:23:14:00:00 \
    --prop:bool:/sim/menubar/visibility=false \
    --enable-splash-screen \
    --enable-terrasync \
    --disable-random-objects \
    --disable-random-buildings \
    --disable-random-vegetation \
    --disable-sound \
    --disable-rembrandt \
    --enable-distance-attenuation \
    --fog-nicest \
    --enable-specular-highlight \
    --prop:/local-weather/autostart=0 \
    --prop:bool:/sim/menubar/visibility=false \
    --disable-mouse-pointer \
    --disable-save-on-exit \
    --disable-splash-screen  \
    --prop:/sim/rendering/multi-sample-buffers=1 \
    --prop:/sim/rendering/multi-samples=4 \
    --prop:/sim/rendering/shaders/skydome=true \
    --terrain-engine=pagedLOD \
    --log-level=$LOGLEVEL \
    --log-class=$LOGCLASS \
    --log-dir="." \
    --enable-ai-models \
    --disable-ai-traffic &

# do not use --enable-clock-freeze, because that also stops multiplay show

if [ "$1" = 'nodueca' ]; then
    echo "not starting dueca"
else
    ./dueca_run.x

    killall fgfs
fi

# --metar="XXXX 012345Z 15001KT 0800 BKN02 OVC005 OVC020 08/06 Q0990" \

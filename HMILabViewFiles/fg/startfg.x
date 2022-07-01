#!/bin/bash

# This nifty command gets the dir name we are running in
DIRECTORY=${PWD##*/}

if [ "$DIRECTORY" == "dutmms3_0" ]; then
    CAMERA_CONFIG=hmi_front_cameras.xml
    UDP_PORT=5501
    RUN_DUECA=0
fi

if [ "$DIRECTORY" == "dutmms3_1" ]; then
    CAMERA_CONFIG=hmi_side_cameras.xml
    UDP_PORT=5502
    RUN_DUECA=1
fi

fgfs \
    --generic=socket,in,100,127.0.0.1,${UDP_PORT},udp,duecavisbinary \
    --config=${CAMERA_CONFIG} \
    --airport=EHAM \
    --fdm=external \
    --httpd=8080 \
    --telnet=5401 \
    --aircraft=ufo \
    --disable-real-weather-fetch \
    --enable-clouds3d \
    --start-date-lat=2016:08:23:14:00:00 \
    --prop:bool:/sim/menubar/visibility=false \
    --enable-splash-screen \
    --enable-terrasync \
    --disable-random-objects \
    --disable-random-buildings \
    --disable-random-vegetation \
    --disable-ai-traffic \
    --enable-clock-freeze \
    --disable-sound \
    --disable-rembrandt \
    --enable-enhanced-lighting \
    --disable-distance-attenuation \
    --fog-nicest \
    --enable-specular-highlight \
    --prop:/local-weather/autostart=0 \
    --prop:bool:/sim/menubar/visibility=false \
    --disable-mouse-pointer \
    --disable-save-on-exit \
    --disable-splash-screen  \
    1>fgfs.normal 2>fgfs.error &

../../../dueca_run.x

killall fgfs

# --metar="XXXX 012345Z 15001KT 0800 BKN02 OVC005 OVC020 08/06 Q0990" \

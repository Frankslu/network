#!/bin/bash
su -- franks -c '
export DISPLAY=:0
xhost +
vlc -vvv
'
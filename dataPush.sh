#!/bin/bash
# file: dataPush.sh
#
# This script pushes data to github
#

# Make sure we run as root
if [ "$(whoami)" != "root" ]; then
    echo "Only root can do this.";
    exit 1;
else
    cd /var/log
    cp -v battery-new.log /home/pi/code/loradtn-pi/battery-new.log
    # Make sure we are in the right directory
    cd /home/pi/code/loradtn-pi;
    # Now add any changes
    git add battery-new.log;
    # Now commit
    git commit -m "Hourly data push";
    git push;
fi;
#! /bin/bash

# script to locally try out a rebuilt pbm with same
# settins as used in pbm (usually run from /etc/init.d)

set -x

cd /home/pi/code/loradtn-pi
cp -v newpbm /etc/init.d/newpbm
update-rc.d newpbm defaults

DESC="new power manager daemon"
NAME=pbmd
DAEMON=stable/./$NAME
# Moderate sleep mode (ie. short sleep time when low battery)
# Simulated battery voltage with depletion
DAEMON_ARGS="MODERATE SIM DEP"
#PIDFILE=/var/run/$NAME.pid
#SCRIPTNAME=/etc/init.d/$NAME

$DAEMON $DAEMON_ARGS

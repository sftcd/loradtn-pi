#! /bin/bash

# script to locally try out a rebuilt pbm with same
# settins as used in pbm (usually run from /etc/init.d)

set -x


DESC="new power manager daemon"
NAME=pbmd
DAEMON=../stable/./$NAME
# Greedy sleep mode (ie. short sleep time when low battery)
# Simulated battery voltage with depletion
DAEMON_ARGS="GREEDY SIM DEP"
#PIDFILE=/var/run/$NAME.pid
#SCRIPTNAME=/etc/init.d/$NAME

$DAEMON $DAEMON_ARGS

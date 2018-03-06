#! /bin/bash

# script to locally try out a rebuilt pbm with same
# settins as used in pbm (usually run from /etc/init.d)

set -x

# PATH should only include /usr/* if it runs after the mountnfs.sh script
DESC="new power manager daemon"
NAME=pbmd
DAEMON=./$NAME
# Sleep duration 60s, default sleep 8pm, wake 8am, greedy mode
DAEMON_ARGS="60 GREEDY"
#PIDFILE=/var/run/$NAME.pid
#SCRIPTNAME=/etc/init.d/$NAME

$DAEMON $DAEMON_ARGS

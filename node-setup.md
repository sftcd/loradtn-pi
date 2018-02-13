
# Building the node...

This'll maybe evolve into a larger build guide, but we'll start where
we start...

- We're using a new [solar charge controller](http://www.allpowers.net/index.php?c=product&id=109)
- We'll likely use a USB GSM backhaul
- We may well use the Pi WiFi as a client and the maintainer's laptop as an AP 

# Initial playing with Pi and pbmd

- Installed a Pi from NOOBS
- Found a page about how to use the [phidgets with the Pi](http://www.instructables.com/id/Getting-Started-with-Phidgets-on-the-Raspberry-Pi/)
- That all just worked
- I cloned the [pbm mercurial repo](https://basil.dsg.cs.tcd.ie/code/n4c/pbm/)
- That also built fine, and more or less worked
- The pbmd power managment scripts (e.g. stbylong.sh) don't work with the Pi of course
  as the Pi doesn't pwoer off really - once connceted it's a battery killer:-)

# Getting the Pi/Witty-Pi setup...

- WittyPi-2 [user guide](http://www.uugear.com/doc/WittyPi2_UserManual.pdf)
- That says to not use NOOBS but to rather [directly install the OS](https://www.raspberrypi.org/documentation/installation/installing-images/README.md)

Next up... do the Pi/WittyPi install...

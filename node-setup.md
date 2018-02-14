
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
- That says to not use NOOBS but to rather [directly install the OS](https://www.raspberrypi.org/documentation/installation/installing-images/README.md), so we'll do that first...

## Installing the Pi

- I'm following the instructions [here](https://www.raspberrypi.org/documentation/installation/installing-images/linux.md)
- When I first did that, I tried using USB disk creator, which resulted in an "unused" 4MB 
  partition on the SD card before the "boot" partition, and that card didn't boot. 
- I had also re-zised the "rootfs" partition, to full size using gparted, but that's not
  needed it turns out (the rPi seems to detect when the SD card is inserted and re-sized
  the partition to full size itself)
- So I zapped the SD card using gparted's "create partition table", then did a quick
  format using the file manager (as FAT), and then used the dd command to write the
  image to the parition - for me that was: 

		sudo dd bs=4M if=2017-11-29-raspbian-stretch.img of=/dev/sdb status=progress conv=fsync

- The end result was the usual boot and rootfs partitions, with the latter being only 
  4.5GB in size - but on insertion to the Pi, the rootfs was resized (the message shown
  was misleading - it implied that someone else had re-sized when the Pi itself seems
  to have done that).
- End of the day, the Pi booted ok
- Connect to the wired network (via "samy's" hub) and since the image is from Nov 
  2017, there's a lot of updates to grab so...

		sudo apt update
		sudo apt upgrade

## WittyPi install

- Back to: WittyPi-2 [user guide](http://www.uugear.com/doc/WittyPi2_UserManual.pdf)
- That talks about Raspbian jessie but we're on stretch so we'll see if that makes
  a difference...
- We'll try run the installer from a code/wittypi directory
	- said yes to removing fake_hwclock
	- said yes to adding Qt GUI stuff, even though we shouldn't need it
	- after <1 min, it told me to reboot...did that via ```sudo reboot```
	  (first time I've rebooted a Pi that way)
	- got an error about "could not get clock", had to hit keyboard to continue (not sure?)
	- after reboot code/wittypi/WittyPi has the scripts...
	- shutdown again to mount wittypi on board...
	- gotta hit the wittypi power button after connecting to power
	- same clock error on boot, hit space again...
	- synced time from system -> RTC (as we're on n/w we have NTP)

## Phidgets build/install

Our existing pbmd uses phidgets (for voltmeter, 2xammeter and solid-state
relay). *TODO: explain those somewhere*

Building those was easy enough first time, hopefully also 2nd time:-)

- We're following [these](http://www.instructables.com/id/Getting-Started-with-Phidgets-on-the-Raspberry-Pi/) 
  instructions for building the phidgets library
- We'll build in code/phidgets...
	- there are some warnings we're ignoring for now - check those some day 

## Existing dodgy-pbm build...

Note that we hope to do away with this but for now here's HOWTO

- we need mercurial

		sudo apt install mercurial
		cd ~/code
		hg clone https://basil.dsg.cs.tcd.ie/code/n4c/pbm
		cd pbm
		make
		make ker-on ker-off # those aren't in the all target for some reason
		sudo make install

Now we should be ready to mount in the box and see what happens...

## Basic testing

Attach your new pi/wittypi usb power cable to the solar charger and the
phidgets USB to the pi and turn 'em on...

- shutdown the pi gracefully
- detact power from the wittypi
- attach the phidgets USB
- attach the USB power to the wittypi and solar charter
- hit the wittypi power button
- start the pbm test code...

		cd ~/code/pbm
		sudo ./trypbm.sh
		...output...


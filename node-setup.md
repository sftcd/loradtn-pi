
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



Next up... do the WittyPi install...

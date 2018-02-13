# loradtn-pi: Pi-based power management for a solar powered LoRa gateway (on an ex-DTN node:-)

- This is the successor design to the
  [loradtn](https://basil.dsg.cs.tcd.ie/code/tcd/loradtn) experiment started in January 2017. 
- That used the DTN node hardware from the [N4C project](https://basil.dsg.cs.tcd.ie/code/n4c/DTN2-N4C/)
	- The N4C project used to be described at http://www.n4c.eu/ but that domain seems to be a gonner at the moment.
	- There's a [detailed document](https://down.dsg.cs.tcd.ie/misc/n4c-wp5-052-dtn-node-build-12.pdf) 
	  (pdf generated in 2018, only found a .doc) about how to build the 2010 version of the DTN node.
	  That's still quite useful for wiring etc.
- We re-used the [power managment](https://basil.dsg.cs.tcd.ie/code/n4c/pbm/) code from N4C in 2017.
- The last of our 10-year old Proteus boards died at the end of the 2017 loradtn tests.
- So we're starting over with [Raspberry Pi](https://www.raspberrypi.org/), 
  combined with a [WittyPi-2](http://www.uugear.com/witty-pi-realtime-clock-power-management-for-raspberry-pi/) for 
  RTC and power management. 
- No doubt others bits'n'pieces will be needed as we go.
- We'll for sure want to replace the pretty-horrible power management daemon code, which worked, but is
  pretty comprehensively ickky, and well past it's use-by date. 

This repo will contain scripts, code and results related to all that.

Contacts: 

	- Stephen Farrell/stephen.farrell@cs.tcd.ie, will try hack the h/w bits together, undoubtedly
	  requiring help from
	- Kerry Hartnett/kerry@tolerantneworks.com, a colleague in Tolerant Networks Limited, but
	  hopefully with most of the work done by...  
	- Robert Cooney, a 4th year student, who'll be re-factoring the power management scheme/code
	  for his final year project.
	




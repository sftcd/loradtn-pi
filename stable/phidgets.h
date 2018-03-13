#ifndef STATE_MANAGER
#define STATE_MANAGER

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <phidget21.h>
#include <time.h>
#include <stdlib.h>
#include <syslog.h>
#include <stdio.h>
#include <phidget21.h>
#include "handlers.h"

#define INIT       0        // Initialised (first run or reset manually)
#define STBYLONG   1        // Standby 180 - reboot, check voltage, sleep 3 hours
#define STBYSHORT  2        // Standby 30 - boot, check voltage, sleep 30 mins
#define OVERRIDE   3        // manually started - run 30 mins then sleep
#define SLEEP      4        // Sleep during the night
#define UP         9        // Running normally - dont sleep

#define GREEN   1145        // Voltage level for normal running
#define AMBER   1135        // Voltage level at which we must go asleep

#define IFK_WAIT_TIME   5000    //  5 second wait time
#define LCD_WAIT_TIME   10000   // 10 second wait time

#define DUTINDEX    2 // Index of Device Under Test (DUT) ammeter
#define VOLTINDEX   1
#define AMPINDEX    0
                                        
//#define STBYLONG_TIME   240       // DEBUG time 4 min
//#define STBYSHORT_TIME  120       // DEBUG time 2 min
//#define OVERRIDE_TIME   120       // DEBUG time 2 min
#define STBYLONG_TIME     10800     // 3 hours
#define STBYSHORT_TIME    1800      // 30 min
#define OVERRIDE_TIME     1800      // 30 min

//#define SLEEPHOURSTART    22        // Time to start sleeping
//#define SLEEPHOUREND      6         // Time to end sleeping
//#define SLEEPTIMEENDSTR   "06:00"   // Time to end sleeping

#define SPIKELIMIT        20

#define BOOTFLAG_MAN_RESTART    "X"
#define BOOTFLAG_MAN_OVERRIDE   "N"
#define BOOTFLAG_NORMAL         "S"

// sleep modes, abs means e.g. 22:00 to 06:00, 
// rel means e.g. sleep 10 of every 30 (starting counting @ top of the hour)
#define SLEEPMODEABS		"ABS"
#define SLEEPMODEREL		"REL"
// if no sleepmode given then use ABS as default, but note it differently
#define SLEEPMODEDEF		"DEF"

void display_generic_properties(CPhidgetHandle phid);
void display_IFK_properties(CPhidgetInterfaceKitHandle phid); 
void display_LCD_properties(CPhidgetTextLCDHandle phid);

int getVoltage(CPhidgetInterfaceKitHandle IFK);
int getAmps(CPhidgetInterfaceKitHandle IFK);
int getDUTAmps(CPhidgetInterfaceKitHandle IFK);


void closeCPhidget(CPhidgetHandle handle);
int testVoltageSpike(int *prevVoltage,
	time_t *prevVoltageTime,
	int *voltage,
	time_t *voltageTime);

CPhidgetInterfaceKitHandle createInterfaceKit();
CPhidgetTextLCDHandle createLCDHandle();
void setupHandlers(CPhidgetInterfaceKitHandle IFK,CPhidgetTextLCDHandle LCD);

void openPhidgets(CPhidgetInterfaceKitHandle IFK,CPhidgetTextLCDHandle LCD);
int checkLCD(CPhidgetTextLCDHandle LCD, CPhidgetInterfaceKitHandle IFK);
void setStartupDisplay(CPhidgetTextLCDHandle LCD);
int checkIFK(CPhidgetInterfaceKitHandle IFK, CPhidgetTextLCDHandle LCD);
int phidgetInit();
void spikeError(int spikeCount, CPhidgetTextLCDHandle LCD, CPhidgetInterfaceKitHandle IFK);

int updateDisplay(int voltage, int amps, char *wakeTimeStr, char *stateDescription, CPhidgetTextLCDHandle LCD);



#endif
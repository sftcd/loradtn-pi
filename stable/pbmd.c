/*
    Handles logging of values - voltmeter, ammeter
    State machine controller
*/
 
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <phidget21.h>
#include <syslog.h>
#include "phidgets.h"

#define BATTERY_LOG "/var/log/battery-new.log" 
#define SNAP_LOG "/var/log/battery-snapshot-new.log" 
#define BOOTFLAGS "/etc/bootflag-new" 

#define SLEEP_STATE 0
#define UP_STATE 1

#define GREEDY_STR "GREEDY"
#define MODERATE_STR "MODERATE"

#define GREEDY_SLEEP 20
#define MODERATE_SLEEP 18
#define CONSERVATIVE_SLEEP 16
 
char *batteryLogLocation, *snapshotLocation, bootflag, *mode;// = "/var/log/battery.log";
 
int sleepDuration, sleepTime, wakeTime;

FILE *logFile; 

#ifndef min
#define min(a,b) (a<b?a:b)
#endif

int updateLogfile(FILE *logfile,
	int voltage,
	int amps,
	int ampsDUT,
	int state,
	int spiking,
	const char *progName);
int updateSnapshotfile(const char *snapFileName, int voltage, int amps,  int state, int spiking);
void getTimeString (int wakeTimeSec, char *wakeTimeStr);
int clearSnapFile(char* snapFileName);
char* getProgName(char* path);
int getSleepTime(char* mode);
char* getStateDesc(int state);


 
void init(int argc, char* argv[])
{
    logFile = fopen( BATTERY_LOG, "a" );
    clearSnapFile(SNAP_LOG);
}
 

 
 
int main(int argc, char* argv[])
{
    //parse args, init
    char *progName = getProgName(argv[0]);
	
    sleepDuration = atoi(argv[1]);
    mode = argv[2];
    
    init(argc, argv);

    //log
    fputs("\nYYYY-M-MDD,HH:MM:SS,mVolts,mAmps,State,Spike\n",logFile);
    
    int voltage = 0;
    int amps = 0;
    int ampsDUT = 0;
    int state = 1;

    time_t rawTime;
	struct tm * timeInfo;
    int sleepTime;
    //get voltage, amps, amps DUT
	int prevVoltage = 0;
	int newState = 0;
	int spiking;
	int spikeCount;
	
	time_t prevVoltageTime, voltageTime;
	
	
	char *wakeTimeStr = malloc(20);


	createInterfaceKit();
    createLCDHandle();
	
	/******************************************************************************************/
	
	setupHandlers();	// Set up handlers for the Interface Kit & TextLCD

	openPhidgets();

	//get the program to wait 10 seconds for an TextLCD device to be attached
    if( checkLCD() == -1 ){
        return -1;
    }
	
	//display_LCD_properties(LCD);	//Display the properties of the attached Text LCD device
	
	//Set the LCD startup screen  
    setStartupDisplay();
		
	
	//wait 5 seconds for attachment of the Interface Kit
	if( checkIFK() == -1){
		return -1;
	}
	//display_IFK_properties(IFK);	//Display the properties of the attached Interface Kit device
	
	
	//CPhidgetInterfaceKit_getSensorChangeTrigger (IFK, 0, &lightTrigger);
	
	/******************************************************************************************/
	
	time (&prevVoltageTime);			//initialise previous voltage time 
	prevVoltage = getVoltage();	  //ininitalise voltage		
		
	spiking = 1;
	spikeCount = 0;
	while ( spiking ) 
	{
		sleep(10);
		time (&voltageTime);
		voltage = getVoltage(); 
		spiking = testVoltageSpike(&prevVoltage, &prevVoltageTime, &voltage, &voltageTime);   
		
		if ( spiking ) 
			syslog ( LOG_ERR, "Initial voltage spike occured - acquiring new voltage to retest" );	
			
		if ( spikeCount++ > SPIKELIMIT ) 
		{
			spikeError(spikeCount);

			
			return 1;
		}		 
	}
    
    while(1){

		time (&voltageTime);
		//Voltage = getVoltage(IFK);			 
		spiking = testVoltageSpike(&prevVoltage, &prevVoltageTime, &voltage, &voltageTime);
		

		
		ampsDUT = getDUTAmps();
		amps = getAmps();

        updateLogfile(logFile, voltage, amps, ampsDUT, state, spiking, progName);
        updateSnapshotfile (SNAP_LOG, voltage, amps, state, spiking);

        //update state
        if(voltage < 1200){
            state = SLEEP_STATE;
        }
        else{
            state = UP_STATE;
        }



		getTimeString (0, wakeTimeStr);
		updateDisplay(voltage,amps, wakeTimeStr, getStateDesc(newState));
	
		// Do what the state demands		
		syslog ( LOG_DEBUG, "Begin main while loop: State is %s", getStateDesc(newState) ); 


       	//get current time
       	time ( &rawTime );
	   	timeInfo = localtime ( &rawTime );
       	sleepTime = getSleepTime(mode);

	   

       	//act based on state
       	if(state == SLEEP_STATE){
            if(timeInfo->tm_hour >= sleepTime){
                // sleep script
            } else {
				if(!strncmp(mode,GREEDY_STR,min(strlen(mode),strlen(GREEDY_STR)))){
                    //sleep short amount of time
				} else if(!strncmp(mode,MODERATE_STR,min(strlen(mode),strlen(MODERATE_STR)))){
                    //sleep moderate time
                } else {
                    //sleep longer
                }
            }
            return 0;
        }

        //get voltage, amps, amps DUT

        //log
        updateLogfile(logFile, voltage, amps, ampsDUT, state, spiking, progName);
        updateSnapshotfile (SNAP_LOG, voltage, amps, state, spiking);
        
        //sleepn
        sleep(sleepDuration);
    }
    
}

void getTimeString (int wakeTimeSec, char *wakeTimeStr)
{
	time_t rawtime;
	struct tm * timeinfo;

	time ( &rawtime );
	//rawtime = rawtime + wakeTimeSec+7200; // ToDo: change to properly display local time
	rawtime = rawtime + wakeTimeSec; // ToDo: change to properly display local time
	timeinfo = localtime ( &rawtime );

	strftime (wakeTimeStr, 20, "%H:%M", timeinfo);
	return;
}

int clearSnapFile(char *snapFileName)
{
	FILE *filesnap = fopen( snapFileName, "w" );   
	if (filesnap == 0) 
	{
		syslog ( LOG_ERR, "Could not open snapshot file: %s", snapFileName );
		return -1;
	}   
	fclose( filesnap ); // we need to overwrite it every time we write to it	
	return 0;
}

char* getStateDesc(int state)
{
    switch(state){
        case SLEEP_STATE:
            return "SLEEPING";
        default:
            return "UP";
    }
}

int updateSnapshotfile(const char *snapFileName, int voltage, int amps,  int state, int spiking)
{
	// Snapshot file is a single line file, overwritten each time with the current and latest values
	time_t rawtime;
	struct tm * timeinfo;
	char timeStr [80];
	char snapEntry [200];
	char * spikingStr = spiking ? " SPIKE" : "";
		
	time ( &rawtime );
	timeinfo = localtime ( &rawtime );
	strftime (timeStr, 80, "%Y-%m-%d,%H:%M:%S", timeinfo);

	FILE *snapfile = fopen( snapFileName, "w" );			// Open file (delete previous)
	if (snapfile == 0) {
		syslog ( LOG_ERR, "Could not open snapshot file: %s", snapFileName );
		return -1;	  
	} else {
		snprintf( snapEntry, 200, "%s %dV%s %dmA %s\n", timeStr, voltage, spikingStr, amps, getStateDesc(state) );
		fputs( snapEntry, snapfile );
		fflush( snapfile );
		fclose( snapfile );
		return 0;
	}		
}

int updateLogfile(FILE *logfile, int voltage, int amps, int dutAmps, int state, int spiking, const char *progName)
{   
	time_t rawTime;
	struct tm * timeInfo;
	char timeStr [80];
	char hostName [80];	
	char logEntry [200];
	char * spikingStr = spiking ? " SPIKE" : "";
	
	time ( &rawTime );
	timeInfo = localtime ( &rawTime );
	strftime (timeStr, 80, "%Y-%m-%d,%H:%M:%S", timeInfo);

	gethostname(hostName, 80);
	
	if (logfile == 0) {
		syslog ( LOG_ERR, "Error: log file not open" );
		return -1;
	} else {
		snprintf( logEntry, 200, "%s %s %s %d %d %s %s DUT: %d\n", 
			timeStr, hostName, progName, voltage, amps, getStateDesc(state), spikingStr, dutAmps );
		fputs( logEntry, logfile );
		fflush( logfile );
		return 0;
	}
}

char* getProgName(char* path)
{
	char* ret = strrchr( path, '/' );	   //returns null if '/' not found
	if( ret == NULL )
		return path;
	else
		return ret + 1;
}
 
int getSleepTime(char* mode)
{
    if(!strncmp(mode,GREEDY_STR,min(strlen(mode),strlen(GREEDY_STR)))){
        return GREEDY_SLEEP;
    } else if(!strncmp(mode,MODERATE_STR,min(strlen(mode),strlen(MODERATE_STR)))){
        return MODERATE_SLEEP;
    }
    return CONSERVATIVE_SLEEP;
}
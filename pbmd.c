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

#define BATTERY_LOG "/var/log/battery.log" 
#define SNAP_LOG "/var/log/battery-snapshot.log" 
#define BOOTFLAGS "/etc/bootflag" 

#define SLEEP_STATE 0
#define UP_STATE 1

#define GREEDY_SLEEP 20
#define MODERATE_SLEEP 18
#define CONSERVATIVE_SLEEP 16
 
char* batteryLogLocation, snapshotLocation, bootflags, mode;// = "/var/log/battery.log";
 
int sleepDuration, sleepTime, wakeTime;

FILE *logFile; 

int updateLogfile(FILE *logfile,
	int voltage,
	int amps,
	int ampsDUT,
	int state,
	int spiking,
	const char *progName);

int clearSnapFile(char* snapFileName);

char* getProgName(char* path)
{
	char* ret = strrchr( path, '/' );	   //returns null if '/' not found
	
	if( ret == NULL )
		return path;
	else
		return ret + 1;
}
 
void init(int argc, char* argv[]){
   
    

    logFile = fopen( BATTERY_LOG, "a" );
    clearSnapFile(SNAP_LOG);
}
 
int getSleepTime(char* mode){

    if(mode == "GREEDY"){
        return GREEDY_SLEEP;
    }
    else if(mode == "MODERATE"){
        return MODERATE_SLEEP;
    }
        
    return CONSERVATIVE_SLEEP;
}
 
 
 
int main(int argc, char* argv[]){
    //parse args, init
    char *progName = getProgName(argv[0]);
    sleepDuration = (int)argv[1];
    mode = argv[2];

    
    init(argc, argv);

    //log
    
    fputs("\nYYYY-M-MDD,HH:MM:SS,mVolts,mAmps,State,Spike\n",logFile);
    
    
    int voltage = 0;
    int amps = 0;
    int ampsDUT = 0;
    int spiking = 0;
    int state = 1;

    time_t rawTime;
	struct tm * timeInfo;
    int sleepTime;
    //get voltage, amps, amps DUT
    
    while(1){
        updateLogfile(logFile, voltage, amps, ampsDUT, state, spiking, progName);
        updateSnapshotfile (SNAP_LOG, voltage, amps, state, spiking);

        //update state
        if(voltage < 1200){
            state = SLEEP_STATE;
        }
        else{
            state = UP_STATE;
        }

        //get current time
        

       time ( &rawTime );
	   timeInfo = localtime ( &rawTime );
       sleepTime = getSleepTime(mode);
       

        //act based on state
        if(state == SLEEP_STATE){
            if(timeInfo->tm_hour >= sleepTime){
                // sleep script
            }

            else{
                if(mode == "GREEDY"){
                    //sleep short amount of time
                }
                else if(mode == "MODERATE"){
                    //sleep moderate time
                }
                else{
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

char* getStateDesc(int state){
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

	if (snapfile == 0)		
	{
		syslog ( LOG_ERR, "Could not open snapshot file: %s", snapFileName );
		return -1;	  
	}
	else 
	{
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
	
	if (logfile == 0)
	{
		syslog ( LOG_ERR, "Error: log file not open" );
		return -1;
	}
	else
	{
		snprintf( logEntry, 200, "%s %s %s %d %d %s %s DUT: %d\n", 
			timeStr, hostName, progName, voltage, amps, getStateDesc(state), spikingStr, dutAmps );

		fputs( logEntry, logfile );
		fflush( logfile );
		return 0;
	}
}
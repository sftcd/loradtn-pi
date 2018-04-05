/*
    Handles logging of values - voltmeter, ammeter
    State machine controller
*/
#define _GNU_SOURCE

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
#define LOW_POWER 2
#define OVERRIDE 3

#define GREEDY_STR "GREEDY"
#define MODERATE_STR "MODERATE"

// Duration of sleep, eg. Greedy sleep = 10 minutes
#define GREEDY_SLEEP 10
#define MODERATE_SLEEP 30
#define CONSERVATIVE_SLEEP 60

#define SECONDS_TO_MINUTES 60

#define SLEEP_DURATION 60

#define SIMULATE 0

CPhidgetTextLCDHandle LCD;
CPhidgetInterfaceKitHandle IFK;

char *batteryLogLocation, *snapshotLocation, bootflag, *mode; // = "/var/log/battery.log";

int sleepDuration, sleepTime, wakeTime, voltMode;

FILE *logFile;

#ifndef min
#define min(a, b) (a < b ? a : b)
#endif

int updateLogfile(FILE *logfile,
				  int voltage,
				  int amps,
				  int ampsDUT,
				  int state,
				  int spiking,
				  const char *progName);
int updateSnapshotfile(const char *snapFileName, int voltage, int amps, int state, int spiking);
void getTimeString(int wakeTimeSec, char *wakeTimeStr);
int clearSnapFile(char *snapFileName);
char *getProgName(char *path);
int getSleepTime(char *mode);
char *getStateDesc(int state);
int simulateVoltage(struct tm *time, int currentV);
int simulateAmps();
int simulateDUTAmps();
char *concat(const char *s1, const char *s2);
char *createStartupString(int timeToSleep);
char *createShutdownString(int timeToSleep);
int getSleepDuration(char *mode);
void kerOn();
void kerOff();

void init()
{
	logFile = fopen(BATTERY_LOG, "a");
	clearSnapFile(SNAP_LOG);
}

int main(int argc, char *argv[])
{
	//parse args, init
	char *progName = getProgName(argv[0]);

	//sleepDuration = atoi(argv[1]);
	mode = argv[1];
	voltMode = strtol(argv[2], NULL, 10);
	init();

	//log
	fputs("\nYYYY-M-MDD,HH:MM:SS,mVolts,mAmps,State,Spike\n", logFile);

	int voltage = 0;
	int amps = 0;
	int ampsDUT = 0;
	int state = 1;

	time_t rawTime;
	struct tm *timeInfo;
	//get voltage, amps, amps DUT
	int prevVoltage = 0;
	int newState = 1;
	int spiking;
	int spikeCount;
	time(&rawTime);
	timeInfo = localtime(&rawTime);
	time_t prevVoltageTime, voltageTime;

	char *wakeTimeStr = malloc(20);

	//phidgetInit();
	//printf("%d\n", voltMode);
	if (voltMode != SIMULATE)
	{
		IFK = createInterfaceKit();
		LCD = createLCDHandle();

		setupHandlers(IFK, LCD); // Set up handlers for the Interface Kit & TextLCD

		openPhidgets(IFK, LCD);

		//get the program to wait 10 seconds for an TextLCD device to be attached
		if (checkLCD(LCD, IFK) == -1)
		{
			return -1;
		}

		//display_LCD_properties(LCD);	//Display the properties of the attached Text LCD device

		//Set the LCD startup screen
		setStartupDisplay(LCD);

		//wait 5 seconds for attachment of the Interface Kit
		if (checkIFK(IFK, LCD) == -1)
		{
			return -1;
		}
		//display_IFK_properties(IFK);	//Display the properties of the attached Interface Kit device

		//CPhidgetInterfaceKit_getSensorChangeTrigger (IFK, 0, &lightTrigger);

		/******************************************************************************************/

		time(&prevVoltageTime);		   //initialise previous voltage time
		prevVoltage = getVoltage(IFK); //ininitalise voltage

		spiking = 1;
		spikeCount = 0;
		while (spiking)
		{
			sleep(10);
			time(&voltageTime);
			voltage = getVoltage(IFK);
			spiking = testVoltageSpike(&prevVoltage, &prevVoltageTime, &voltage, &voltageTime);

			if (spiking)
				syslog(LOG_ERR, "Initial voltage spike occured - acquiring new voltage to retest");

			if (spikeCount++ > SPIKELIMIT)
			{
				spikeError(spikeCount, LCD, IFK);

				return 1;
			}
		}
	}
	else
	{
		voltage = 1800;
	}

	while (1)
	{
		if (voltMode != SIMULATE)
		{
			time(&voltageTime);
			voltage = getVoltage(IFK);
			spiking = testVoltageSpike(&prevVoltage, &prevVoltageTime, &voltage, &voltageTime);

			ampsDUT = getDUTAmps(IFK);
			amps = getAmps(IFK);
		}
		else
		{
			voltage = simulateVoltage(timeInfo, voltage);
			amps = simulateAmps();
			ampsDUT = simulateDUTAmps();
			spiking = 0;
		}

		updateLogfile(logFile, voltage, amps, ampsDUT, state, spiking, progName);
		updateSnapshotfile(SNAP_LOG, voltage, amps, state, spiking);

		//update state
		if (voltage < 1000)
		{
			state = SLEEP_STATE;
		}
		else if (voltage < 1100)
		{
			state = LOW_POWER;
		}
		else
		{
			state = UP_STATE;
		}

		getTimeString(0, wakeTimeStr);
		if (voltMode != SIMULATE)
		{
			updateDisplay(voltage, amps, wakeTimeStr, getStateDesc(newState), LCD, mode);
		}
		// Do what the state demands
		syslog(LOG_DEBUG, "Begin main while loop: State is %s", getStateDesc(newState));
		syslog(LOG_DEBUG, "Mode is %s", mode);

		//get current time
		time(&rawTime);
		timeInfo = localtime(&rawTime);
		//sleepTime = getSleepTime(mode);

		//act based on state
		int sleepDuration = 0;
		if (state == SLEEP_STATE)
		{
			sleepDuration = getSleepDuration(mode);
			char *startupTime = createStartupString(sleepDuration);
			printf("%s\n", startupTime);
			system(startupTime);

			kerOff();

			sleep(SLEEP_DURATION / 2);
			system("./utilities.sh do_shutdown");
			return 0;
		}
		else if (state == LOW_POWER)
		{
			kerOff();
		}
		else if (state == OVERRIDE)
		{

			//run for 30(?) minutes then sleep script
			kerOn();
			sleepDuration = getSleepDuration(mode);
			char *shutdownTime = createShutdownString(30 * SECONDS_TO_MINUTES);
			char *startupTime = createStartupString((30 * SECONDS_TO_MINUTES) + sleepDuration);

			system(shutdownTime); // set shutdown and startup times
			system(startupTime);

			time(&rawTime);
			time_t time_wanted, currentTime;
			time_wanted = time(NULL) + (30 * SECONDS_TO_MINUTES);
			currentTime = time(NULL);
			while (1)
			{
				time(&rawTime);
				currentTime = time(NULL);
				if ((currentTime + (5 * SECONDS_TO_MINUTES)) > time_wanted)
				{
					kerOff();
				}

				if (voltMode != SIMULATE)
				{
					time(&voltageTime);
					voltage = getVoltage(IFK);
					spiking = testVoltageSpike(&prevVoltage, &prevVoltageTime, &voltage, &voltageTime);

					ampsDUT = getDUTAmps(IFK);
					amps = getAmps(IFK);
				}
				else
				{
					voltage = simulateVoltage(timeInfo, voltage);
					amps = simulateAmps();
					ampsDUT = simulateDUTAmps();
					spiking = 0;
				}

				//log
				updateLogfile(logFile, voltage, amps, ampsDUT, state, spiking, progName);
				updateSnapshotfile(SNAP_LOG, voltage, amps, state, spiking);

				//sleepn
				sleep(SLEEP_DURATION);
			}

			return 0;
		}
		else
		{

			//get voltage, amps, amps DUT
			if (voltMode != SIMULATE)
			{
				kerOn();
				time(&voltageTime);
				voltage = getVoltage(IFK);
				spiking = testVoltageSpike(&prevVoltage, &prevVoltageTime, &voltage, &voltageTime);

				ampsDUT = getDUTAmps(IFK);
				amps = getAmps(IFK);
			}
			else
			{
				voltage = simulateVoltage(timeInfo, voltage);
				amps = simulateAmps();
				ampsDUT = simulateDUTAmps();
				spiking = 0;
			}

			//log
			updateLogfile(logFile, voltage, amps, ampsDUT, state, spiking, progName);
			updateSnapshotfile(SNAP_LOG, voltage, amps, state, spiking);

			//sleepn
			sleep(SLEEP_DURATION);
		}
	}
}

void getTimeString(int wakeTimeSec, char *wakeTimeStr)
{
	time_t rawtime;
	struct tm *timeinfo;

	time(&rawtime);
	//rawtime = rawtime + wakeTimeSec+7200; // ToDo: change to properly display local time
	rawtime = rawtime + wakeTimeSec; // ToDo: change to properly display local time
	timeinfo = localtime(&rawtime);

	strftime(wakeTimeStr, 20, "%H:%M", timeinfo);
	return;
}

int clearSnapFile(char *snapFileName)
{
	FILE *filesnap = fopen(snapFileName, "w");
	if (filesnap == 0)
	{
		syslog(LOG_ERR, "Could not open snapshot file: %s", snapFileName);
		return -1;
	}
	fclose(filesnap); // we need to overwrite it every time we write to it
	return 0;
}

char *getStateDesc(int state)
{
	switch (state)
	{
	case SLEEP_STATE:
		return "SLEEPING";
	case LOW_POWER:
		return "LOW POWER";
	case OVERRIDE:
		return "OVERRIDE";
	default:
		return "UP";
	}
}

int updateSnapshotfile(const char *snapFileName, int voltage, int amps, int state, int spiking)
{
	// Snapshot file is a single line file, overwritten each time with the current and latest values
	time_t rawtime;
	time_t time_wanted;
	struct tm *timeInfo;
	char timeStr[80];
	char snapEntry[200];
	char *spikingStr = spiking ? " SPIKE" : "";

	time(&rawtime);
	timeInfo = localtime(&rawtime);
	if (timeInfo->tm_isdst > 0)
	{
		time_wanted = time(NULL) - (60 * 60);
		timeInfo = localtime(&time_wanted);
	}
	strftime(timeStr, 80, "%Y-%m-%d,%H:%M:%S", timeInfo);

	FILE *snapfile = fopen(snapFileName, "w"); // Open file (delete previous)
	if (snapfile == 0)
	{
		syslog(LOG_ERR, "Could not open snapshot file: %s", snapFileName);
		return -1;
	}
	else
	{
		snprintf(snapEntry, 200, "%s %dV%s %dmA %s\n", timeStr, voltage, spikingStr, amps, getStateDesc(state));
		fputs(snapEntry, snapfile);
		fflush(snapfile);
		fclose(snapfile);
		return 0;
	}
}

int updateLogfile(FILE *logfile, int voltage, int amps, int dutAmps, int state, int spiking, const char *progName)
{
	time_t rawTime, time_wanted;
	struct tm *timeInfo;
	char timeStr[80];
	char hostName[80];
	char logEntry[200];
	char *spikingStr = spiking ? " SPIKE" : "";

	time(&rawTime);
	timeInfo = localtime(&rawTime);
	if (timeInfo->tm_isdst > 0)
	{
		time_wanted = time(NULL) - (60 * 60);
		timeInfo = localtime(&time_wanted);
	}
	strftime(timeStr, 80, "%Y-%m-%d,%H:%M:%S", timeInfo);

	gethostname(hostName, 80);

	if (logfile == 0)
	{
		syslog(LOG_ERR, "Error: log file not open");
		return -1;
	}
	else
	{
		snprintf(logEntry, 200, "%s %s %s %d %d %s %s DUT: %d\n",
				 timeStr, hostName, progName, voltage, amps, getStateDesc(state), spikingStr, dutAmps);
		fputs(logEntry, logfile);
		fflush(logfile);
		return 0;
	}
}

char *getProgName(char *path)
{
	char *ret = strrchr(path, '/'); //returns null if '/' not found
	if (ret == NULL)
		return path;
	else
		return ret + 1;
}

int getSleepTime(char *mode)
{

	if (strncmp(mode, GREEDY_STR, min(strlen(mode), strlen(GREEDY_STR))) == 0)
	{
		return GREEDY_SLEEP;
	}
	else if (!strncmp(mode, MODERATE_STR, min(strlen(mode), strlen(MODERATE_STR))) == 0)
	{
		return MODERATE_SLEEP;
	}

	return CONSERVATIVE_SLEEP;
}

int simulateVoltage(struct tm *time, int currentV)
{
	int hour = time->tm_hour;
	if (hour > 20 || hour < 8)
	{
		if (currentV > 800)
		{
			return currentV - (rand() % 20);
		}
		else
		{
			return currentV;
		}
	}

	return currentV + ((rand() % 20) - 5);
}

int simulateAmps()
{
	return (rand() % 2) + 3;
}

int simulateDUTAmps()
{
	return (rand() % 2) + 3;
}

char *concat(const char *s1, const char *s2)
{
	char *result = malloc(strlen(s1) + strlen(s2) + 1); //+1 for the null-terminator
	//in real code you would check for errors in malloc here
	strcpy(result, s1);
	strcat(result, s2);
	return result;
}

char *createStartupString(int timeToSleep)
{
	time_t time_wanted;
	struct tm *timeInfo;
	time_wanted = time(NULL) + timeToSleep;

	timeInfo = localtime(&time_wanted);
	if (timeInfo->tm_isdst > 0)
	{
		time_wanted = time_wanted - (60 * 60);
		timeInfo = localtime(&time_wanted);
	}
	int hour = timeInfo->tm_hour;

	int minute = timeInfo->tm_min;
	char hourBuffer[2];
	char minuteBuffer[2];
	sprintf(hourBuffer, "%d", hour);
	sprintf(minuteBuffer, "%d", minute);
	char *startupTime = "./utilities.sh set_startup_time ?? ";
	startupTime = concat(startupTime, hourBuffer);
	startupTime = concat(startupTime, " ");
	startupTime = concat(startupTime, minuteBuffer);
	startupTime = concat(startupTime, " ");
	startupTime = concat(startupTime, "00");
	return startupTime;
}

char *createShutdownString(int timeToSleep)
{
	time_t time_wanted;
	struct tm *timeInfo;
	time_wanted = time(NULL) + timeToSleep;

	timeInfo = localtime(&time_wanted);
	if (timeInfo->tm_isdst > 0)
	{
		time_wanted = time_wanted - (60 * 60);
		timeInfo = localtime(&time_wanted);
	}
	int hour = timeInfo->tm_hour;

	int minute = timeInfo->tm_min;
	char hourBuffer[2];
	char minuteBuffer[2];
	sprintf(hourBuffer, "%d", hour);
	sprintf(minuteBuffer, "%d", minute);
	char *startupTime = "./utilities.sh set_startup_time ?? ";
	startupTime = concat(startupTime, hourBuffer);
	startupTime = concat(startupTime, " ");
	startupTime = concat(startupTime, minuteBuffer);
	startupTime = concat(startupTime, " ");
	startupTime = concat(startupTime, "00");
	return startupTime;
}

int getSleepDuration(char *mode)
{
	int sleepDuration = 0;
	if (strncmp(mode, GREEDY_STR, min(strlen(mode), strlen(GREEDY_STR))) == 0)
	{
		sleepDuration = GREEDY_SLEEP * SECONDS_TO_MINUTES;
	}
	else if (strncmp(mode, MODERATE_STR, min(strlen(mode), strlen(MODERATE_STR))) == 0)
	{
		sleepDuration = MODERATE_SLEEP * SECONDS_TO_MINUTES;
	}
	else
	{
		sleepDuration = CONSERVATIVE_SLEEP * SECONDS_TO_MINUTES;
	}
	return sleepDuration;
}

void kerOn()
{
	int ret;
	if ((ret = system("/usr/sbin/ker-on")) != 0)
	{
		syslog(LOG_ERR, "Failed to run ker-on. Error: %d", ret);
	}
	else
	{
		syslog(LOG_NOTICE, "Ran ker-on. ret: %d", ret);
	}
}

void kerOff()
{
	int ret;
	if ((ret = system("/usr/sbin/ker-off")) != 0)
	{
		syslog(LOG_ERR, "Failed to run ker-off. Error: %d", ret);
	}
	else
	{
		syslog(LOG_NOTICE, "Ran ker-off. ret: %d", ret);
	}
}
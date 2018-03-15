#include "phidgets.h"

#ifndef PATH_MAX
#define PATH_MAX 1024
#endif

#define LCDINDEX 0
#define IFKINDEX 0
#define BUFFER_SIZE 21



char topBuffer [BUFFER_SIZE];
char bottomBuffer [BUFFER_SIZE];

void display_generic_properties(CPhidgetHandle phid)
{
	int serNum, version;
	const char *deviceType;
	
	CPhidget_getDeviceType(phid, &deviceType);
	CPhidget_getSerialNumber(phid, &serNum);
	CPhidget_getDeviceVersion(phid, &version);

	printf("%s\n", deviceType);
	printf("Version: %8d \nSerialNumber: %10d\n", version, serNum);
	
	return;
}

void display_IFK_properties(CPhidgetInterfaceKitHandle phid)
{
	int numInputs, numOutputs, numSensors;	
	
	CPhidgetInterfaceKit_getOutputCount( phid, &numOutputs);
	CPhidgetInterfaceKit_getInputCount ( phid, &numInputs);
	CPhidgetInterfaceKit_getSensorCount( phid, &numSensors);	
	CPhidgetInterfaceKit_setOutputState( phid, 0, 1);
	
	display_generic_properties((CPhidgetHandle)phid);
	
	printf("Sensors:%d Inputs:%d Outputs:%d\n", numSensors, numInputs, numOutputs);
	
	return;
}

void display_LCD_properties(CPhidgetTextLCDHandle phid)
{
	int numRows, numColumns, backlight, cursor, contrast, cursor_blink;

	CPhidgetTextLCD_getRowCount (phid, &numRows);
	CPhidgetTextLCD_getColumnCount (phid, &numColumns);
	CPhidgetTextLCD_getBacklight (phid, &backlight);
	CPhidgetTextLCD_getContrast (phid, &contrast);
	CPhidgetTextLCD_getCursorOn (phid, &cursor);
	CPhidgetTextLCD_getCursorBlink (phid, &cursor_blink);

	display_generic_properties((CPhidgetHandle) phid);
	
	printf("# Rows: %d\n# Columns: %d\n", numRows, numColumns);
	printf("Current Contrast Level: %d\nBacklight Status: %d\n", contrast, backlight);
	printf("Cursor Status: %d\nCursor Blink Status: %d\n", cursor, cursor_blink);
	
	return;
}

int getVoltage(CPhidgetInterfaceKitHandle IFK)
{
	float correctedValue = 0, totalValue = 0;
	int currentValue = 0, i = 0;

	CPhidgetInterfaceKit_setRatiometric(IFK, PFALSE);				// set to non-ratio
	sleep(2);														// wait at least 800ms to settle
	CPhidgetInterfaceKit_getSensorRawValue(IFK, VOLTINDEX, &currentValue);  // Voltmeter

	// Calibration
	// when the phidget is not connected to anything it returns a voltage of -5V. 
	// this must be adjusted to zero by adding 5V
	currentValue += 5;

	// get an average over 5 readings to allow for sensor noise. 1 per second for 5 seconds
	for (i = 0; i < 5 ; i++)
	{ 
		// http://www.phidgets.com/documentation/Phidgets/1123.pdf
		// If you want maximum accuracy, you can use the RawSensorValue property from the PhidgetInterfaceKit. 
		// To adjust a formula, substitute (SensorValue) with (RawSensorValue / 4.095)
		//
		// The Formula to translate SensorValue into Voltage is:
		// Voltage (in volts) = (SensorValue x 0.06) - 30		
		correctedValue = currentValue;
		correctedValue = (((correctedValue/4.095)*0.06)-30)*100;	// multiply raw value and up by 100 to keep
		totalValue += correctedValue;
		sleep (1);
	}
			
	return totalValue / 5;
}

int getAmps(CPhidgetInterfaceKitHandle IFK)
{
	float correctedValue;
	int value;
	CPhidgetInterfaceKit_setRatiometric (IFK, PTRUE);				// set to ratiometric
	sleep(2);														// wait at least 800ms
	CPhidgetInterfaceKit_getSensorRawValue(IFK, AMPINDEX, &value);   // Ampmeter
	// Calibration
	// when the phidget is not connected to anything it returns a voltage of -2mA. 
	// this must be adjusted to zero by adding 2mA
	value += 2;
	// http://www.phidgets.com/documentation/Phidgets/1122.pdf
	// If you want maximum accuracy, you can use the RawSensorValue property from the PhidgetInterfaceKit. 
	// To adjust a formula, substitute (SensorValue) with (RawSensorValue / 4.095)
	//
	// The formula to translate SensorValue into Current is:
	// DC Amps = (SensorValue / 13.2) - 37.8787	
	correctedValue = value;
	correctedValue = (((correctedValue/4.095)/13.2)-37.8787)*1000;  // multiply raw value to get amps
	return correctedValue * -1;							 // invert so + is charging and - is amp draw
}

int getDUTAmps(CPhidgetInterfaceKitHandle IFK)
{
	float correctedValue;
	int value;
	CPhidgetInterfaceKit_setRatiometric (IFK, PTRUE);				// set to ratiometric
	sleep(2);														// wait at least 800ms
	CPhidgetInterfaceKit_getSensorRawValue(IFK, DUTINDEX, &value);   // Ampmeter
	// Calibration
	// when the phidget is not connected to anything it returns a voltage of -2mA. 
	// this must be adjusted to zero by adding 2mA
	value += 2;
	// http://www.phidgets.com/documentation/Phidgets/1122.pdf
	// If you want maximum accuracy, you can use the RawSensorValue property from the PhidgetInterfaceKit. 
	// To adjust a formula, substitute (SensorValue) with (RawSensorValue / 4.095)
	//
	// The formula to translate SensorValue into Current is:
	// DC Amps = (SensorValue / 13.2) - 37.8787	
	correctedValue = value;
	correctedValue = (((correctedValue/4.095)/13.2)-37.8787)*1000;  // multiply raw value to get amps
	return correctedValue ;	
}



void closeCPhidget(CPhidgetHandle handle)
{
	CPhidget_close(handle);
	CPhidget_delete(handle);
	return;
}		 

//if the rate of change in voltage is greater than the max allowable, a spike has occured
//if spike:  set voltage & voltageTime to prev value and return 1
//otherwise: update prevVoltage to voltage and return 0
int testVoltageSpike(int *prevVoltage, time_t *prevVoltageTime, int *voltage, time_t *voltageTime)
{
	const float maxRateOfChange = 1;
	
	float deltaSeconds = (*voltageTime) - (*prevVoltageTime);	
	float deltaVoltage = (*voltage) - (*prevVoltage); 
	deltaVoltage = deltaVoltage < 0 ? -1 * deltaVoltage : deltaVoltage;	 //use abs value
		
	// eg. if the voltage changes by more than 1 volt over 100 sec a spike has occured
	if( deltaVoltage / deltaSeconds > maxRateOfChange )
	{
		syslog ( LOG_ERR, "Voltage spike from: %dV to: %dV in %d seconds. Using prevVoltage: %dV", *prevVoltage, *voltage, (int)deltaSeconds, *prevVoltage ); 
		*voltage=*prevVoltage;
		*voltageTime=*prevVoltageTime; 
		return 1;		   
	}  
	else 
	{
		*prevVoltage = *voltage;
		*prevVoltageTime = *voltageTime;			
		return 0;
	}
}

int phidgetInit()
{
	CPhidgetInterfaceKitHandle IFK = IFKINDEX;
	CPhidgetInterfaceKit_create(&IFK);
	CPhidgetTextLCDHandle LCD = LCDINDEX;
	CPhidgetTextLCD_create (&LCD);
	setHandlers(IFK, LCD);
	CPhidget_open((CPhidgetHandle)IFK, -1);
	CPhidget_open((CPhidgetHandle)LCD, -1);
	syslog ( LOG_DEBUG, "Waiting %d seconds for LCD to be attached...", LCD_WAIT_TIME/1000 );	
	int err;
	if( (err = CPhidget_waitForAttachment((CPhidgetHandle)LCD, LCD_WAIT_TIME)) != EPHIDGET_OK )
	{
		const char *errStr;
		CPhidget_getErrorDescription(err, &errStr);
		syslog ( LOG_ERR, "Error waiting for LCD attachment: %d %s", err, errStr ); 
		
		closeCPhidget((CPhidgetHandle)IFK);
		closeCPhidget((CPhidgetHandle)LCD);
		return -1;
	}
	
	snprintf (topBuffer, BUFFER_SIZE, "Welcome to loradtn");
	snprintf (bottomBuffer, BUFFER_SIZE, "Loading...");

	CPhidgetTextLCD_setDisplayString (LCD, 0, topBuffer); 
	CPhidgetTextLCD_setDisplayString (LCD, 1, bottomBuffer);	

	syslog ( LOG_DEBUG, "Waiting %d seconds for IFK to be attached...", IFK_WAIT_TIME/1000 );
	if( (err = CPhidget_waitForAttachment((CPhidgetHandle)IFK, IFK_WAIT_TIME)) != EPHIDGET_OK )
	{
		const char *errStr;
		CPhidget_getErrorDescription( err, &errStr );
		syslog ( LOG_ERR, "Error waiting for IFK attachment: %d %s", err, errStr );		   

		closeCPhidget((CPhidgetHandle)IFK);
		closeCPhidget((CPhidgetHandle)LCD);
		return -1;
	}
	return 0;
}

CPhidgetInterfaceKitHandle createInterfaceKit()
{
	CPhidgetInterfaceKitHandle IFK = IFKINDEX;
	CPhidgetInterfaceKit_create(&IFK);
	return IFK;
}

CPhidgetTextLCDHandle createLCDHandle(){
	CPhidgetTextLCDHandle LCD = LCDINDEX;
	CPhidgetTextLCD_create (&LCD);
	return LCD;
}

void setupHandlers(CPhidgetInterfaceKitHandle IFK,CPhidgetTextLCDHandle LCD)
{
	setHandlers(IFK, LCD);
}

void openPhidgets(CPhidgetInterfaceKitHandle IFK,CPhidgetTextLCDHandle LCD)
{
	CPhidget_open((CPhidgetHandle)IFK, -1);
	CPhidget_open((CPhidgetHandle)LCD, -1);
}

int checkLCD(CPhidgetTextLCDHandle LCD, CPhidgetInterfaceKitHandle IFK)
{
	syslog ( LOG_DEBUG, "Waiting %d seconds for LCD to be attached...", LCD_WAIT_TIME/1000 );	
	int err;
	if( (err = CPhidget_waitForAttachment((CPhidgetHandle)LCD, LCD_WAIT_TIME)) != EPHIDGET_OK )
	{
		const char *errStr;
		CPhidget_getErrorDescription(err, &errStr);
		syslog ( LOG_ERR, "Error waiting for LCD attachment: %d %s", err, errStr ); 
		
		closeCPhidget((CPhidgetHandle)IFK);
		closeCPhidget((CPhidgetHandle)LCD);
		return -1;
	}
	return err;
}		

void setStartupDisplay(CPhidgetTextLCDHandle LCD) 
{
	snprintf (topBuffer, BUFFER_SIZE, "Welcome to loradtn");
	snprintf (bottomBuffer, BUFFER_SIZE, "Loading...");

	CPhidgetTextLCD_setDisplayString (LCD, 0, topBuffer); 
	CPhidgetTextLCD_setDisplayString (LCD, 1, bottomBuffer);	
}		 

int checkIFK(CPhidgetInterfaceKitHandle IFK, CPhidgetTextLCDHandle LCD) 
{
	syslog ( LOG_DEBUG, "Waiting %d seconds for IFK to be attached...", IFK_WAIT_TIME/1000 );
	int err;
	if( (err = CPhidget_waitForAttachment((CPhidgetHandle)IFK, IFK_WAIT_TIME)) != EPHIDGET_OK )
	{
		const char *errStr;
		CPhidget_getErrorDescription( err, &errStr );
		syslog ( LOG_ERR, "Error waiting for IFK attachment: %d %s", err, errStr );		   

		closeCPhidget((CPhidgetHandle)IFK);
		closeCPhidget((CPhidgetHandle)LCD);
		return -1;
	}
	return err;
}

void spikeError(int spikeCount, CPhidgetTextLCDHandle LCD, CPhidgetInterfaceKitHandle IFK) 
{
	syslog ( LOG_ERR, "Error initial voltage spike continuing after %d tests, closing program", spikeCount );
						   
	snprintf (topBuffer, BUFFER_SIZE, "Voltage Spike");
	snprintf (bottomBuffer, BUFFER_SIZE, "Power App Restart");

	CPhidgetTextLCD_setDisplayString (LCD, 0, topBuffer); 
	CPhidgetTextLCD_setDisplayString (LCD, 1, bottomBuffer);  
			
	closeCPhidget((CPhidgetHandle)IFK);
	closeCPhidget((CPhidgetHandle)LCD); 
}

int updateDisplay(int voltage, int amps, char *wakeTimeStr, char *stateDescription, CPhidgetTextLCDHandle LCD, char* mode) 
{ 
	snprintf (topBuffer, BUFFER_SIZE, "V:%4d A:%-5d %s", voltage, amps,wakeTimeStr);
	snprintf (bottomBuffer, BUFFER_SIZE, "%-s , %s", stateDescription, mode);

	CPhidgetTextLCD_setDisplayString (LCD, 0, topBuffer);
	CPhidgetTextLCD_setDisplayString (LCD, 1, bottomBuffer);

	return 0;
}
		


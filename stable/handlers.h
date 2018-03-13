#ifndef HANDLERS
#define HANDLERS

#include <stdio.h>
#include <phidget21.h>

int IFK_DetachHandler(CPhidgetHandle IFK, void *userptr);
int IFK_ErrorHandler(CPhidgetHandle IFK, void *userptr, int ErrorCode, const char *unknown);
int IFK_OutputChangeHandler(CPhidgetInterfaceKitHandle IFK, void *userptr, int Index, int Value);
int IFK_InputChangeHandler(CPhidgetInterfaceKitHandle IFK, void *userptr, int Index, int Value);
int IFK_SensorChangeHandler(CPhidgetInterfaceKitHandle IFK, void *userptr, int Index, int Value);
int IFK_AttachHandler(CPhidgetHandle IFK, void *userptr);
int LCD_AttachHandler(CPhidgetHandle IFK, void *userptr);
int LCD_DetachHandler(CPhidgetHandle IFK, void *userptr);
int LCD_ErrorHandler(CPhidgetHandle IFK, void *userptr, int ErrorCode, const char *unknown);
void setHandlers(CPhidgetInterfaceKitHandle IFK, CPhidgetTextLCDHandle LCD);

#endif

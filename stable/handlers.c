#include "handlers.h"

int IFK_DetachHandler(CPhidgetHandle IFK, void *userptr)
{
    printf("IFK Detach handler ran!\n");
    return 0;
}

int IFK_ErrorHandler(CPhidgetHandle IFK, void *userptr, int ErrorCode, const char *unknown)
{
    printf("IFK Error handler ran!\n");
    return 0;
}

int IFK_OutputChangeHandler(CPhidgetInterfaceKitHandle IFK, void *userptr, int Index, int Value)
{
    printf("Output %d is %d\n", Index, Value);
    return 0;
}

int IFK_InputChangeHandler(CPhidgetInterfaceKitHandle IFK, void *userptr, int Index, int Value)
{
    printf("Input %d is %d\n", Index, Value);
    return 0;
}

int IFK_SensorChangeHandler(CPhidgetInterfaceKitHandle IFK, void *userptr, int Index, int Value)
{
    //todo: this function doesn't appear to do anything

    float fValue;

    if (Index == 0)
    {
        fValue = Value;                        // Sensor 0 is Amps - adj accordingly
        fValue = (fValue*13.2)-37.8787;        // (SensorValue / 13.2) - 37.8787
        Value = fValue;
    }                

    return 0;
}

int IFK_AttachHandler(CPhidgetHandle IFK, void *userptr)
{
    CPhidgetInterfaceKit_setSensorChangeTrigger((CPhidgetInterfaceKitHandle)IFK, 0, 0);
    printf("IFK Attach handler ran!\n");
    return 0;
}

int LCD_AttachHandler(CPhidgetHandle IFK, void *userptr)
{
    printf("LCD Attach handler ran!\n");
    return 0;
}

int LCD_DetachHandler(CPhidgetHandle IFK, void *userptr)
{
    printf("LCD Detach handler ran!\n");
    return 0;
}

int LCD_ErrorHandler(CPhidgetHandle IFK, void *userptr, int ErrorCode, const char *unknown)
{
    printf("LCD Error handler ran!\n");
    return 0;
}

void setHandlers(CPhidgetInterfaceKitHandle IFK, CPhidgetTextLCDHandle LCD)
{
    CPhidget_set_OnAttach_Handler((CPhidgetHandle)IFK, IFK_AttachHandler, NULL);
    CPhidget_set_OnDetach_Handler((CPhidgetHandle)IFK, IFK_DetachHandler, NULL);
    CPhidget_set_OnError_Handler((CPhidgetHandle)IFK, IFK_ErrorHandler, NULL);

    CPhidget_set_OnAttach_Handler((CPhidgetHandle)LCD, LCD_AttachHandler, NULL);
    CPhidget_set_OnDetach_Handler((CPhidgetHandle)LCD, LCD_DetachHandler, NULL);
    CPhidget_set_OnError_Handler((CPhidgetHandle)LCD, LCD_ErrorHandler, NULL);
} 


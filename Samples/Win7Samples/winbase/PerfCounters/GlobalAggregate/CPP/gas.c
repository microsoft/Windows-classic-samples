/*++

THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
PARTICULAR PURPOSE.

Copyright (c) Microsoft Corporation. All rights reserved.

Module Name:
    
    gas.c

Abstract:

    This module contains sample code to demonstrate how to provide
    counter data from a executalbe file with globalAggregate.

Environment:

    User mode

--*/


#include <windows.h>
#include <perflib.h>
#include <winperf.h>
#include <stdio.h>
#include <conio.h>
#include <strsafe.h>
#include <math.h>
#include "gasCounters.h"

#define M_PI 3.14159265358979323846 
#define TIME_INTERVAL 1000
#define AMPLITUDE 30.0

ULONG
CreateInstance(
    PPERF_COUNTERSET_INSTANCE *ObjectInstance
    )
/*++

Routine Description:

    Create instance for the provider. 

Arguments:

    A pointer whcih is for receiving the instance.

Return Value:

    Standard ULONG Status indicating if the instance was properly created.

--*/
{

    //
    // Create the instances for a single instance counter set.
    //

    *ObjectInstance = PerfCreateInstance(GlobalAggregateSample, & TrignometricWaveGuid, L"GlobalAggregate", 0);
    if (*ObjectInstance == NULL) {
        return GetLastError();
    }
    return ERROR_SUCCESS;
}
    
ULONG
RunSample(
    VOID
    )
/*++

Routine Description:

    Execute the sample.

Arguments:

    None.

Return Value:

    Standard ULONG Status indicating if sample were running properly.

--*/
{
    double Angle;
    ULONG Base;
    ULONG Cosine;
    ULONG Degree;
    ULONG NaturalNumbers;
    PPERF_COUNTERSET_INSTANCE ObjectInstance;
    ULONG Status;
    ULONG Sine;

    //
    // Call CounterInitialize();
    // CounterInitialize() is created by ctrpp.exe and is present in gasCounters.h
    // CounterInitialize starts the provider and initializes the counter set.
    // In this sample we have a single instance counter set.
    //

    Status = CounterInitialize(NULL,NULL,NULL,NULL);
    if (Status != ERROR_SUCCESS) {
        return Status;
    }

    Status = CreateInstance(&ObjectInstance);
    if (Status != ERROR_SUCCESS){
        goto Cleanup;
    }

    Status = PerfSetCounterRefValue(GlobalAggregateSample, ObjectInstance, 1, & Sine);
    if (Status != ERROR_SUCCESS) {
        goto Cleanup;
    }

    Status = PerfSetCounterRefValue(GlobalAggregateSample, ObjectInstance, 2, & Cosine);
    if (Status != ERROR_SUCCESS) {
        goto Cleanup;
    }

    Base = 50;
    Status = PerfSetCounterRefValue(GlobalAggregateSample, ObjectInstance, 3, & Base);
    if (Status != ERROR_SUCCESS) {
        goto Cleanup;
    }

    Status = PerfSetULongCounterValue(GlobalAggregateSample, ObjectInstance, 4, Base);
    if (Status != ERROR_SUCCESS) {
        goto Cleanup;
    }
    
    NaturalNumbers = 1;
    Status = PerfSetCounterRefValue(GlobalAggregateSample, ObjectInstance, 5, &NaturalNumbers);
    if (Status != ERROR_SUCCESS) {
        goto Cleanup;
    }

    Degree = 0;

    printf("\tPress any key to quit\n");

    while (!_kbhit()) {

        //
        // Increment the Degree value to between 0 - 360
        //
        
        Degree = (Degree + 10) % 360;
        
        //
        // Increment the Natural Number counter. Set it to 1 if we reach 100
        //
        
        NaturalNumbers = ++NaturalNumbers % 100;

        //
        // Set raw counter data for SingleInstanceCounterSet
        //
        
        Angle   = (((double) Degree) * M_PI) / (180.00);
        Sine   = Base + (ULONG) (AMPLITUDE * sin(Angle));
        Cosine = Base + (ULONG) (AMPLITUDE * cos(Angle));
        printf("\tSingleInstance(%d,%d,%d,%d)\n", Sine, Cosine, Base, Degree);

        Sleep(TIME_INTERVAL);
    }
    printf(" Provider finished\n");

Cleanup:

    CounterCleanup();
    return Status;
}

int 
__cdecl wmain(
    VOID
    )
/*++

Routine Description:

    Starting point of the V2 provider executable.
    1. Initializes the provider 
    2. Creates instacnes for the counter sets
    3. Sets the counter values for the object's counters in counter 
        set iteratively sleeps for 1 second between iterations. 

Arguments:

    Default arguments to wmain.

Return Value:

    Standard ULONG Status of perflib V2 provider API.

--*/
{
    ULONG Status = ERROR_SUCCESS;

    Status = RunSample();

    return Status;
}

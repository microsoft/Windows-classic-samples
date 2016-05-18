/*++

THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
PARTICULAR PURPOSE.

Copyright (c) Microsoft Corporation. All rights reserved.

Module Name:
    
    ucs.c

Abstract:

    This module contains sample code to demonstrate how to provide
    counter data from a executalbe file with multipleAggregate.

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
#include "ucsCounters.h"

#define M_PI 3.14159265358979323846 
#define TIME_INTERVAL 1000
#define AMPLITUDE 30.0
#define ALTITUDE 30

ULONG
CreateInstance(
    PPERF_COUNTERSET_INSTANCE *Object1Instance1, 
    PPERF_COUNTERSET_INSTANCE *Object1Instance2, 
    PPERF_COUNTERSET_INSTANCE *Object1Instance3,
    PPERF_COUNTERSET_INSTANCE *Object2Instance
    )
/*++

Routine Description:

    Create Instances for the Provider.

Arguments:

    Pointers whcih are for receiving the instances.

Return Value:

    Standard ULONG Status indicating if instances were properly created.

--*/
{

    //
    // Create the instances for multiple instance counter set.
    //

    *Object1Instance1 = PerfCreateInstance(UserModeCountersSample, & GeometricWaveGuid, L"Instance_1", 0);
    if (*Object1Instance1 == NULL) {
        return GetLastError();
    }

    *Object1Instance2 = PerfCreateInstance(UserModeCountersSample, & GeometricWaveGuid, L"Instance_2", 0);
    if (*Object1Instance2 == NULL) {
        return GetLastError();
    }

    *Object1Instance3 = PerfCreateInstance(UserModeCountersSample, & GeometricWaveGuid, L"Instance_3", 0);
    if (*Object1Instance2 == NULL) {
        return GetLastError();
    }

    //
    // Create the instances for a single instance counter set.
    //

    *Object2Instance = PerfCreateInstance(UserModeCountersSample, & TrignometricWaveGuid, L"_Default", 0);
    if (*Object2Instance == NULL) {
        return GetLastError();
    }
    return ERROR_SUCCESS;
}

ULONG
UpdataGeometricWave(
    PPERF_COUNTERSET_INSTANCE Object,
    ULONG MinimalValue,
    ULONG Degree
    )
/*++

Routine Description:

    Generate the triangle wave and square wave for the counters.

Arguments:

    Object - Point to the object which needs update.

    MinimalValue - The minimal value for the counters.

    Degree - According to this value to generate the date for the counters.

Return Value:

    Standard ULONG Status indicating if instances were properly updated.

--*/
{
    ULONG High;
    ULONG Increase; 
    ULONG Status;
    
    High = ((Degree % 180) > 90) ? ALTITUDE : -ALTITUDE;
    Status = PerfSetULongCounterValue(UserModeCountersSample, Object, 2, MinimalValue + High);
    if (Status != ERROR_SUCCESS){
        return Status;
    }

    Increase = (Degree < 180) ? Degree : 360 - Degree;
    Increase = (ULONG)((double)ALTITUDE / 180 * Increase);
    Status = PerfSetULongCounterValue(UserModeCountersSample, Object, 1, MinimalValue + Increase);
    if (Status != ERROR_SUCCESS){
        return Status;
    }

    return Status;
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
    PPERF_COUNTERSET_INSTANCE Object1Instance1;
    PPERF_COUNTERSET_INSTANCE Object1Instance2;
    PPERF_COUNTERSET_INSTANCE Object1Instance3;
    PPERF_COUNTERSET_INSTANCE Object2Instance;
    ULONG Status;
    ULONG Sine;

    //
    // Call CounterInitialize();
    // CounterInitialize() is created by ctrpp.exe and is present in ucsCounter.h
    // CounterInitialize starts the provider and initializes the counter sets.
    // In this sample we have a multiple instance counter set and a single instance counter set.
    //

    Status = CounterInitialize( NULL, NULL, NULL, NULL);
    if (Status != ERROR_SUCCESS) {
        return Status;
    }

    Status = CreateInstance(&Object1Instance1, &Object1Instance2, &Object1Instance3, &Object2Instance);
    if (Status != ERROR_SUCCESS){
        goto Cleanup;
    }

    //
    // Set raw counter data for SingleInstanceCounterSet.
    //

    Status = PerfSetCounterRefValue(UserModeCountersSample, Object2Instance, 1, & Sine);
    if (Status != ERROR_SUCCESS) {
        goto Cleanup;
    }

    Status = PerfSetCounterRefValue(UserModeCountersSample, Object2Instance, 2, & Cosine);
    if (Status != ERROR_SUCCESS) {
        goto Cleanup;
    }

    Base = 50;
    Status = PerfSetCounterRefValue(UserModeCountersSample, Object2Instance, 3, & Base);
    if (Status != ERROR_SUCCESS) {
        goto Cleanup;
    }

    Status = PerfSetULongCounterValue(UserModeCountersSample, Object2Instance, 4, Base);
    if (Status != ERROR_SUCCESS) {
        goto Cleanup;
    }

    NaturalNumbers = 1;
    Status = PerfSetCounterRefValue(UserModeCountersSample, Object2Instance, 5, &NaturalNumbers);
    if (Status != ERROR_SUCCESS) {
        goto Cleanup;
    }

    Degree = 0;
    
    printf("\tPress any key to quit\n");

    while (!_kbhit()) {
        
        //
        // Increment the Degree value to between 0 - 360.
        //

        Degree = (Degree + 10) % 360;
        
        //
        // Increment the Natural Number counter. Set it to 1 if we reach 100.
        //

        NaturalNumbers = ++NaturalNumbers % 100;

        //
        // Set raw counter data for SingleInstanceCounterSet.
        //

        Angle   = (((double) Degree) * M_PI) / (180.00);
        Sine   = Base + (ULONG) (AMPLITUDE * sin(Angle));
        Cosine = Base + (ULONG) (AMPLITUDE * cos(Angle));

        Status = UpdataGeometricWave(Object1Instance1, 30, Degree);
        if (Status != ERROR_SUCCESS) {
            break;
        }

        Status = UpdataGeometricWave(Object1Instance2, 50, Degree);
        if (Status != ERROR_SUCCESS) {
            break;
        }

        Status = UpdataGeometricWave(Object1Instance3, 80, Degree);
        if (Status != ERROR_SUCCESS) {
            break;
        }

        //
        //Sleep for 1 second before iterating once again to change the counter values.
        //

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
    1. Initializes the provider.
    2. Creates instacnes for the counter sets.
    3. Sets the counter values for all the objects' counters in both counter 
        sets iteratively sleeps for 1 second between iterations. 

Arguments:

    Default arguments to wmain.

Return Value:

    Standard ULONG Status of perflib V2 provider API.

--*/
{
    ULONG Status;
    
    Status = RunSample();

    return Status;
}

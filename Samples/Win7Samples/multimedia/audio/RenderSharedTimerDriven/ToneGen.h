// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//

//
//  Sine tone generator.
//
#include "stdafx.h"

#define _USE_MATH_DEFINES
#include <math.h>
#include <limits.h>

template<typename T> T Convert(double Value);



//
//  Generate samples which represent a sine wave that fits into the specified buffer.  
//
//  T:  Type of data holding the sample (short, int, byte, float)
//  Buffer - Buffer to hold the samples
//  BufferLength - Length of the buffer.
//  ChannelCount - Number of channels per audio frame.
//  SamplesPerSecond - Samples/Second for the output data.
//  InitialTheta - Initial theta value - start at 0, modified in this function.
//
template <typename T>
void GenerateSineSamples(BYTE *Buffer, size_t BufferLength, DWORD Frequency, WORD ChannelCount, DWORD SamplesPerSecond, double *InitialTheta)
{
    double sampleIncrement = (Frequency * (M_PI*2)) / (double)SamplesPerSecond;
    T *dataBuffer = reinterpret_cast<T *>(Buffer);
    double theta = (InitialTheta != NULL ? *InitialTheta : 0);

    for (size_t i = 0 ; i < BufferLength / sizeof(T) ; i += ChannelCount)
    {
        double sinValue = sin( theta );
        for(size_t j = 0 ;j < ChannelCount; j++)
        {
            dataBuffer[i+j] = Convert<T>(sinValue);
        }
        theta += sampleIncrement;
    }

    if (InitialTheta != NULL)
    {
        *InitialTheta = theta;
    }
}



//
//  Convert from double to float, byte, short or int32.
//
template<> 
float Convert<float>(double Value)
{
    return (float)(Value);
};

template<> 
short Convert<short>(double Value)
{
    return (short)(Value * _I16_MAX);
};


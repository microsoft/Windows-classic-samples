//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

//
//  Sine tone generator.
//
#include "pch.h"

#define _USE_MATH_DEFINES
#include <math.h>
#include <limits>

// Convert from [-1.0...+1.0] to sample format.
template<typename T>
T Convert(double Value)
{
    using limits = std::numeric_limits<T>;
    if constexpr (limits::is_integer)
    {
        // Scale to [-limits::max() .. +limits::max() ]
        static_assert(limits::is_signed);
        return static_cast<T>(Value * (limits::max)());
    }
    else
    {
        // Floating point types all use the range [-1.0 .. +1.0]
        return static_cast<T>(Value);
    }
}



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
void GenerateSineSamples(BYTE* Buffer, size_t BufferLength, DWORD Frequency, WORD ChannelCount, DWORD SamplesPerSecond, double* InitialTheta)
{
    double sampleIncrement = (Frequency * (M_PI * 2)) / (double)SamplesPerSecond;
    T* dataBuffer = reinterpret_cast<T*>(Buffer);
    double theta = (InitialTheta != NULL ? *InitialTheta : 0);

    for (size_t i = 0; i < BufferLength / sizeof(T); i += ChannelCount)
    {
        double sinValue = sin(theta);
        for (size_t j = 0; j < ChannelCount; j++)
        {
            dataBuffer[i + j] = Convert<T>(sinValue);
        }
        theta += sampleIncrement;
    }

    if (InitialTheta != NULL)
    {
        *InitialTheta = theta;
    }
}

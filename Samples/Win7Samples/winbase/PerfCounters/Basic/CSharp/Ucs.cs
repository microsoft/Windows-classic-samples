// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

using System;
using System.Threading;
using System.Collections.Generic;
using System.Diagnostics.PerformanceData;

namespace Microsoft.Sdk.Samples.PerformanceData.Ucs
{
    /// <summary>
    /// This class demonstrates providing performance counters from a .Net application using System.Diagnostics.PerformanceData namespace.
    /// The functionality of this sample is identical to the native code Ucs sample.
    /// NOTE that setting CounterData.Value is not thread-safe.
    /// </summary>
    static class Ucs
    {
        private const float M_PI = 3.14159265358979323846F;
        private const int TIME_INTERVAL = 1000;
        private const float AMPLITUDE = 30.0F;
        private const uint ALTITUDE = 30;
        private const uint BASE = 50;

        static void Main(string[] args)
        {
            // Initialize the provider and counters

            Guid providerId = new Guid("{5AE84FD4-BF72-49c4-936E-A7473237C338}");

            Guid geometricWavesCounterSetId = new Guid("{F7DC6E2D-9A3F-4239-AC8D-28DCE96CCA98}");
            CounterSet geometricWavesCounterSet = new CounterSet(providerId, geometricWavesCounterSetId, CounterSetInstanceType.MultipleAggregate);
            geometricWavesCounterSet.AddCounter(1, CounterType.RawData32); //"Triangle Wave"
            geometricWavesCounterSet.AddCounter(2, CounterType.RawData32); //"Square Wave"
            CounterSetInstance geomCsInstance1 = geometricWavesCounterSet.CreateCounterSetInstance("Instance_1");
            CounterSetInstance geomCsInstance2 = geometricWavesCounterSet.CreateCounterSetInstance("Instance_2");
            CounterSetInstance geomCsInstance3 = geometricWavesCounterSet.CreateCounterSetInstance("Instance_3");

            Guid trigWavesCounterSetId = new Guid("{F89A016D-A5D1-4ce2-8489-D5612FDD2C6F}");
            CounterSet trigWavesCounterSet = new CounterSet(providerId, trigWavesCounterSetId, CounterSetInstanceType.Single);
            trigWavesCounterSet.AddCounter(1, CounterType.RawData32); //"Sine Wave"
            trigWavesCounterSet.AddCounter(2, CounterType.RawData32); //"Cosine Wave"
            trigWavesCounterSet.AddCounter(3, CounterType.RawData32); //"Constant Value"
            trigWavesCounterSet.AddCounter(4, CounterType.RawBase32); //"Constant Number"
            trigWavesCounterSet.AddCounter(5, CounterType.RawFraction32); //"Raw Fraction"                
            CounterSetInstance trigCsInstance = trigWavesCounterSet.CreateCounterSetInstance("_Default");

            // Initialize variables used in counter calculations.
            UInt32 Degree = 0;                        
            UInt32 Base = BASE;               
            UInt32 NaturalNumbers = 1;
            double Angle = 0;
            UInt32 Sine = 0;
            UInt32 Cosine = 0;

            // Set the constant counter value.
            trigCsInstance.Counters[4].Value = BASE;

            Console.WriteLine("\tPress any key to quit");
            while (!Console.KeyAvailable)
            {  
                // Increment the Degree value to between 0 - 360.
                Degree = (Degree + 10) % 360;
                
                // Increment the Natural Number counter. Set it to 1 if we reach 100.
                NaturalNumbers = ++NaturalNumbers % 100;

                Angle   = (((double) Degree) * M_PI) / (180.00);
                Sine   = Base + (UInt32) (AMPLITUDE * Math.Sin(Angle));
                Cosine = Base + (UInt32)(AMPLITUDE * Math.Cos(Angle));

                // Set raw counter data for SingleInstanceCounterSet.
                UpdataGeometricWave(geomCsInstance1, 30, Degree);
                UpdataGeometricWave(geomCsInstance2, 50, Degree);
                UpdataGeometricWave(geomCsInstance3, 80, Degree);

                //Update TrigonometricWave counters
                trigCsInstance.Counters[1].Value = Sine;
                trigCsInstance.Counters[2].Value = Cosine;
                trigCsInstance.Counters[3].Value = Base;
                trigCsInstance.Counters[5].Value = NaturalNumbers;                   

                //Sleep for 1 second before iterating once again to change the counter values.
                Thread.Sleep(TIME_INTERVAL);
            }
        }

        /// <summary>
        /// Helper function to generate the triangle wave and square wave for the counters.
        /// </summary>
        /// <param name="CsInstance"> CounterSetInstance object which needs the value update.</param>
        /// <param name="MinimalValue"> The minimal value for the counters.</param>
        /// <param name="Degree"> Generate the data according to this value.</param>
        static void UpdataGeometricWave(CounterSetInstance CsInstance, UInt32 MinimalValue, UInt32 Degree)       
        {
            long High;
            UInt32 Increase;       

            High = ((Degree % 180) > 90) ? ALTITUDE : -ALTITUDE;
            CsInstance.Counters[2].Value = MinimalValue + High; //"Square Wave"
            
            Increase = (Degree < 180) ? Degree : 360 - Degree;
            Increase = (UInt32)((double)ALTITUDE / 180 * Increase);
            CsInstance.Counters[1].Value = MinimalValue + Increase; //"Triangle Wave"           
        }
    }

}

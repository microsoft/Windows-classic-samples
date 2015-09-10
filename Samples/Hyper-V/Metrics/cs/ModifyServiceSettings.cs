// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft. All rights reserved.

namespace Microsoft.Samples.HyperV.Metrics
{
    using System;
    using System.Globalization;
    using System.Management;
    using Microsoft.Samples.HyperV.Common;
    
    static class ModifyServiceSettingsSample
    {
        /// <summary>
        /// Configures the MetricsFlushInterval, which determines how frequently metric data should
        /// be flushed to disk.
        /// </summary>
        /// <param name="interval">The interval to configure.</param>
        internal static void
        ConfigureMetricsFlushInterval(
            TimeSpan interval)
        {
            ManagementScope scope = new ManagementScope(@"root\virtualization\v2");

            //
            // Create an instance of the Msvm_MetricServiceSettingData class and set the specified
            // metrics flush interval. Note that the TimeSpan must be converted to a DMTF time
            // interval first.
            //
            string dmtfTimeInterval = ManagementDateTimeConverter.ToDmtfTimeInterval(interval);

            string serviceSettingDataEmbedded;

            using (ManagementClass serviceSettingDataClass = new ManagementClass(
                "Msvm_MetricServiceSettingData"))
            {
                serviceSettingDataClass.Scope = scope;

                using (ManagementObject serviceSettingData = serviceSettingDataClass.CreateInstance())
                {
                    serviceSettingData["MetricsFlushInterval"] = dmtfTimeInterval;

                    serviceSettingDataEmbedded = serviceSettingData.GetText(TextFormat.WmiDtd20);
                }
                
            }
            
            //
            // Call the Msvm_MetricService::ModifyServiceSettings method. Note that the 
            // Msvm_MetricServiceSettingData instance must be passed as an embedded instance.
            //
            using (ManagementObject metricService = MetricUtilities.GetMetricService(scope))
            {
                using (ManagementBaseObject inParams = 
                    metricService.GetMethodParameters("ModifyServiceSettings"))
                {
                    inParams["SettingData"] = serviceSettingDataEmbedded;

                    using (ManagementBaseObject outParams = 
                        metricService.InvokeMethod("ModifyServiceSettings", inParams, null))
                    {
                        WmiUtilities.ValidateOutput(outParams, scope);

                        Console.WriteLine(string.Format(CultureInfo.CurrentCulture,
                            "The MetricsFlushInterval was successfully configured to interval \"{0}\".",
                            interval.ToString()));
                    }
                }
            }
        }
    }
}

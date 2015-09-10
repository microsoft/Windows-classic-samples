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

    enum MetricEnabledState
    {
        Unknown = 0,
        Enabled = 2,
        Disabled = 3,
        PartiallyEnabled = 32768
    };

    static class EnumerateMetricsSample
    {
        /// <summary>
        /// Queries whether the metric collection is enabled for a given virtual machine and metric
        /// definition.
        /// </summary>
        /// <param name="name">The name of the virtual machine.</param>
        internal static void
        QueryMetricCollectionEnabledForVirtualMachine(
            string name)
        {
            ManagementScope scope = new ManagementScope(@"root\virtualization\v2");

            //
            // Retrieve the Msvm_ComputerSystem and the CIM_BaseMetricDefinition derived instance
            // that we want to query the MetricCollectionEnabled state for.
            //
            SelectQuery metricDefForMeQuery;
            const string metricDefinitionName = "Aggregated Average CPU Utilization";

            using (ManagementObject vm = WmiUtilities.GetVirtualMachine(name, scope))
            using (ManagementObject metricDefinition = 
                MetricUtilities.GetMetricDefinition(metricDefinitionName, scope))
            {
                //
                // Build the WQL query used to retrieve the Msvm_MetricDefForME association between 
                // these two objects. It is the one that contains the MetricCollectionEnabled 
                // property.
                //
                string metricDefForMeQueryWql = string.Format(CultureInfo.InvariantCulture,
                    "SELECT * FROM Msvm_MetricDefForME WHERE Antecedent=\"{0}\" AND Dependent=\"{1}\"",
                    WmiUtilities.EscapeObjectPath(vm.Path.Path),
                    WmiUtilities.EscapeObjectPath(metricDefinition.Path.Path));

                metricDefForMeQuery = new SelectQuery(metricDefForMeQueryWql);
            }

            using (ManagementObjectSearcher metricDefForMeSearcher = 
                new ManagementObjectSearcher(scope, metricDefForMeQuery))
            using (ManagementObjectCollection metricDefForMeCollection = 
                metricDefForMeSearcher.Get())
            {
                //
                // There will always only be one Msvm_MetricDefForME for a given managed element and 
                // metric definition.
                //
                if (metricDefForMeCollection.Count != 1)
                {
                    throw new ManagementException(string.Format(CultureInfo.CurrentCulture,
                        "A single Msvm_MetricDefForME could not be found for virtual machine " +
                        "\"{0}\" and metric definition \"{1}\"", name, metricDefinitionName));
                }

                foreach (ManagementObject metricDefForMe in metricDefForMeCollection)
                {
                    using (metricDefForMe)
                    {
                        string metricCollectionState = Enum.Parse(typeof(MetricEnabledState),
                            metricDefForMe["MetricCollectionEnabled"].ToString()).ToString();

                        Console.WriteLine("MetricCollectionEnabled = {0}", metricCollectionState);
                    }
                }
            }
        }

        /// <summary>
        /// Enumerates the discrete metrics that compose a given metric for a virtual machine.
        /// </summary>
        /// <param name="name">The name of the virtual machine.</param>
        internal static void
        EnumerateDiscreteMetricsForVm(
            string name)
        {
            ManagementScope scope = new ManagementScope(@"root\virtualization\v2");

            //
            // Retrieve the first aggregate metric associated with this virtual machine.
            //
            using (ManagementObject vm = WmiUtilities.GetVirtualMachine(name, scope))
            using (ManagementObjectCollection vmMetricCollection = 
                vm.GetRelated("Msvm_AggregationMetricValue",
                    "Msvm_MetricForME",
                    null, null, null, null, false, null))
            using (ManagementObject vmAggregateMetric =
                WmiUtilities.GetFirstObjectFromCollection(vmMetricCollection))
            {
                //
                // Enumerate the discrete metrics that compose that aggregate metric.
                //
                using (ManagementObjectCollection discreteMetricCollection = vmAggregateMetric.GetRelated(
                    null, "Msvm_MetricCollectionDependency", null, null, null, null, false, null))
                {
                    foreach (ManagementObject discreteMetric in discreteMetricCollection)
                        using (discreteMetric)
                    {
                        Console.WriteLine(
                            "Discrete Metric Value:\t{0}", discreteMetric["MetricValue"]);
                    }
                }
            }
        }

        /// <summary>
        /// Enumerate the metrics for a given resource pool.
        /// </summary>
        /// <param name="resourceType">The resource type of the resource pool.</param>
        /// <param name="resourceSubType">The resource subtype of the resource pool.</param>
        /// <param name="poolId">The pool id the resource pool.</param>
        internal static void 
        EnumerateMetricsForResourcePool(
            string resourceType,
            string resourceSubType,
            string poolId)
        {
            ManagementScope scope = new ManagementScope(@"root\virtualization\v2");

            //
            // Retrieve the resource pool that we want to get metrics for and iterate through the 
            // metric definitions that are supported by it.
            //
            using (ManagementObject pool = 
                WmiUtilities.GetResourcePool(resourceType, resourceSubType, poolId, scope))
            using (ManagementObjectCollection metricDefinitionCollection = 
                pool.GetRelated("Msvm_AggregationMetricDefinition",
                    "Msvm_MetricDefForME",
                    null, null, null, null, false, null))
            using (ManagementObjectCollection metricValueCollection =
                pool.GetRelated("Msvm_AggregationMetricValue",
                    "Msvm_MetricForME",
                    null, null, null, null, false, null))
            {
                foreach (ManagementObject metricDefinition in metricDefinitionCollection)
                using (metricDefinition)
                {
                    Console.WriteLine("Metric Definition:\t{0}", metricDefinition["ElementName"]);

                    //
                    // For each supported metric definition, retrieve the corresponding metric value.
                    //
                    string id = metricDefinition["Id"].ToString();

                    foreach (ManagementObject metricValue in metricValueCollection)
                    using (metricValue)
                    {
                        string metricDefinitionId = metricValue["MetricDefinitionId"].ToString();

                        if (metricDefinitionId == id)
                        {
                            Console.WriteLine("Metric Value:\t\t{0}", metricValue["MetricValue"]);
                            break;
                        }
                    }
                }
            }
        }
    }
}
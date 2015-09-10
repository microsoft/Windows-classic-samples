// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft. All rights reserved.

namespace Microsoft.Samples.HyperV.Networking
{
    using System;
    using System.Management;
    using System.Globalization;
    using Microsoft.Samples.HyperV.Common;

    static class DeleteSwitchSample
    {
        /// <summary>
        /// Deletes the specified switch. Note that this can be used to delete any type of switch. 
        /// Any ports connected to external or internal resources are also deleted.
        /// </summary>
        /// <param name="switchName">The name of the switch to delete.</param>
        static void
        DeleteSwitch(
            string switchName)
        {
            ManagementScope scope = new ManagementScope(@"root\virtualization\v2");
            
            //
            // Find the switch that we want to delete.
            //
            using (ManagementObject ethernetSwitch = NetworkingUtilities.FindEthernetSwitch(switchName, scope))

            //
            // Now that we have the switch object we can delete it.
            //
            using (ManagementObject switchService = NetworkingUtilities.GetEthernetSwitchManagementService(scope))
            using (ManagementBaseObject inParams = switchService.GetMethodParameters("DestroySystem"))
            {
                inParams["AffectedSystem"] = ethernetSwitch.Path.Path;
    
                using (ManagementBaseObject outParams = switchService.InvokeMethod("DestroySystem", inParams, null))
                {
                    WmiUtilities.ValidateOutput(outParams, scope);
                }
            }

            Console.WriteLine(string.Format(CultureInfo.CurrentCulture,
                "The switch '{0}' was deleted successfully.", switchName));
        }

        /// <summary>
        /// Entry point for the DeleteSwitch sample.
        /// </summary>
        /// <param name="args">The command line arguments.</param>
        internal static void
        ExecuteSample(
            string[] args)
        {
            if (args.Length != 1 || (args.Length > 0 && args[0] == "/?"))
            {
                Console.WriteLine("Usage: DeleteSwitch SwitchName\n");
                Console.WriteLine("Example: DeleteSwitch MySwitch");
                return;
            }

            try
            {
                DeleteSwitch(args[0]);
            }
            catch (Exception ex)
            {
                Console.WriteLine("Failed to delete the switch. Error message details:\n");
                Console.WriteLine(ex.Message);
            }
        }
    }
}

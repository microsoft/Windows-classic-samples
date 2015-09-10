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

    static class ModifySwitchSample
    {
        /// <summary>
        /// Modifies the name and notes of an existing switch. We also show how to modify the 
        /// IOVPreferred property.
        /// </summary>
        /// <param name="existingSwitchName">The current name of the switch to change.</param>
        /// <param name="newSwitchName">The new name for the switch.</param>
        /// <param name="newNotes">The new notes to apply to the switch.</param>
        static void
        ModifySwitchName(
            string existingSwitchName,
            string newSwitchName,
            string newNotes)
        {
            ManagementScope scope = new ManagementScope(@"root\virtualization\v2");
            
            //
            // Get the switch we want to modify.
            //
            using (ManagementObject ethernetSwitch = NetworkingUtilities.FindEthernetSwitch(existingSwitchName, scope))

            //
            // To make modifications to the switch, we need to modify its configuration object. 
            //
            using (ManagementObject ethernetSwitchSettings = WmiUtilities.GetFirstObjectFromCollection(
                ethernetSwitch.GetRelated("Msvm_VirtualEthernetSwitchSettingData",
                    "Msvm_SettingsDefineState",
                    null, null, null, null, false, null)))
            {
                //
                // Modify its properties as desired.
                //
                ethernetSwitchSettings["ElementName"] = newSwitchName;
                ethernetSwitchSettings["Notes"] = new string[] { newNotes };

                //
                // This is how you can modify the IOV Preferred property from its current value.  
                // We leave this commented out because it could potentially fail for a system that doesn't
                // support IOV.
                //
                // ethernetSwitchSettings["IOVPreferred"] = !((bool)ethernetSwitchSettings["IOVPreferred"]);

                //
                // Now call the ModifySystemSettings method to apply the changes.
                //
                using (ManagementObject switchService = NetworkingUtilities.GetEthernetSwitchManagementService(scope))
                using (ManagementBaseObject inParams = switchService.GetMethodParameters("ModifySystemSettings"))
                {
                    inParams["SystemSettings"] = ethernetSwitchSettings.GetText(TextFormat.WmiDtd20);
    
                    using (ManagementBaseObject outParams = 
                           switchService.InvokeMethod("ModifySystemSettings", inParams, null))
                    {
                        WmiUtilities.ValidateOutput(outParams, scope);
                    }
                }
            }

            Console.WriteLine(string.Format(CultureInfo.CurrentCulture,
                "The switch '{0}' was renamed to '{1}' successfully.",
                existingSwitchName, newSwitchName));
        }

        /// <summary>
        /// Entry point for the ModifySwitch sample.
        /// </summary>
        /// <param name="args">The command line arguments.</param>
        internal static void
        ExecuteSample(
            string[] args)
        {
            if ((args.Length != 2 && args.Length != 3) || (args.Length > 0 && args[0] == "/?"))
            {
                Console.WriteLine("Usage: ModifySwitch SwitchName NewSwitchName [Notes]\n");
                Console.WriteLine("Example: ModifySwitch MySwitch MyModifiedSwitch \"I updated this switch\"");
                return;
            }

            try
            {
                string notes = (args.Length == 3) ? args[2] : null;
                ModifySwitchName(args[0], args[1], notes);
            }
            catch (Exception ex)
            {
                Console.WriteLine("Failed to modify the switch. Error message details:\n");
                Console.WriteLine(ex.Message);
            }
        }
    }
}

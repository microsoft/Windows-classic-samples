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

    static class ManageExtensionSample
    {
        const UInt16 CimEnabledStateEnabled = 2;
        const UInt16 CimEnabledStateDisabled = 3;
        const UInt16 CimEnabledStateNotApplicable = 5;

        /// <summary>
        /// Modifies the name and notes of an existing switch. We also show how to modify the 
        /// IOVPreferred property.
        /// </summary>
        /// <param name="switchName">The name of the switch on which the extension will be enabled or disabled.</param>
        /// <param name="extensionsName">The name of the extension to enable or disable.</param>
        /// <param name="enabled">The new enabled state of the extension.</param>
        static void
        SetExtensionEnabledState(
            string switchName,
            string extensionName,
            bool enabled)
        {
            bool found = false;
            
            ManagementScope scope = new ManagementScope(@"root\virtualization\v2");

            //
            // Get the switch we want to modify.
            //
            using (ManagementObject ethernetSwitch = NetworkingUtilities.FindEthernetSwitch(switchName, scope))

            //
            // To make modifications to the switch, we need to modify its configuration object. 
            //
            using (ManagementObjectCollection extensions = ethernetSwitch.GetRelated(
                "Msvm_EthernetSwitchExtension",
                "Msvm_HostedEthernetSwitchExtension",
                null,
                null,
                null,
                null,
                false,
                null))
            {
                foreach (ManagementObject extension in extensions)
                using (extension)
                {
                    if (!String.Equals(
                        (string)extension["ElementName"],
                        extensionName,
                        StringComparison.OrdinalIgnoreCase))
                    {
                        continue;
                    }

                    found = true;

                    if ((UInt16)extension["EnabledState"] == CimEnabledStateNotApplicable)
                    {
                        Console.WriteLine("The enabled state of the specified extension cannot be changed.");
                        continue;
                    }

                    if (((UInt16)extension["EnabledState"] == CimEnabledStateEnabled) && enabled)
                    {
                        Console.WriteLine("The specified extension is already enabled.");
                        continue;
                    }

                    if (((UInt16)extension["EnabledState"] == CimEnabledStateDisabled) && !enabled)
                    {
                        Console.WriteLine("The specified extension is already disabled.");
                        continue;
                    }

                    using (ManagementBaseObject inParams = extension.GetMethodParameters("RequestStateChange"))
                    {
                        inParams["RequestedState"] = enabled ? CimEnabledStateEnabled : CimEnabledStateDisabled;

                        using (ManagementBaseObject outParams =
                            extension.InvokeMethod("RequestStateChange", inParams, null))
                        {
                            WmiUtilities.ValidateOutput(outParams, scope);

                            Console.WriteLine(string.Format(
                                CultureInfo.CurrentCulture,
                                "Extension '{0}' was successfully {1} on switch '{2}'.",
                                extensionName,
                                enabled ? "enabled" : "disabled",
                                switchName));
                        }
                    }
                }
            }

            if (!found)
            {
                throw new ApplicationException(String.Format(
                    CultureInfo.CurrentCulture,
                    "Could not find extension '{0}' on switch '{1}'.",
                    extensionName,
                    switchName));
            }
        }

        static void
        MoveExtension(
            string switchName,
            string extensionName,
            int offset)
        {
            ManagementScope scope = new ManagementScope(@"root\virtualization\v2");
            
            //
            // Get the switch we want to modify.
            //
            using (ManagementObject ethernetSwitch = NetworkingUtilities.FindEthernetSwitch(switchName, scope))

            //
            // To make modifications to the switch, we need to modify its configuration object. 
            //
            using (ManagementObject ethernetSwitchSettings = WmiUtilities.GetFirstObjectFromCollection(
                ethernetSwitch.GetRelated("Msvm_VirtualEthernetSwitchSettingData",
                    "Msvm_SettingsDefineState",
                    null, null, null, null, false, null)))
            {
                string[] extensionOrder = (string[])ethernetSwitchSettings["ExtensionOrder"];
                byte[] extensionTypes = new byte[extensionOrder.Length];
                int extensionIndex = -1;                

                for (int idx = 0; idx < extensionOrder.Length; ++idx)
                {
                    using (ManagementObject extension = new ManagementObject(extensionOrder[idx]))
                    {
                        extension.Get();

                        extensionTypes[idx] = (byte)extension["ExtensionType"];

                        if (String.Equals(
                            (string)extension["ElementName"],
                            extensionName,
                            StringComparison.OrdinalIgnoreCase))
                        {
                            extensionIndex = idx;
                        }
                    }
                }

                if (extensionIndex == -1)
                {
                    throw new ApplicationException(String.Format(
                        CultureInfo.CurrentCulture,
                        "Could not find extension '{0}' on switch '{1}'.",
                        extensionName,
                        switchName));
                }
                
                //
                // Ensure that this is a valid move operation. The new extension index must
                // be in range, and the extension cannot be interleaved with other extensions of
                // a different type.
                //
                int newExtensionIndex = extensionIndex + offset;

                if ((newExtensionIndex < 0) || (newExtensionIndex >= extensionOrder.Length))
                {
                    throw new ApplicationException("Invalid move operation.");
                }

                if (extensionTypes[newExtensionIndex] != extensionTypes[extensionIndex])
                {
                    throw new ApplicationException("Invalid move operation.");
                }

                //
                // Reorder the extensions and apply the switch settings changes.
                //
                string temp = extensionOrder[newExtensionIndex];
                extensionOrder[newExtensionIndex] = extensionOrder[extensionIndex];
                extensionOrder[extensionIndex] = temp;

                ethernetSwitchSettings["ExtensionOrder"] = extensionOrder;

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

            Console.WriteLine(string.Format(
                CultureInfo.CurrentCulture,
                "Extension '{0}' was successfully reordered on switch '{1}'.",
                extensionName,
                switchName));
        }

        /// <summary>
        /// Displays usage information for this sample.
        /// </summary>
        internal static void
        ShowUsage()
        {
            Console.WriteLine(
                "Usage: ManageExtension SwitchName ExtensionName (Enable/Disable/MoveUp/MoveDown)\n" +
                "\n" +
                "Example: ManageExtension MySwitch \"NDIS Capture LightWeight Filter\" Enable");
        }

        /// <summary>
        /// Entry point for the ManageExtension sample.
        /// </summary>
        /// <param name="args">The command line arguments.</param>
        internal static void
        ExecuteSample(
            string[] args)
        {
            if ((args.Length != 3) || (args.Length > 0 && args[0] == "/?"))
            {
                ShowUsage();
                return;
            }

            try
            {
                string switchName = args[0];
                string extensionName = args[1];
                string operation = args[2].ToLowerInvariant();

                switch (operation)
                {
                    case "enable":
                        SetExtensionEnabledState(switchName, extensionName, true);
                        break;

                    case "disable":
                        SetExtensionEnabledState(switchName, extensionName, false);
                        break;

                    case "moveup":
                        MoveExtension(switchName, extensionName, -1);
                        break;

                    case "movedown":
                        MoveExtension(switchName, extensionName, 1);
                        break;

                    default:
                        ShowUsage();
                        break;
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine("Failed to modify switch extension. Error message details:\n");
                Console.WriteLine(ex.Message);
            }
        }
    }
}

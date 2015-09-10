// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft. All rights reserved.

namespace Microsoft.Samples.HyperV.Slp
{
    using System;
    using System.Management;
    using Microsoft.Samples.HyperV.Common;

    class Slp
    {
        private UInt32 SummaryInformationAvailableMemoryBuffer = 113;
        private UInt32 SummaryInformationSwapFilesInUse = 121;

        /// <summary>
        /// Gets the VM's memory status.
        /// </summary>
        /// <param name="hostMachine">The host name of the computer on which
        /// the VM is running.</param>
        /// <param name="vmName">The VM name.</param>
        public
        void
        GetVmMemoryStatus(
            string hostMachine,
            string vmName
            )
        {
            ManagementScope scope = new ManagementScope(
                @"\\" + hostMachine + @"\root\virtualization\v2", null);

            // Get the memory information.
            using (ManagementObject service = WmiUtilities.GetVirtualMachineManagementService(scope))
            using (ManagementObject vm = WmiUtilities.GetVirtualMachine(vmName, scope))
            using (ManagementObject vmSettings = WmiUtilities.GetVirtualMachineSettings(vm))
            using (ManagementBaseObject inParams = service.GetMethodParameters("GetSummaryInformation"))
            {
                ManagementObject[] vmSettingsArray = new ManagementObject[] { vmSettings };

                inParams["SettingData"] = vmSettingsArray;
                inParams["RequestedInformation"] = new UInt32[] { 
                    SummaryInformationAvailableMemoryBuffer,
                    SummaryInformationSwapFilesInUse };

                using (ManagementBaseObject outParams =
                    service.InvokeMethod("GetSummaryInformation", inParams, null))
                {
                    if ((uint)outParams["ReturnValue"] != 0)
                    {
                        throw new ManagementException("Method call GetSummaryInformation failed");
                    }

                    using (ManagementBaseObject summaryInfo =
                        ((ManagementBaseObject[])outParams["SummaryInformation"])[0])
                    {
                        Int32 availableMemoryBuffer = (Int32)(summaryInfo)["AvailableMemoryBuffer"];
                        bool swapFilesInUse = (bool)(summaryInfo)["SwapFilesInUse"];

                        Console.WriteLine("SLP in use? : {0}", swapFilesInUse);

                        Console.Write("VM Memory status: ");
                        if (availableMemoryBuffer == Int32.MaxValue)
                        {
                            Console.WriteLine("Unknown");
                        }
                        else if (availableMemoryBuffer < 0)
                        {
                            Console.WriteLine("Warning");
                        }
                        else if (availableMemoryBuffer < 80)
                        {
                            Console.WriteLine("Low");
                        }
                        else
                        {
                            Console.WriteLine("OK");
                        }
                    }
                }
            }
        }

        /// <summary>
        /// Gets the current swap file data root location.
        /// </summary>
        /// <param name="hostMachine">The host name of the computer on which
        /// the VM is running.</param>
        /// <param name="vmName">The VM name.</param>
        public
        void
        GetSlpDataRoot(
            string hostMachine,
            string vmName
            )
        {
            ManagementScope scope = new ManagementScope(
                @"\\" + hostMachine + @"\root\virtualization\v2", null);

            // Get the VM object and its settings.
            using (ManagementObject vm = WmiUtilities.GetVirtualMachine(vmName, scope))
            using (ManagementObject vmSettings = WmiUtilities.GetVirtualMachineSettings(vm))
            {
                // Get the current swap file data root.
                string currentSwapDataRoot = (string)vmSettings["SwapFileDataRoot"];

                Console.WriteLine("Current SLP Data Root is: {0}", currentSwapDataRoot);
            }
        }

        
        /// <summary>
        /// Modify the swap file data root location.
        /// </summary>
        /// <param name="hostMachine">The host name of the computer on which
        /// the VM is running.</param>
        /// <param name="vmName">The VM name.</param>
        /// <param name="newLocation">New SLP data root location.</param>
        public
        void
        ModifySlpDataRoot(
            string hostMachine,
            string vmName,
            string newLocation
            )
        {
            ManagementScope scope = new ManagementScope(
                @"\\" + hostMachine + @"\root\virtualization\v2", null);

            // Get the management service, VM object and its settings.
            using (ManagementObject service = WmiUtilities.GetVirtualMachineManagementService(scope))
            using (ManagementObject vm = WmiUtilities.GetVirtualMachine(vmName, scope))
            using (ManagementObject vmSettings = WmiUtilities.GetVirtualMachineSettings(vm))
            {
                // Set the swap file data root.
                vmSettings["SwapFileDataRoot"] = newLocation;

                // Modify the VM settings.
                using (ManagementBaseObject inParams = service.GetMethodParameters("ModifySystemSettings"))
                {
                    inParams["SystemSettings"] = vmSettings.GetText(TextFormat.CimDtd20);

                    using (ManagementBaseObject outParams =
                        service.InvokeMethod("ModifySystemSettings", inParams, null))
                    {
                        WmiUtilities.ValidateOutput(outParams, scope);
                        Console.WriteLine("Successfully updated SLP Data Root");
                    }
                }
            }
        }
    }
}
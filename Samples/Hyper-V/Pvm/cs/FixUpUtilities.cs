// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft. All rights reserved.

namespace Microsoft.Samples.HyperV.PVM
{
    using System;
    using System.Globalization;
    using System.Management;
    using System.IO;
    using Microsoft.Samples.HyperV.Common;

    static class FixUpUtilities
    {
        /// <summary>
        /// Finds the first Planned Virtual Machine matching pvmName and corrects
        /// the paths of all VHDs and AVHDs on the VM and its Snapshots to point
        /// to the folder specified by vhdFolderPath.
        /// </summary>
        /// <param name="pvmName">The name of the Planned VM to fix.</param>
        /// <param name="vhdFolderPath">The location of the VHDs.</param>
        internal static void
        FixVhdPaths(
            string pvmName,
            string vhdFolderPath
            )
        {
            ManagementScope scope = new ManagementScope(@"root\virtualization\v2");

            using (ManagementObject pvm = WmiUtilities.GetPlannedVirtualMachine(pvmName, scope))
            {
                using (ManagementObject vmSettingData = WmiUtilities.GetVirtualMachineSettings(pvm))
                {
                    FixVhdPaths(vmSettingData, vhdFolderPath);
                }

                using (ManagementObjectCollection snapshotSettings = GetSnapshotSettings(pvm))
                foreach (ManagementObject snapshotSettingData in snapshotSettings)
                using (snapshotSettingData)
                {
                    FixVhdPaths(snapshotSettingData, vhdFolderPath);
                }
            }
        }

        /// <summary>
        /// Given a particular Virtual System Setting Data, changes the VHD folder paths
        /// for all VHD Resource Allocation Setting Datas associated with it.
        /// </summary>
        /// <param name="settingData">The virtual system setting data whose VHD paths will be
        /// fixed.</param>
        /// <param name="vhdFolderPath">The location of the VHDs.</param>
        internal static void
        FixVhdPaths(
            ManagementObject settingData,
            string vhdFolderPath
            )
        {
            ManagementScope scope = new ManagementScope(@"root\virtualization\v2");

            ManagementObject[] hardDiskSettings =
                WmiUtilities.GetVhdSettingsFromVirtualMachineSettings(settingData);

            if (hardDiskSettings != null)
            {
                foreach (ManagementObject hardDiskSettingData in hardDiskSettings)
                using (hardDiskSettingData)
                {
                    string oldPath = ((string[])hardDiskSettingData["HostResource"])[0];

                    string filename = Path.GetFileName(oldPath);
                    string newPath = Path.Combine(vhdFolderPath, filename);

                    string[] newConnection = new string[] { newPath };

                    hardDiskSettingData["HostResource"] = newConnection;

                    using (ManagementObject managementService = WmiUtilities.GetVirtualMachineManagementService(scope))
                    using (ManagementBaseObject inParams =
                        managementService.GetMethodParameters("ModifyResourceSettings"))
                    {
                        string[] resources = new string[] { hardDiskSettingData.GetText(TextFormat.CimDtd20) };
                        inParams["ResourceSettings"] = resources;

                        using (ManagementBaseObject outParams =
                            managementService.InvokeMethod("ModifyResourceSettings", inParams, null))
                        {
                            WmiUtilities.ValidateOutput(outParams, scope);
                        }
                    }
                }
            }

            
        }

        /// <summary>
        /// Gets the virtual machine's snapshots' configuration settings objects.
        /// </summary>
        /// <param name="virtualMachine">The virtual machine.</param>
        /// <returns>The virtual machine's snapshots' configuration objects.</returns>
        public static ManagementObjectCollection
        GetSnapshotSettings(
            ManagementObject virtualMachine)
        {
            return virtualMachine.GetRelated("Msvm_VirtualSystemSettingData",
                "Msvm_ElementSettingData", null, null, null, null, false, null);
        }
    }
}

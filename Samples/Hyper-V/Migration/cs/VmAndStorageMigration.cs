// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft. All rights reserved.

namespace Microsoft.Samples.HyperV.Migration
{
    using System;
    using System.IO;
    using System.Management;
    using System.Diagnostics;
    using Microsoft.Samples.HyperV.Common;

    class VmAndStorageMigration : MigrationCommon
    {
        /// <summary>
        /// Migrates a VM without any modification, using all defaults.
        /// Resource pools are used to get the correct path for the VHDs.
        /// </summary>
        /// <param name="sourceHost">Migration source host name.</param>
        /// <param name="destinationHost">
        /// Migration destination host name.
        /// </param>
        /// <param name="vmName">VM name.</param>
        public
        void
        VmAndStorageMigrationSimple(
            string sourceHost,
            string destinationHost,
            string vmName
            )
        {
            ManagementScope srcScope = new ManagementScope(
                @"\\" + sourceHost + @"\root\virtualization\v2", null);

            using (ManagementObject migrationSettingData = GetMigrationSettingData(srcScope))
            using (ManagementObject vm = WmiUtilities.GetVirtualMachine(vmName, srcScope))
            {
                migrationSettingData["MigrationType"] = MigrationType.VirtualSystemAndStorage;
                migrationSettingData["TransportType"] = TransportType.TCP;

                // Get the VHD SASDs.
                string[] sasds = null;
                ManagementObject[] vhdList = WmiUtilities.GetVhdSettings(vm);
                if (vhdList != null)
                {
                    sasds = new string[vhdList.Length];

                    for (uint index = 0; index < vhdList.Length; ++index)
                    {
                        using (ManagementObject vhd = vhdList[index])
                        {
                            // Change the VHD path to an empty string, which will force
                            // the system to use resource pools to get the right path at
                            // the destination node.
                            vhd["HostResource"] = new string[] { "" };

                            // Create an array of embedded instances.
                            sasds[index] = vhd.GetText(TextFormat.CimDtd20);
                        }
                    }
                }
                else
                {
                    Console.WriteLine("No VHDs found associated with the VM. Skipping VHDs...");
                }

                // Perform migration.
                Console.WriteLine("Performing migration...");
                Migrate(srcScope,
                    vmName,
                    destinationHost,
                    migrationSettingData.GetText(TextFormat.CimDtd20),
                    null,
                    sasds);
            }
        }

        /// <summary>
        /// Migrates all VM files to a new location at the destination host.
        /// </summary>
        /// <param name="sourceHost">Migration source host name.</param>
        /// <param name="destinationHost"> Migration destination host name.</param>
        /// <param name="vmName">VM name.</param>
        /// <param name="newLocation">Destination location for VM files.</param>
        public
        void
        VmMigrationSimpleWithNewDataRoot(
            string sourceHost,
            string destinationHost,
            string vmName,
            string newLocation
            )
        {
            ManagementScope srcScope = new ManagementScope(
                @"\\" + sourceHost + @"\root\virtualization\v2", null);

            using (ManagementObject migrationSettingData = GetMigrationSettingData(srcScope))
            using (ManagementObject vm = WmiUtilities.GetVirtualMachine(vmName, srcScope))
            using (ManagementObject vssd = WmiUtilities.GetVirtualMachineSettings(vm))
            {
                migrationSettingData["MigrationType"] = MigrationType.VirtualSystemAndStorage;
                migrationSettingData["TransportType"] = TransportType.TCP;

                // Get the VHD SASDs.
                // Note: The code below gets only VHDs that are referenced in the current VM
                // configuration. VHDs that are referenced only in a snapshot
                // and not in the current configuration will not be moved.
                string[] sasds = null;
                ManagementObject[] vhdList = WmiUtilities.GetVhdSettings(vm);
                if (vhdList != null)
                {
                    sasds = new string[vhdList.Length];

                    for (uint index = 0; index < vhdList.Length; ++index)
                    {
                        using (ManagementObject vhd = vhdList[index])
                        {
                            // Prepare new VHD path.
                            string vhdPath = ((string[])vhd["HostResource"])[0];
                            string vhdName = Path.GetFileName(vhdPath);
                            string newVhdPath = Path.Combine(newLocation, vhdName);

                            // Change the VHD path.
                            // NOTE: Each VHD can be moved to a different location.
                            vhd["HostResource"] = new string[] { newVhdPath };

                            // Set the pool ID to an empty string so 
                            // the VHD will be moved to the primordial pool at the destination node.
                            vhd["PoolId"] = "";

                            // Create an array of embedded instances.
                            sasds[index] = vhd.GetText(TextFormat.CimDtd20);
                        }
                    }
                }
                else
                {
                    Console.WriteLine("No VHDs found associated with the VM. Skipping VHDs...");
                }

                //
                // Change the data roots.
                // NOTE: Each data root can be set individually and to different
                // locations as well.
                //
                vssd["ConfigurationDataRoot"] = newLocation;
                vssd["SnapshotDataRoot"] = newLocation;
                vssd["SwapFileDataRoot"] = newLocation;

                // Perform migration.
                Console.WriteLine("Performing migration...");
                Migrate(srcScope,
                    vmName,
                    destinationHost,
                    migrationSettingData.GetText(TextFormat.CimDtd20),
                    vssd.GetText(TextFormat.CimDtd20),
                    sasds);
            }
        }
    }
}
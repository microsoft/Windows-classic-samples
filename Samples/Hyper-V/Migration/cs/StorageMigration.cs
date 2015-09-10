// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft. All rights reserved.

namespace Microsoft.Samples.HyperV.Migration
{
    using System;
    using System.Collections;
    using System.IO;
    using System.Management;
    using System.Diagnostics;
    using Microsoft.Samples.HyperV.Common;

    class StorageMigration : MigrationCommon
    {
        /// <summary>
        /// Migrates VM's VHDs and data roots.
        /// </summary>
        /// <param name="vmHostName">The host where the VM is hosted.</param>
        /// <param name="vmName">The VM name.</param>
        /// <param name="newLocation">New location for all VHDs and data roots.</param>
        public
        void
        StorageMigrateSimple(
            string vmHostName,
            string vmName,
            string newLocation
            )
        {
            ManagementScope scope = new ManagementScope(
                @"\\" + vmHostName + @"\root\virtualization\v2", null);

            using (ManagementObject migrationSettingData = GetMigrationSettingData(scope))
            using (ManagementObject vm = WmiUtilities.GetVirtualMachine(vmName, scope))
            using (ManagementObject vssd = WmiUtilities.GetVirtualMachineSettings(vm))
            {
                migrationSettingData["MigrationType"] = MigrationType.Storage;

                // Get the VHD SASDs.
                // Note: The code below gets only VHDs that are referenced in the current VM
                // configuration. VHDs that are referenced only in a snapshot and not 
                // in the current configuration will not be moved.
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

                            // Create array of embedded instances.
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
                Migrate(scope,
                    vmName,
                    null,
                    migrationSettingData.GetText(TextFormat.CimDtd20),
                    vssd.GetText(TextFormat.CimDtd20),
                    sasds);
            }
        }

        
        /// <summary>
        /// Migrates a VM's first VHD to a new resource pool.
        /// </summary>
        /// <param name="vmHostName">The host where the VM is hosted.</param>
        /// <param name="vmName">The VM name.</param>
        /// <param name="newPoolId">The target resource pool ID.</param>
        /// <param name="basePath">The resource pool base directory for VHD.</param>
        public
        void
        StorageMigrationSimpleWithPool(
            string vmHostName,
            string vmName,
            string newPoolId,
            string basePath
            )
        {
            ManagementScope scope = new ManagementScope(
                @"\\" + vmHostName + @"\root\virtualization\v2", null);

            using (ManagementObject migrationSettingData = GetMigrationSettingData(scope))
            using (ManagementObject vm = WmiUtilities.GetVirtualMachine(vmName, scope))
            {
                migrationSettingData["MigrationType"] = MigrationType.Storage;

                // Get the VHD SASDs.
                ManagementObject[] vhdList = WmiUtilities.GetVhdSettings(vm);
                if (vhdList == null)
                {
                    Console.WriteLine("No VHDs found associated with the VM. Skipping VHDs...");
                    return;
                }

                string[] sasds = new string[vhdList.Length];

                for (uint index = 0; index < vhdList.Length; ++index)
                {
                    using (ManagementObject vhd = vhdList[index])
                    {
                        // Get VHD name.
                        string vhdPath = ((string[])vhd["HostResource"])[0];
                        string vhdName = Path.GetFileName(vhdPath);

                        // Change the VHD path.
                        string newVhdPath = Path.Combine(basePath, vhdName);
                        vhd["HostResource"] = new string[] { newVhdPath };

                        // Change the VHD Pool Id.
                        vhd["PoolId"] = newPoolId;

                        // Create array of embedded instances.
                        sasds[index] = vhd.GetText(TextFormat.CimDtd20);
                    }
                }

                // Perform migration.
                Console.WriteLine("Performing migration...");
                Migrate(scope,
                    vmName,
                    null,
                    migrationSettingData.GetText(TextFormat.CimDtd20),
                    null,
                    sasds);
            }
        }
    }
}
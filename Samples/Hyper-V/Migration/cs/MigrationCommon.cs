// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft. All rights reserved.

namespace Microsoft.Samples.HyperV.Migration
{
    using System;
    using System.Collections.Generic;
    using System.Management;
    using Microsoft.Samples.HyperV.Common;

    class MigrationCommon
    {
        //
        // Compatibility Vectors definitions
        //
        public enum VMCompatibility
        {
            Equal = 0,
            Superset = 1,
            Subset = 2,
            Disjoint = 3,
            GreaterThan = 4,
            GreaterThanOrEqual = 5,
            LessThan = 6,
            LessThanOrEqual = 7,
            Multiple = 8,
            Divisible = 9
        }

        public enum VectorID
        {
            ProcessorFeatures = 1,
            ProcessorXSave = 2,
            CacheLineFlushSize = 3
        }

        //
        ///////////////////////////////////////////////////////////////////////
        // Common values and methods for all migration types.
        ///////////////////////////////////////////////////////////////////////

        /// <summary>
        /// Defines the migration types.
        /// </summary>
        public enum MigrationType
        {
            VirtualSystem = 32768,
            Storage = 32769,
            Staged = 32770,
            VirtualSystemAndStorage = 32771
        };

        /// <summary>
        /// Defines migration transport types.
        /// </summary>
        public enum TransportType
        {
            TCP = 5,
            SMB = 32768
        };

        /// <summary>
        /// Gets the virtual system migration service.
        /// </summary>
        /// <param name="scope">The scope to use when connecting to WMI.</param>
        /// <returns>The virtual system migration service.</returns>
        public
        ManagementObject
        GetVirtualMachineMigrationService(
            ManagementScope scope)
        {
            using (ManagementClass migrationServiceClass = new ManagementClass("Msvm_VirtualSystemMigrationService"))
            {
                migrationServiceClass.Scope = scope;

                ManagementObject migrationService =
                    WmiUtilities.GetFirstObjectFromCollection(migrationServiceClass.GetInstances());

                return migrationService;
            }
        }

        /// <summary>
        /// Returns the instance of
        /// Msvm_VirtualSystemMigrationServiceSettingData
        /// associated with the service.
        /// </summary>
        /// <param name="service">Migration service instance.</param>
        /// <returns>
        /// Instance of Msvm_VirtualSystemMigrationServiceSettingData.
        /// </returns>
        public
        ManagementObject
        GetMigrationServiceSettings(
            ManagementObject service
            )
        {
            ManagementObject serviceSetting = null;
            using (ManagementObjectCollection settingCollection =
                service.GetRelated("Msvm_VirtualSystemMigrationServiceSettingData"))
            {
                foreach (ManagementObject mgmtObj in settingCollection)
                {
                    serviceSetting = mgmtObj;
                    break;
                }
            }

            return serviceSetting;
        }

        /// <summary>
        /// Retuns the migration setting data object to be used for
        /// migration.
        /// </summary>
        /// <param name="scope">The namespace to be used.</param>
        /// <returns>Migration setting data object.</returns>
        public
        ManagementObject
        GetMigrationSettingData(
            ManagementScope scope
            )
        {
            ManagementObject migrationSettingData = null;
            ManagementPath settingPath =
                new ManagementPath("Msvm_VirtualSystemMigrationSettingData");

            using (ManagementClass migrationSettingDataClass = new ManagementClass(scope, settingPath, null))
            {
                migrationSettingData = migrationSettingDataClass.CreateInstance();
            }

            return migrationSettingData;
        }

        /// <summary>
        /// Performs the migration.
        /// </summary>
        /// <param name="scope">The namespace to be used.</param>
        /// <param name="vmName">The virtual machine name.</param>
        /// <param name="destinationHost">Migration destination host.</param>
        /// <param name="migrationSetting">Embedded instance of
        /// Msvm_VirtualSystemMigrationSettingData.</param>
        /// <param name="vssd">Embedded instance of
        /// Msvm_VirtualSystemSettingData.</param>
        /// <param name="rasds">Array of embedded instances of
        /// Msvm_ResourceAllocationSettingData.</param>
        public
        void
        Migrate(
            ManagementScope scope,
            string vmName,
            string destinationHost,
            string migrationSetting,
            string vssd,
            string[] rasds
            )
        {
            using (ManagementObject service = GetVirtualMachineMigrationService(scope))
            using (ManagementBaseObject inParams = service.GetMethodParameters("MigrateVirtualSystemToHost"))
            using (ManagementObject vm = WmiUtilities.GetVirtualMachine(vmName, scope))
            {
                inParams["ComputerSystem"] = vm.Path.Path;
                inParams["DestinationHost"] = destinationHost;
                inParams["MigrationSettingData"] = migrationSetting;
                inParams["NewSystemSettingData"] = vssd;
                inParams["NewResourceSettingData"] = rasds;

                using (ManagementBaseObject outParams =
                    service.InvokeMethod("MigrateVirtualSystemToHost", inParams, null))
                {
                    WmiUtilities.ValidateOutput(outParams, scope);
                }
            }
        }

        /// <summary>
        /// Performs migratability check.
        /// </summary>
        /// <param name="scope">The namespace to be used.</param>
        /// <param name="vmName">The virtual machine name.</param>
        /// <param name="destinationHost">Migration destination host.</param>
        /// <param name="migrationSetting">Embedded instance of
        /// Msvm_VirtualSystemMigrationSettingData.</param>
        /// <param name="vssd">Embedded instance of
        /// Msvm_VirtualSystemSettingData.</param>
        /// <param name="rasds">Array of embedded instances of
        /// Msvm_ResourceAllocationSettingData.</param>
        /// <returns>true is the VM is migratable; otherwise, false.</returns>
        public
        bool
        CheckMigratability(
            ManagementScope scope,
            string vmName,
            string destinationHost,
            string migrationSetting,
            string vssd,
            string[] rasds
            )
        {
            using (ManagementObject service = GetVirtualMachineMigrationService(scope))
            using (ManagementBaseObject inParams =
                service.GetMethodParameters("MigrateVirtualSystemToHost"))
            using (ManagementObject vm = WmiUtilities.GetVirtualMachine(vmName, scope))
            {
                inParams["ComputerSystem"] = vm.Path.Path;
                inParams["DestinationHost"] = destinationHost;
                inParams["MigrationSettingData"] = migrationSetting;
                inParams["NewSystemSettingData"] = vssd;
                inParams["NewResourceSettingData"] = rasds;

                using (ManagementBaseObject outParams =
                    service.InvokeMethod("CheckVirtualSystemIsMigratable", inParams, null))
                {
                    return WmiUtilities.ValidateOutput(outParams, scope, false, true);
                }
            }
        }

        /// <summary>
        /// Returns an array of IP addresses on which the migration destination
        /// host is listening for incoming VM migration requests.
        /// </summary>
        /// <param name="destinationHost">
        /// Migration destination host name.
        /// </param>
        /// <returns>Listening IP addresses.</returns>
        public
        string[]
        GetMigrationDestinationListenAddresses(
            string destinationHost
            )
        {
            string[] ipAddresses = null;

            ManagementScope scope = new ManagementScope(
                @"\\" + destinationHost + @"\root\virtualization\v2", null);

            using (ManagementObject service = GetVirtualMachineMigrationService(scope))
            using (ManagementObject serviceSetting = GetMigrationServiceSettings(service))
            {
                if (!((bool)serviceSetting["EnableVirtualSystemMigration"]))
                {
                    throw new Exception("Destination host does not " +
                        "allow VM migration");
                }

                ipAddresses =
                    (string[])service["MigrationServiceListenerIPAddressList"];
                if (ipAddresses == null)
                {
                    throw new Exception("Destination host does not have " +
                        "networks set for VM migration");
                }
            }

            return ipAddresses;
        }
    }
}
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

    class MigrationSettings : MigrationCommon
    {
        /// <summary>
        /// Reads and modifies migration service settings.
        /// </summary>
        /// <param name="sourceHost">Host name of the server.</param>
        public
        void
        MigrationServiceSettings(
            string sourceHost
            )
        {
            //
            // Get the service & service setting values.
            //

            ManagementScope scope = new ManagementScope(
                @"\\" + sourceHost + @"\root\virtualization\v2", null);

            using (ManagementObject service = GetVirtualMachineMigrationService(scope))
            using (ManagementObject serviceSettings = GetMigrationServiceSettings(service))
            {
                Console.WriteLine("Currently active VM migrations: {0}",
                    service["ActiveVirtualSystemMigrationCount"]);

                Console.WriteLine("Currently active storage migrations: {0}",
                    service["ActiveStorageMigrationCount"]);

                Console.WriteLine("Maximum allowed concurrent Storage migrations: {0}",
                    serviceSettings["MaximumActiveStorageMigration"]);

                Console.WriteLine("Is VM migration allowed: {0}",
                    serviceSettings["EnableVirtualSystemMigration"]);

                Console.WriteLine("Maximum allowed concurrent VM migrations: {0}",
                    serviceSettings["MaximumActiveVirtualSystemMigration"]);

                Console.WriteLine("Maximum allowed concurrent Storage migrations: {0}",
                    serviceSettings["MaximumActiveStorageMigration"]);

                //
                // Set new setting values.
                //

                serviceSettings["EnableVirtualSystemMigration"] = true;
                serviceSettings["MaximumActiveVirtualSystemMigration"] = 4;
                serviceSettings["MaximumActiveStorageMigration"] = 5;

                // Perform service setting change.
                using (ManagementBaseObject inParams = service.GetMethodParameters("ModifyServiceSettings"))
                {
                    inParams["ServiceSettingData"] = serviceSettings.GetText(TextFormat.CimDtd20);

                    using (ManagementBaseObject outParams =
                        service.InvokeMethod("ModifyServiceSettings", inParams, null))
                    {
                        WmiUtilities.ValidateOutput(outParams, scope);
                    }
                }
            }
        }

        /// <summary>
        /// Reads and modifies migration service networks.
        /// </summary>
        /// <param name="sourceHost">Host name of the server.</param>
        public
        void
        MigrationServiceNetworks(
            string sourceHost
            )
        {
            // Get the service setting object.
            ManagementScope scope = new ManagementScope(
                @"\\" + sourceHost + @"\root\virtualization\v2", null);

            using (ManagementObject service = GetVirtualMachineMigrationService(scope))
            using (ManagementObject serviceSettings = GetMigrationServiceSettings(service))
            {
                //
                // Get currently set migration networks.
                //

                Console.WriteLine("Get currently set migration networks...");
                foreach (ManagementObject network in
                    serviceSettings.GetRelated("Msvm_VirtualSystemMigrationNetworkSettingData"))
                {
                    using (network)
                    {
                        Console.WriteLine("Network settings...");
                        Console.WriteLine("Subnet Number: {0}", network["SubnetNumber"]);
                        Console.WriteLine("Prefix length: {0}", network["PrefixLength"]);
                        Console.WriteLine();
                    }
                }

                //
                // Add a migration network.
                //
                Console.WriteLine("Add a migration network...");
                {
                    ManagementPath settingPath =
                        new ManagementPath("Msvm_VirtualSystemMigrationNetworkSettingData");

                    using (ManagementClass networkSettingDataClass = new ManagementClass(scope, settingPath, null))
                    using (ManagementObject newNetwork = networkSettingDataClass.CreateInstance())
                    {
                        newNetwork["SubnetNumber"] = "192.168.1.0";
                        newNetwork["PrefixLength"] = 24;
                        newNetwork["Metric"] = 100;
                        newNetwork["Tags"] = new string[] { "Microsoft:UserManaged" };

                        // Perform add network.
                        using (ManagementBaseObject inParams = service.GetMethodParameters("AddNetworkSettings"))
                        {
                            inParams["NetworkSettings"] =
                                new string[] { newNetwork.GetText(TextFormat.CimDtd20) };

                            using (ManagementBaseObject outParams =
                                service.InvokeMethod("AddNetworkSettings", inParams, null))
                            {
                                WmiUtilities.ValidateOutput(outParams, scope);
                            }
                        }
                    }

                    Console.WriteLine("Press ENTER to continue...");
                    Console.ReadLine();
                }

                //
                // Modify and remove a migration network.
                //

                foreach (ManagementObject network in
                    serviceSettings.GetRelated("Msvm_VirtualSystemMigrationNetworkSettingData"))
                {
                    using (network)
                    {
                        // Modify the added network.
                        if (((string)network["SubnetNumber"]).Equals("192.168.1.0") &&
                            ((Byte)network["PrefixLength"]).Equals(24))
                        {
                            //
                            // Modify the added network.
                            //
                            Console.WriteLine("Modify migration network...");
                            {
                                // Change metric value from 100 to 200.
                                network["Metric"] = 200;

                                using (ManagementBaseObject inParams =
                                    service.GetMethodParameters("ModifyNetworkSettings"))
                                {
                                    inParams["NetworkSettings"] =
                                        new string[] { network.GetText(TextFormat.CimDtd20) };

                                    using (ManagementBaseObject outParams =
                                        service.InvokeMethod("ModifyNetworkSettings", inParams, null))
                                    {
                                        WmiUtilities.ValidateOutput(outParams, scope);
                                    }
                                }

                                Console.WriteLine("Press ENTER to continue...");
                                Console.ReadLine();
                            }

                            //
                            // Remove migration network.
                            //
                            Console.WriteLine("Remove migration network...");
                            {
                                using (ManagementBaseObject inParams =
                                    service.GetMethodParameters("RemoveNetworkSettings"))
                                {
                                    inParams["NetworkSettings"] = new string[] { network.Path.Path };

                                    using (ManagementBaseObject outParams =
                                        service.InvokeMethod("RemoveNetworkSettings", inParams, null))
                                    {
                                        WmiUtilities.ValidateOutput(outParams, scope);
                                    }
                                }

                                Console.WriteLine("Press ENTER to continue...");
                                Console.ReadLine();
                            }
                        }
                    }
                }
            }
        }

        /// <summary>
        /// Enables compression on the host.
        /// </summary>
        /// <param name="sourceHost">Host name of the server.</param>
        public
        void
        EnableCompression(
            string sourceHost
            )
        {
            //
            // Get the service & service setting values.
            //

            ManagementScope scope = new ManagementScope(
                @"\\" + sourceHost + @"\root\virtualization\v2", null);

            using (ManagementObject service = GetVirtualMachineMigrationService(scope))
            using (ManagementObject serviceSettings = GetMigrationServiceSettings(service))
            {
                Console.WriteLine("Is compression enabled: {0}",
                    serviceSettings["EnableCompression"]);

                //
                // Set new setting values.
                //

                serviceSettings["EnableCompression"] = true;

                // Perform service setting change.
                using (ManagementBaseObject inParams = service.GetMethodParameters("ModifyServiceSettings"))
                {
                    inParams["ServiceSettingData"] = serviceSettings.GetText(TextFormat.CimDtd20);

                    using (ManagementBaseObject outParams =
                        service.InvokeMethod("ModifyServiceSettings", inParams, null))
                    {
                        WmiUtilities.ValidateOutput(outParams, scope);
                    }
                }
            }
        }

        /// <summary>
        /// Enables smb transport.
        /// </summary>
        /// <param name="sourceHost">Host name of the server.</param>
        public
        void
        EnableSmbTransport(
            string sourceHost
            )
        {
            //
            // Get the service & service setting values.
            //
            ManagementScope scope = new ManagementScope(
                @"\\" + sourceHost + @"\root\virtualization\v2", null);

            using (ManagementObject service = GetVirtualMachineMigrationService(scope))
            using (ManagementObject serviceSettings = GetMigrationServiceSettings(service))
            {
                Console.WriteLine("Is SMB enabled: {0}",
                    serviceSettings["EnableSmbTransport"]);

                //
                // Set new setting values.
                //

                serviceSettings["EnableSmbTransport"] = true;

                // Perform service setting change.
                using (ManagementBaseObject inParams = service.GetMethodParameters("ModifyServiceSettings"))
                {
                    inParams["ServiceSettingData"] = serviceSettings.GetText(TextFormat.CimDtd20);

                    using (ManagementBaseObject outParams =
                        service.InvokeMethod("ModifyServiceSettings", inParams, null))
                    {
                        WmiUtilities.ValidateOutput(outParams, scope);
                    }
                }
            }
        }

    }
}
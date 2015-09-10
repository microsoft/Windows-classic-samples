// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft. All rights reserved.

namespace Microsoft.Samples.HyperV.Replica
{
    using System;
    using System.Globalization;
    using System.Management;
    using Microsoft.Samples.HyperV.Common;

    static class ModifyReplicationServiceSettings
    {
        /// <summary>
        /// Enables or disables the replication service with integrated authentication.
        /// </summary>
        /// <param name="enableReplicaServer">Enable or disable replication service.</param>
        internal static void
        ModifyServiceSettings(
            bool enableReplicaServer)
        {
            ManagementScope scope = new ManagementScope(@"root\virtualization\v2");
            string serviceSettingDataEmbedded;

            //
            // Call the Msvm_ReplicationService::ModifyServiceSettings method. Note that the 
            // Msvm_ReplicationServiceSettingData instance must be passed as an embedded instance.
            //
            using (ManagementObject replicationService = 
                ReplicaUtilities.GetVirtualMachineReplicationService(scope))
            {
                //
                // Gets an instance of the Msvm_ReplicationServiceSettingData class and set the 
                // RecoveryServerEnabled field. Note that this sample enables the replication server
                // for integrated authentication.
                //
                using (ManagementObject serviceSettingData = 
                    ReplicaUtilities.GetReplicationServiceSettings(replicationService))
                {
                    serviceSettingData["RecoveryServerEnabled"] = enableReplicaServer;
                    serviceSettingData["AllowedAuthenticationType"] = 1;
                    serviceSettingDataEmbedded = serviceSettingData.GetText(TextFormat.WmiDtd20);

                    using (ManagementBaseObject inParams =
                        replicationService.GetMethodParameters("ModifyServiceSettings"))
                    {
                        inParams["SettingData"] = serviceSettingDataEmbedded;

                        using (ManagementBaseObject outParams =
                            replicationService.InvokeMethod("ModifyServiceSettings", inParams, null))
                        {
                            WmiUtilities.ValidateOutput(outParams, scope);
                        }

                        Console.WriteLine(string.Format(CultureInfo.CurrentCulture,
                            "The replication service was successfully {0}.",
                            enableReplicaServer ? "enabled" : "disabled"));
                    }
                }
            }
        }

        /// <summary>
        /// Allows a primary server or servers from a domain to establish a 
        /// replication relationship.
        /// </summary>
        /// <param name="primaryHostSystem">FQDN of the primary server.</param>
        /// <param name="trustGroup">Identifies group of trusted primary servers.</param>
        /// <param name="replicaStoragePath">Storage location for incoming replications.</param>
        internal static void
        AddAuthorizationEntry(
            string primaryHostSystem,
            string trustGroup,
            string replicaStoragePath)
        {
            ManagementScope scope = new ManagementScope(@"root\virtualization\v2");

            //
            // Call the Msvm_ReplicationService::AddAuthorizationEntry method. Note that the 
            // Msvm_ReplicationAuthorizationSettingData instance must be passed as an embedded instance.
            //
            using (ManagementObject replicationService = 
                ReplicaUtilities.GetVirtualMachineReplicationService(scope))
            {
                //
                // Create an instance of the Msvm_ReplicationAuthorizationSettingData.
                //
                string authSettingDataEmbedded;

                using (ManagementClass authSettingDataClass = new ManagementClass(
                    "Msvm_ReplicationAuthorizationSettingData"))
                {
                    authSettingDataClass.Scope = scope;

                    using (ManagementObject authSettingData = authSettingDataClass.CreateInstance())
                    {
                        authSettingData["AllowedPrimaryHostSystem"] = primaryHostSystem;
                        authSettingData["TrustGroup"] = trustGroup;
                        authSettingData["ReplicaStorageLocation"] = replicaStoragePath;

                        authSettingDataEmbedded = authSettingData.GetText(TextFormat.WmiDtd20);

                        using (ManagementBaseObject inParams =
                            replicationService.GetMethodParameters("AddAuthorizationEntry"))
                        {
                            inParams["AuthorizationEntry"] = authSettingDataEmbedded;

                            using (ManagementBaseObject outParams =
                                replicationService.InvokeMethod("AddAuthorizationEntry", inParams, null))
                            {
                                WmiUtilities.ValidateOutput(outParams, scope);
                            }

                            Console.WriteLine(string.Format(CultureInfo.CurrentCulture,
                                "Replication is authorized for \"{0}\".", primaryHostSystem));
                        }
                    }
                }
            }
        }

        /// <summary>
        /// Removes authorization entry for a given primary server.
        /// </summary>
        /// <param name="primaryHostSystem">FQDN of the primary server.</param>
        internal static void
        RemoveAuthorizationEntry(
            string primaryHostSystem)
        {
            ManagementScope scope = new ManagementScope(@"root\virtualization\v2");

            //
            // Call the Msvm_ReplicationService::RemoveAuthorizationEntry method.
            //
            using (ManagementObject replicationService =
                ReplicaUtilities.GetVirtualMachineReplicationService(scope))
            {
                using (ManagementBaseObject inParams =
                    replicationService.GetMethodParameters("RemoveAuthorizationEntry"))
                {
                    inParams["AllowedPrimaryHostSystem"] = primaryHostSystem;

                    using (ManagementBaseObject outParams =
                        replicationService.InvokeMethod("RemoveAuthorizationEntry", inParams, null))
                    {
                        WmiUtilities.ValidateOutput(outParams, scope);
                    }

                    Console.WriteLine(string.Format(CultureInfo.CurrentCulture,
                        "Replication authorization entry removed for \"{0}\".", primaryHostSystem));
                }
            }
        }

        /// <summary>
        /// Associates a virtual machine with a given server as primary server, so that 
        /// replication can be trusted.
        /// </summary>
        /// <param name="name">The name of the virtual machine to associate a primary server.</param>
        /// <param name="primaryHostSystem">FQDN of a primary server.</param>
        internal static void
        SetAuthorizationEntry(
            string name,
            string primaryHostSystem)
        {
            ManagementScope scope = new ManagementScope(@"root\virtualization\v2");

            //
            // Retrieve the Msvm_ComputerSystem.
            //
            using (ManagementObject vm = WmiUtilities.GetVirtualMachine(name, scope))
            {
                string vmPath = vm.Path.Path;

                using (ManagementObject authEntry = 
                    ReplicaUtilities.GetAuthorizationEntry(primaryHostSystem))
                {
                    string authEntryEmbedded = authEntry.GetText(TextFormat.WmiDtd20);

                    using (ManagementObject replicationService = 
                        ReplicaUtilities.GetVirtualMachineReplicationService(scope))
                    {
                        using (ManagementBaseObject inParams =
                            replicationService.GetMethodParameters("SetAuthorizationEntry"))
                        {
                            inParams["ComputerSystem"] = vmPath;
                            inParams["AuthorizationEntry"] = authEntryEmbedded;

                            using (ManagementBaseObject outParams =
                                replicationService.InvokeMethod("SetAuthorizationEntry", 
                                    inParams, 
                                    null))
                            {
                                WmiUtilities.ValidateOutput(outParams, scope);
                            }
                        }
                    }

                    Console.WriteLine(string.Format(CultureInfo.CurrentCulture,
                        "Replication for virtual machine \"{0}\" is successfully associated with \"{1}\"",
                        name, primaryHostSystem));
                }
            }
        }
    }
}

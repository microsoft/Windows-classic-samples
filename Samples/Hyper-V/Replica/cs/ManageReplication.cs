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

    static class ManageReplication
    {
        /// <summary>
        /// Enables replication for a virtual machine to a specified server using 
        /// integrated authentication.
        /// </summary>
        /// <param name="name">The name of the virtual machine to enable replication.</param>
        /// <param name="recoveryServerName">The name of the recovery server.</param>
        internal static void
        CreateReplicationRelationship(
            string name,
            string recoveryServerName)
        {
            ManagementScope scope = new ManagementScope(@"root\virtualization\v2");

            //
            // Retrieve the Msvm_ComputerSystem.
            //
            using (ManagementObject vm = WmiUtilities.GetVirtualMachine(name, scope))
            {
                string vmPath = vm.Path.Path;

                using (ManagementObject replicationSettingData =
                    ReplicaUtilities.GetReplicationSettings(vm))
                {
                    replicationSettingData["RecoveryConnectionPoint"] = recoveryServerName;
                    replicationSettingData["AuthenticationType"] = 1;
                    replicationSettingData["RecoveryServerPortNumber"] = 80;
                    replicationSettingData["CompressionEnabled"] = 1;

                    // Keep 24 recovery points.
                    replicationSettingData["RecoveryHistory"] = 24;

                    // Replicate changes after every 300 seconds.
                    replicationSettingData["ReplicationInterval"] = 300;

                    // Take VSS snapshot every one hour.
                    replicationSettingData["ApplicationConsistentSnapshotInterval"] = 1;

                    // Include all disks for replication.
                    replicationSettingData["IncludedDisks"] = WmiUtilities.GetVhdSettings(vm);

                    string settingDataEmbedded =
                        replicationSettingData.GetText(TextFormat.WmiDtd20);

                    using (ManagementObject replicationService =
                        ReplicaUtilities.GetVirtualMachineReplicationService(scope))
                    {
                        using (ManagementBaseObject inParams =
                            replicationService.GetMethodParameters("CreateReplicationRelationship"))
                        {
                            inParams["ComputerSystem"] = vmPath;
                            inParams["ReplicationSettingData"] = settingDataEmbedded;

                            using (ManagementBaseObject outParams =
                                replicationService.InvokeMethod("CreateReplicationRelationship",
                                    inParams,
                                    null))
                            {
                                WmiUtilities.ValidateOutput(outParams, scope);
                            }
                        }
                    }

                    Console.WriteLine(string.Format(CultureInfo.CurrentCulture,
                        "Replication is successfully enabled for virtual machine \"{0}\"", name));
                }
            }
        }

        /// <summary>
        /// Removes the specified replication relationship for a virtual machine.
        /// </summary>
        /// <param name="name">The name of the virtual machine to remove replication for.</param>
        /// <param name="relationshipType">The replication relationship (Primary(0) or Extended(1)) to be removed.</param>
        internal static void
        RemoveReplicationRelationshipEx(
            string name,
            UInt16 relationshipType)
        {
            if (relationshipType > 1)
            {
                throw new ArgumentException("Replication relationship should be either 0 or 1");
            }

            ManagementScope scope = new ManagementScope(@"root\virtualization\v2");

            //
            // Retrieve the Msvm_ComputerSystem.
            //
            using (ManagementObject vm = WmiUtilities.GetVirtualMachine(name, scope))
            {
                string vmPath = vm.Path.Path;

                //
                // Retrieve the specified Msvm_ReplicationRelationship object.
                //
                using (ManagementObject replicationRelationship =
                    ReplicaUtilities.GetReplicationRelationshipObject(vm, relationshipType))
                {
                    if (replicationRelationship == null)
                    {
                        throw new ManagementException(string.Format(CultureInfo.CurrentCulture,
                            "No Msvm_ReplicationRelationship object with relationship type {0} could be found",
                            relationshipType));
                    }

                    string replicationRelationshipEmbedded = 
                        replicationRelationship.GetText(TextFormat.WmiDtd20);

                    using (ManagementObject replicationService =
                        ReplicaUtilities.GetVirtualMachineReplicationService(scope))
                    {
                        using (ManagementBaseObject inParams =
                            replicationService.GetMethodParameters("RemoveReplicationRelationshipEx"))
                        {
                            inParams["ComputerSystem"] = vmPath;
                            inParams["ReplicationRelationship"] = replicationRelationshipEmbedded;

                            using (ManagementBaseObject outParams =
                                replicationService.InvokeMethod("RemoveReplicationRelationshipEx",
                                    inParams,
                                    null))
                            {
                                WmiUtilities.ValidateOutput(outParams, scope);
                            }
                        }
                    }
                }

                Console.WriteLine(string.Format(CultureInfo.CurrentCulture,
                    "{0} replication is successfully removed for virtual machine \"{1}\"",
                    relationshipType == 0 ? "Primary" : "Extended",
                    name));
            }
        }

        /// <summary>
        /// Reverses replication for a virtual machine to original primary server.
        /// Virtual machine on primary should be in correct state and should be associated with
        /// the recovery server.
        /// </summary>
        /// <param name="name">The name of the virtual machine to reverse replication.</param>
        internal static void
        ReverseReplicationRelationship(
            string name)
        {
            ManagementScope scope = new ManagementScope(@"root\virtualization\v2");

            //
            // Retrieve the Msvm_ComputerSystem.
            //
            using (ManagementObject vm = WmiUtilities.GetVirtualMachine(name, scope))
            {
                string vmPath = vm.Path.Path;

                using (ManagementObject replicationSettingData = ReplicaUtilities.GetReplicationSettings(vm))
                {
                    //
                    // Simply reverse the recovery server name with that of primary, other
                    // properties are already populated.
                    //
                    replicationSettingData["RecoveryConnectionPoint"] =
                        replicationSettingData["PrimaryConnectionPoint"];

                    string settingDataEmbedded = replicationSettingData.GetText(TextFormat.WmiDtd20);

                    using (ManagementObject replicationService =
                        ReplicaUtilities.GetVirtualMachineReplicationService(scope))
                    {
                        using (ManagementBaseObject inParams =
                            replicationService.GetMethodParameters("ReverseReplicationRelationship"))
                        {
                            inParams["ComputerSystem"] = vmPath;
                            inParams["ReplicationSettingData"] = settingDataEmbedded;

                            using (ManagementBaseObject outParams =
                                replicationService.InvokeMethod("ReverseReplicationRelationship",
                                    inParams,
                                    null))
                            {
                                WmiUtilities.ValidateOutput(outParams, scope);
                            }
                        }
                    }

                    Console.WriteLine(string.Format(CultureInfo.CurrentCulture,
                        "Replication is successfully reversed for virtual machine \"{0}\"", name));
                }
            }
        }

        /// <summary>
        /// Starts replication over network for a given virtual machine.
        /// </summary>
        /// <param name="name">The name of the virtual machine to start replication for.</param>
        internal static void
        StartReplication(
            string name)
        {
            ManagementScope scope = new ManagementScope(@"root\virtualization\v2");

            //
            // Retrieve the Msvm_ComputerSystem.
            //
            using (ManagementObject vm = WmiUtilities.GetVirtualMachine(name, scope))
            {
                //
                // Call the Msvm_ReplicationService::StartReplication method. 
                // Note the input parameters values are as below -
                // InitialReplicationType - 1 for transfer over network
                //                          2 for exporting the initial replication to the location specified
                //                            in InitialReplicationExportLocation parameter.
                //                          3 for replication with a restored copy on recovery.
                // InitialReplicationExportLocation - null or export location path when InitialReplicationType is 2.
                // StartTime - null or scheduled start time in UTC.
                //
                string vmPath = vm.Path.Path;

                using (ManagementObject replicationService =
                    ReplicaUtilities.GetVirtualMachineReplicationService(scope))
                {
                    using (ManagementBaseObject inParams =
                        replicationService.GetMethodParameters("StartReplication"))
                    {
                        inParams["ComputerSystem"] = vmPath;
                        inParams["InitialReplicationType"] = 1;
                        inParams["InitialReplicationExportLocation"] = null;
                        inParams["StartTime"] = null;

                        using (ManagementBaseObject outParams =
                            replicationService.InvokeMethod("StartReplication", inParams, null))
                        {
                            WmiUtilities.ValidateOutput(outParams, scope);
                        }
                    }
                }

                Console.WriteLine(string.Format(CultureInfo.CurrentCulture,
                    "Replication is successfully started for virtual machine \"{0}\"", name));
            }
        }

        /// <summary>
        /// Creates test replica virtual machine for a given replica virtual machine.
        /// </summary>
        /// <param name="name">The name of the virtual machine to create test replica 
        /// virtual machine for.</param>
        internal static void
        TestReplicaSystem(
            string name)
        {
            ManagementScope scope = new ManagementScope(@"root\virtualization\v2");

            //
            // Retrieve the Msvm_ComputerSystem.
            //
            using (ManagementObject vm = WmiUtilities.GetVirtualMachine(name, scope))
            {
                //
                // Call the Msvm_ReplicationService::TestReplicaSystem method. 
                // Note the input paramters values are as below -
                // SnapshotSettingData - null for latest recovery point.
                //                       OR Embedded instance of CIM_VirtualSystemSettingData 
                //                       pointing to recovery snapshot.
                //
                string vmPath = vm.Path.Path;

                using (ManagementObject replicationService =
                    ReplicaUtilities.GetVirtualMachineReplicationService(scope))
                {
                    using (ManagementBaseObject inParams =
                        replicationService.GetMethodParameters("TestReplicaSystem"))
                    {
                        inParams["ComputerSystem"] = vmPath;
                        inParams["SnapshotSettingData"] = null;

                        using (ManagementBaseObject outParams =
                            replicationService.InvokeMethod("TestReplicaSystem", inParams, null))
                        {
                            WmiUtilities.ValidateOutput(outParams, scope);
                        }
                    }
                }

                Console.WriteLine(string.Format(CultureInfo.CurrentCulture,
                    "Test replica virtual machine \"{0} - Test\" is successfully created for virtual machine \"{0}\"", name));
            }
        }

        /// <summary>
        /// Initiates failover for a given virtual machine.
        /// </summary>
        /// <param name="name">The name of the virtual machine to initiate failover for.</param>
        internal static void
        InitiateFailover(
            string name)
        {
            ManagementScope scope = new ManagementScope(@"root\virtualization\v2");

            //
            // Retrieve the Msvm_ComputerSystem.
            //
            using (ManagementObject vm = WmiUtilities.GetVirtualMachine(name, scope))
            {
                //
                // Call the Msvm_ReplicationService::InitiateFailover method. 
                // Note the input paramters values are as below -
                // SnapshotSettingData - null for latest recovery point.
                //                       OR Embedded instance of CIM_VirtualSystemSettingData 
                //                       pointing to recovery snapshot.
                //
                string vmPath = vm.Path.Path;

                using (ManagementObject replicationService =
                    ReplicaUtilities.GetVirtualMachineReplicationService(scope))
                {
                    using (ManagementBaseObject inParams =
                        replicationService.GetMethodParameters("InitiateFailover"))
                    {
                        inParams["ComputerSystem"] = vmPath;
                        inParams["SnapshotSettingData"] = null;

                        using (ManagementBaseObject outParams =
                            replicationService.InvokeMethod("InitiateFailover", inParams, null))
                        {
                            WmiUtilities.ValidateOutput(outParams, scope);
                        }
                    }
                }

                Console.WriteLine(string.Format(CultureInfo.CurrentCulture,
                    "Failover is successfully completed for virtual machine \"{0}\"", name));
            }
        }

        /// <summary>
        /// Changes replication state of the specified replication relationship of a virtual machine.
        /// </summary>
        /// <param name="name">The name of the virtual machine to change replication state.</param>
        /// <param name="requestedState">Requested replication state.</param>
        /// <param name="relationshipType">The replication relationship (Primary(0) or Extended(1)) 
        /// whose replication state is to be changed.</param>
        internal static void
        RequestReplicationStateChangeEx(
            string name,
            UInt16 requestedState,
            UInt16 relationshipType)
        {
            if (relationshipType > 1)
            {
                throw new ArgumentException("Replication relationship should be either 0 or 1");
            }

            ManagementScope scope = new ManagementScope(@"root\virtualization\v2");

            //
            // Retrieve the Msvm_ComputerSystem.
            //
            using (ManagementObject vm = WmiUtilities.GetVirtualMachine(name, scope))
            {
                //
                // Retrieve the specified Msvm_ReplicationRelationship object.
                //
                using (ManagementObject replicationRelationship =
                    ReplicaUtilities.GetReplicationRelationshipObject(vm, relationshipType))
                {
                    if (replicationRelationship == null)
                    {
                        throw new ManagementException(string.Format(CultureInfo.CurrentCulture,
                            "No Msvm_ReplicationRelationship object with relationship type {0} could be found",
                            relationshipType));
                    }

                    string replicationRelationshipEmbedded = 
                        replicationRelationship.GetText(TextFormat.WmiDtd20);
                    
                    using (ManagementBaseObject inParams =
                        vm.GetMethodParameters("RequestReplicationStateChangeEx"))
                    {
                        inParams["RequestedState"] = requestedState;
                        inParams["ReplicationRelationship"] = replicationRelationshipEmbedded;

                        using (ManagementBaseObject outParams =
                            vm.InvokeMethod("RequestReplicationStateChangeEx", inParams, null))
                        {
                            WmiUtilities.ValidateOutput(outParams, scope);
                        }

                        ReplicaUtilities.ReplicationState state = 
                            (ReplicaUtilities.ReplicationState)requestedState;
                        Console.WriteLine(string.Format(CultureInfo.CurrentCulture,
                            "{0} replication state for virtual machine \"{1}\" is changed to \"{2}\"",
                            relationshipType == 0 ? "Primary" : "Extended",
                            name, 
                            state.ToString()));
                    }
                }
            }
        }

        /// <summary>
        /// Prints the information present in the specified replication relationship object associated 
        /// with the given vm.
        /// </summary>
        /// <param name="name">The name of the virtual machine to fetch the relationship object.</param>
        /// <param name="relationshipType">The replication relationship (Primary(0) or Extended(1)) 
        /// whose information is to be fetched.</param>
        internal static void
        GetReplicationRelationshipInfo(
            string name,
            UInt16 relationshipType)
        {
            if (relationshipType > 1)
            {
                throw new ArgumentException("Replication relationship should be either 0 or 1");
            }

            ManagementScope scope = new ManagementScope(@"root\virtualization\v2");

            //
            // Retrieve the Msvm_ComputerSystem.
            //
            using (ManagementObject vm = WmiUtilities.GetVirtualMachine(name, scope))
            {
                //
                // Retrieve the specified Msvm_ReplicationRelationship object.
                //
                using (ManagementObject replicationRelationship =
                    ReplicaUtilities.GetReplicationRelationshipObject(vm, relationshipType))
                {
                    if (replicationRelationship == null)
                    {
                        throw new ManagementException(string.Format(CultureInfo.CurrentCulture,
                            "No Msvm_ReplicationRelationship object with relationship type {0} could be found",
                            relationshipType));
                    }

                    //
                    // Print the information present in the replication relationship object.
                    //
                    ReplicaUtilities.PrintReplicationRelationshipObject(replicationRelationship);
                }
            }
        }
    }
}

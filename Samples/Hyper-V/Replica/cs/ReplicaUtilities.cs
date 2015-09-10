// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft. All rights reserved.

namespace Microsoft.Samples.HyperV.Replica
{
    using System;
    using System.Management;
    using System.Globalization;
    using Microsoft.Samples.HyperV.Common;

    static class ReplicaUtilities
    {
        /// <summary>
        /// Defines the replication states.
        /// </summary>
        public enum ReplicationState
        {
            Disabled = 0,
            Ready = 1,
            WaitingForIrCompletion = 2,
            Replicating = 3,
            PrimarySyncedReplicationComplete = 4,
            RecoveryRecovered = 5,
            RecoveryCommitted = 6,
            Suspended = 7,
            Critical = 8,
            WaitingForStartResynchronize = 9,
            Resynchronizing = 10,
            ResynchronizeSuspended = 11,
            RecoveryInProgress = 12,
            FailbackInProgress = 13,
            FailbackComplete = 14
        };

        /// <summary>
        /// Defines the replication health values.
        /// </summary>
        public enum ReplicationHealth
        {
            NotApplicable = 0,
            Ok = 1,
            Warning = 2,
            Error = 3
        };

        /// <summary>
        /// Gets the virtual system replication service.
        /// </summary>
        /// <param name="scope">The scope to use when connecting to WMI.</param>
        /// <returns>The virtual machine replication service.</returns>
        public static ManagementObject
        GetVirtualMachineReplicationService(
            ManagementScope scope)
        {
            SelectQuery query = new SelectQuery("select * from Msvm_ReplicationService");

            using (ManagementObjectSearcher queryExecute = new ManagementObjectSearcher(scope, query))
            using (ManagementObjectCollection serviceCollection = queryExecute.Get())
            {
                if (serviceCollection.Count == 0)
                {
                    throw new ManagementException("Cannot find the replication service object. " +
                        "Please check that the Hyper-V Virtual Machine Management service is running.");
                }

                return WmiUtilities.GetFirstObjectFromCollection(serviceCollection);
            }
        }

        /// <summary>
        /// Gets the replication service settings object.
        /// </summary>
        /// <param name="replicationService">The replication service object.</param>
        /// <returns>The replication service settings object.</returns>
        internal static ManagementObject
        GetReplicationServiceSettings(
            ManagementObject replicationService)
        {
            using (ManagementObjectCollection settingsCollection =
                    replicationService.GetRelated("Msvm_ReplicationServiceSettingData"))
            {
                ManagementObject replicationServiceSettings = 
                    WmiUtilities.GetFirstObjectFromCollection(settingsCollection);

                return replicationServiceSettings;
            }
        }

        /// <summary>
        /// Gets object for authorization entry.
        /// </summary>
        /// <param name="primaryHostSystem">FQDN of the primary server.</param>
        internal static ManagementObject
        GetAuthorizationEntry(
            string primaryHostSystem)
        {
            ManagementScope scope = new ManagementScope(@"root\virtualization\v2");

            using (ManagementObject replicationService = 
                ReplicaUtilities.GetVirtualMachineReplicationService(scope))
            {
                ManagementObject serviceSetting = null;
                using (ManagementObjectCollection settingCollection =
                    replicationService.GetRelated("Msvm_ReplicationAuthorizationSettingData"))
                {
                    foreach (ManagementObject mgmtObj in settingCollection)
                    {
                        if (String.Equals(mgmtObj["AllowedPrimaryHostSystem"].ToString(),
                            primaryHostSystem,
                            StringComparison.OrdinalIgnoreCase))
                        {
                            serviceSetting = mgmtObj;
                            break;
                        }
                        else
                        {
                            mgmtObj.Dispose();
                        }
                    }

                    if (serviceSetting == null)
                    {
                        Console.WriteLine(string.Format(CultureInfo.CurrentCulture,
                            "Msvm_ReplicationAuthorizationSettingData not found for \"{0}\"",
                            primaryHostSystem));
                    }
                }

                return serviceSetting;
            }
        }

        /// <summary>
        /// Gets the replication settings object for a virtual machine.
        /// </summary>
        /// <param name="virtualMachine">The virtual machine object.</param>
        /// <returns>The replication settings object.</returns>
        internal static ManagementObject
        GetReplicationSettings(
            ManagementObject virtualMachine)
        {
            using (ManagementObjectCollection settingsCollection =
                    virtualMachine.GetRelated("Msvm_ReplicationSettingData"))
            {
                ManagementObject replicationSettings = 
                    WmiUtilities.GetFirstObjectFromCollection(settingsCollection);

                return replicationSettings;
            }
        }

        /// <summary>
        /// Gets the Msvm_ReplicationRelationship instance having the specified instance id
        /// associated with the given Msvm_ComputerSystem object.
        /// </summary>
        /// <param name="virtualMachine">The Msvm_ComputerSystem instance to retrieve the 
        /// relationship object from.</param>
        /// <param name="relationshipType">The instance id of the relationship object
        /// to retrieve.</param>
        /// <returns>The Msvm_ReplicationRelationship instance.</returns>
        internal static ManagementObject
        GetReplicationRelationshipObject(
            ManagementObject virtualMachine,
            UInt16 relationshipType)
        {
            if (relationshipType > 1)
            {
                throw new ArgumentException("Replication relationship should be either 0 or 1");
            }

            using (ManagementObjectCollection relationshipCollection = 
                virtualMachine.GetRelated("Msvm_ReplicationRelationship"))
            {
                if (relationshipCollection.Count == 0)
                {
                    throw new ManagementException(
                        "No Msvm_ReplicationRelationship instance could be found");
                }

                //
                // Return the relationship instance whose InstanceID ends with given relationship type.
                //
                foreach (ManagementObject relationshipObject in relationshipCollection)
                {
                    string instanceID = relationshipObject.GetPropertyValue("InstanceID").ToString();
                    if (instanceID.EndsWith(relationshipType.ToString(CultureInfo.CurrentCulture), StringComparison.CurrentCulture))
                    {
                        return relationshipObject;
                    }
                }
            }

            return null;
        }

        /// <summary>
        /// Prints the information present in the given Msvm_ReplicationRelationship instance.
        /// </summary>
        /// <param name="relationshipObject">The Msvm_ReplicationRelationship instance.</param>
        internal static void
        PrintReplicationRelationshipObject(
            ManagementObject relationshipObject)
        {
            DateTime lastReplicationTime = ManagementDateTimeConverter.ToDateTime(
                relationshipObject.GetPropertyValue("LastReplicationTime").ToString());
            DateTime lastApplyTime = ManagementDateTimeConverter.ToDateTime(
                relationshipObject.GetPropertyValue("LastApplyTime").ToString());
            ReplicationHealth replicationHealth = 
                (ReplicationHealth)((UInt16)relationshipObject.GetPropertyValue("ReplicationHealth"));
            ReplicationState replicationState = 
                (ReplicationState)((UInt16)relationshipObject.GetPropertyValue("ReplicationState"));
            bool isPrimaryRelationship = 
                relationshipObject.GetPropertyValue("InstanceID").ToString().EndsWith("0", StringComparison.CurrentCulture);

            Console.WriteLine("Name               \t: {0}", 
                relationshipObject.GetPropertyValue("ElementName").ToString());
            Console.WriteLine("RelationshipType   \t: {0}", 
                isPrimaryRelationship ? "Primary" : "Extended");
            Console.WriteLine("ReplicationHealth  \t: {0}", replicationHealth.ToString());
            Console.WriteLine("ReplicationState   \t: {0}", replicationState.ToString());
            Console.WriteLine("LastReplicationTime\t: {0}", lastReplicationTime.ToString());
            Console.WriteLine("LastApplyTime      \t: {0}", lastApplyTime.ToString());
        }
    }
}

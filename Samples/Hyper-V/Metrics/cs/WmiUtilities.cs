// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft. All rights reserved.

namespace Microsoft.Samples.HyperV.Common
{
    using System;
    using System.Globalization;
    using System.Management;
    using System.Threading;
    using System.IO;
    using System.Xml;
    using System.Collections.Generic;
    
    enum JobState
    {
        New = 2,
        Starting = 3,
        Running = 4,
        Suspended = 5,
        ShuttingDown = 6,
        Completed = 7,
        Terminated = 8,
        Killed = 9,
        Exception = 10,
        CompletedWithWarnings = 32768
    }

    public static class VirtualSystemTypeNames
    {
        public const string RealizedVM = "Microsoft:Hyper-V:System:Realized";
        public const string PlannedVM = "Microsoft:Hyper-V:System:Planned";
        public const string RealizedSnapshot = "Microsoft:Hyper-V:Snapshot:Realized";
        public const string RecoverySnapshot = "Microsoft:Hyper-V:Snapshot:Recovery";
        public const string PlannedSnapshot = "Microsoft:Hyper-V:Snapshot:Planned";
        public const string MissingSnapshot = "Microsoft:Hyper-V:Snapshot:Missing";
        public const string ReplicaStandardRecoverySnapshot = "Microsoft:Hyper-V:Snapshot:Replica:Standard";
        public const string ReplicaApplicationConsistentRecoverySnapshot = "Microsoft:Hyper-V:Snapshot:Replica:ApplicationConsistent";
        public const string ReplicaPlannedRecoverySnapshot = "Microsoft:Hyper-V:Snapshot:Replica:PlannedFailover";
        public const string ReplicaSettings = "Microsoft:Hyper-V:Replica"; 
    }
    public static class WmiUtilities
    {

        /// <summary>
        /// Validates the output parameters of a method call and prints errors, if any.
        /// </summary>
        /// <param name="outputParameters">The output parameters of a WMI method call.</param>
        /// <param name="scope">The ManagementScope to use to connect to WMI.</param>
        /// <returns><c>true</c> if successful and not firing an alert; otherwise, <c>false</c>.</returns>
        public static bool
        ValidateOutput(
            ManagementBaseObject outputParameters,
            ManagementScope scope)
        {
            return ValidateOutput(outputParameters, scope, true, false);
        }

        /// <summary>
        /// Validates the output parameters of a method call and prints errors, if any.
        /// </summary>
        /// <param name="outputParameters">The output parameters of a WMI method call.</param>
        /// <param name="scope">The ManagementScope to use to connect to WMI.</param>
        /// <param name="throwIfFailed"> If true, the method throws on failure.</param>
        /// <param name="printErrors">If true, Msvm_Error messages are displayed.</param>
        /// <returns><c>true</c> if successful and not firing an alert; otherwise, <c>false</c>.</returns>
        public static bool
        ValidateOutput(
            ManagementBaseObject outputParameters,
            ManagementScope scope,
            bool throwIfFailed,
            bool printErrors)
        {
            bool succeeded = true;
            string errorMessage = "The method call failed.";

            if ((uint)outputParameters["ReturnValue"] == 4096)
            {
                //
                // The method invoked an asynchronous operation. Get the Job object
                // and wait for it to complete. Then we can check its result.
                //

                using (ManagementObject job = new ManagementObject((string)outputParameters["Job"]))
                {
                    job.Scope = scope;

                    while (!IsJobComplete(job["JobState"]))
                    {
                        Thread.Sleep(TimeSpan.FromSeconds(1));

                        // 
                        // ManagementObjects are offline objects. Call Get() on the object to have its
                        // current property state.
                        //
                        job.Get();
                    }

                    if (!IsJobSuccessful(job["JobState"]))
                    {
                        succeeded = false;

                        //
                        // In some cases the Job object can contain helpful information about
                        // why the method call failed. If it did contain such information,
                        // use it instead of a generic message.
                        //
                        if (!string.IsNullOrEmpty((string)job["ErrorDescription"]))
                        {
                            errorMessage = (string)job["ErrorDescription"];
                        }

                        if (printErrors)
                        {
                            PrintMsvmErrors(job);
                        }

                        if (throwIfFailed)
                        {
                            throw new ManagementException(errorMessage);
                        }
                    }
                }
            }
            else if ((uint)outputParameters["ReturnValue"] != 0)
            {
                succeeded = false;

                if (throwIfFailed)
                {
                    throw new ManagementException(errorMessage);
                }
            }

            return succeeded;
        }

        /// <summary>
        /// Prints the relevant message from embedded instances of Msvm_Error.
        /// </summary>
        /// <param name="job">The job from which errors are to be printed.</param>
        public static void
        PrintMsvmErrors(
            ManagementObject job)
        {
            string[] errorList;

            using (ManagementBaseObject inParams = job.GetMethodParameters("GetErrorEx"))
            using (ManagementBaseObject outParams = job.InvokeMethod("GetErrorEx", inParams, null))
            {
                if ((uint)outParams["ReturnValue"] != 0)
                {
                    throw new ManagementException(string.Format(CultureInfo.CurrentCulture,
                                                                "GetErrorEx() call on the job failed"));
                }

                errorList = (string[])outParams["Errors"];
            }

            if (errorList == null)
            {
                Console.WriteLine("No errors found.");
                return;
            }

            Console.WriteLine("Detailed errors: \n");

            foreach (string error in errorList)
            {
                string errorSource = string.Empty;
                string errorMessage = string.Empty;
                int propId = 0;
                
                XmlReader reader = XmlReader.Create(new StringReader(error));

                while (reader.Read())
                {
                    if (reader.Name.Equals("PROPERTY", StringComparison.OrdinalIgnoreCase))
                    {
                        propId = 0;

                        if (reader.HasAttributes)
                        {
                            string propName = reader.GetAttribute(0);

                            if (propName.Equals("ErrorSource", StringComparison.OrdinalIgnoreCase))
                            {
                                propId = 1;
                            }
                            else if (propName.Equals("Message", StringComparison.OrdinalIgnoreCase))
                            {
                                propId = 2;
                            }
                        }
                    }
                    else if (reader.Name.Equals("VALUE", StringComparison.OrdinalIgnoreCase))
                    {
                        if (propId == 1)
                        {
                            errorSource = reader.ReadElementContentAsString();
                        }
                        else if (propId == 2)
                        {
                            errorMessage = reader.ReadElementContentAsString();
                        }

                        propId = 0;
                    }
                    else
                    {
                        propId = 0;
                    }
                }

                Console.WriteLine("Error Message: {0}", errorMessage);
                Console.WriteLine("Error Source:  {0}\n", errorSource);
            }
        }

        /// <summary>
        /// Gets the Msvm_ComputerSystem instance that matches the requested virtual machine name.
        /// </summary>
        /// <param name="name">The name of the virtual machine to retrieve the path for.</param>
        /// <param name="scope">The ManagementScope to use to connect to WMI.</param>
        /// <returns>The Msvm_ComputerSystem instance.</returns>
        public static ManagementObject
        GetVirtualMachine(
            string name,
            ManagementScope scope)
        {
            return GetVmObject(name, "Msvm_ComputerSystem", scope);
        }

        
        /// <summary>
        /// Gets the Msvm_PlannedComputerSystem instance matching the requested virtual machine name.
        /// </summary>
        /// <param name="name">The name of the virtual machine to retrieve the path for.</param>
        /// <param name="scope">The ManagementScope to use to connect to WMI.</param>
        /// <returns>The Msvm_PlannedComputerSystem instance.</returns>
        public static ManagementObject
        GetPlannedVirtualMachine(
            string name,
            ManagementScope scope)
        {
            return GetVmObject(name, "Msvm_PlannedComputerSystem", scope);
        }

        /// <summary>
        /// Gets the first virtual machine object of the given class with the given name.
        /// </summary>
        /// <param name="name">The name of the virtual machine to retrieve the path for.</param>
        /// <param name="className">The class of virtual machine to search for.</param>
        /// <param name="scope">The ManagementScope to use to connect to WMI.</param>
        /// <returns>The instance representing the virtual machine.</returns>
        private static ManagementObject
        GetVmObject(
            string name,
            string className,
            ManagementScope scope)
        {
            string vmQueryWql = string.Format(CultureInfo.InvariantCulture,
                "SELECT * FROM {0} WHERE ElementName=\"{1}\"", className, name);

            SelectQuery vmQuery = new SelectQuery(vmQueryWql);

            using (ManagementObjectSearcher vmSearcher = new ManagementObjectSearcher(scope, vmQuery))
            using (ManagementObjectCollection vmCollection = vmSearcher.Get())
            {
                if (vmCollection.Count == 0)
                {
                    throw new ManagementException(string.Format(CultureInfo.CurrentCulture,
                        "No {0} could be found with name \"{1}\"",
                        className,
                        name));
                }

                //
                // If multiple virtual machines exist with the requested name, return the first 
                // one.
                //
                ManagementObject vm = GetFirstObjectFromCollection(vmCollection);

                return vm;
            }
        }
        
        
        /// <summary>
        /// Gets the virtual machine's configuration settings object.
        /// </summary>
        /// <param name="virtualMachine">The virtual machine.</param>
        /// <returns>The virtual machine's configuration object.</returns>
        public static ManagementObject
        GetVirtualMachineSettings(
            ManagementObject virtualMachine)
        {
            using (ManagementObjectCollection settingsCollection = 
                    virtualMachine.GetRelated("Msvm_VirtualSystemSettingData", "Msvm_SettingsDefineState",
                    null, null, null, null, false, null))
            {
                ManagementObject virtualMachineSettings =
                    GetFirstObjectFromCollection(settingsCollection);

                return virtualMachineSettings;
            }
        }

        /// <summary>
        /// Gets the Msvm_ComputerSystem instance that matches the host computer system.
        /// </summary>
        /// <param name="scope">The ManagementScope to use to connect to WMI.</param>
        /// <returns>The Msvm_ComputerSystem instance for the host computer system.</returns>
        public static ManagementObject
        GetHostComputerSystem(
            ManagementScope scope)
        {
            //
            // The host computer system uses the same WMI class (Msvm_ComputerSystem) as the 
            // virtual machines, so we can simply reuse the GetVirtualMachine with the name
            // of the host computer system.
            //
            return GetVirtualMachine(Environment.MachineName, scope);
        }

        /// <summary>
        /// Gets the Msvm_ComputerSystem instance for the host computer system with name hostName.
        /// </summary>
        /// <param name="hostName">Host computer system name.</param>
        /// <param name="scope">The ManagementScope to use to connect to WMI.</param>
        /// <returns>The Msvm_ComputerSystem instance for the host computer system.</returns>
        public static ManagementObject
        GetHostComputerSystem(
            string hostName,
            ManagementScope scope)
        {
            //
            // The host computer system uses the same WMI class (Msvm_ComputerSystem) as the 
            // virtual machines, so we can simply reuse the GetVirtualMachine with the name
            // of the host computer system.
            //
            return GetVirtualMachine(hostName, scope);
        }

        /// <summary>
        /// Gets the CIM_ResourcePool derived instance matching the specified type, subtype and
        /// pool id.
        /// </summary>
        /// <param name="resourceType">The resource type of the resource pool.</param>
        /// <param name="resourceSubtype">The resource subtype of the resource pool.</param>
        /// <param name="poolId">The pool id of the resource pool.</param>
        /// <param name="scope">The ManagementScope to use to connect to WMI.</param>
        /// <returns>The CIM_ResourcePool derived instance.</returns>
        public static ManagementObject
        GetResourcePool(
            string resourceType,
            string resourceSubtype,
            string poolId,
            ManagementScope scope)
        {
            string poolQueryWql;

            if (resourceType == "1") // OtherResourceType
            {
                poolQueryWql = string.Format(CultureInfo.InvariantCulture,
                    "SELECT * FROM CIM_ResourcePool WHERE ResourceType=\"{0}\" AND " +
                    "OtherResourceType=\"{1}\" AND PoolId=\"{2}\"",
                    resourceType, resourceSubtype, poolId);
            }
            else
            {
                poolQueryWql = string.Format(CultureInfo.InvariantCulture,
                    "SELECT * FROM CIM_ResourcePool WHERE ResourceType=\"{0}\" AND " +
                    "ResourceSubType=\"{1}\" AND PoolId=\"{2}\"",
                    resourceType, resourceSubtype, poolId);
            }

            SelectQuery poolQuery = new SelectQuery(poolQueryWql);
            
            using (ManagementObjectSearcher poolSearcher = new ManagementObjectSearcher(scope, poolQuery))
            using (ManagementObjectCollection poolCollection = poolSearcher.Get())
            {
                //
                // There will always only be one resource pool for a given type, subtype and pool id.
                //
                if (poolCollection.Count != 1)
                {
                    throw new ManagementException(string.Format(CultureInfo.CurrentCulture,
                        "A single CIM_ResourcePool derived instance could not be found for " +
                        "ResourceType \"{0}\", ResourceSubtype \"{1}\" and PoolId \"{2}\"",
                        resourceType, resourceSubtype, poolId));
                }

                ManagementObject pool = GetFirstObjectFromCollection(poolCollection);

                return pool;
            }
        }

        /// <summary>
        /// Gets the CIM_ResourcePool derived instances matching the specified type, and subtype.
        /// </summary>
        /// <param name="resourceType">The resource type of the resource pool.</param>
        /// <param name="resourceSubtype">The resource subtype of the resource pool.</param>
        /// <param name="scope">The ManagementScope to use to connect to WMI.</param>
        /// <returns>The CIM_ResourcePool derived instance.</returns>
        public static ManagementObjectCollection
        GetResourcePools(
            string resourceType,
            string resourceSubtype,
            ManagementScope scope)
        {
            string poolQueryWql;

            if (resourceType == "1") // OtherResourceType
            {
                poolQueryWql = string.Format(CultureInfo.InvariantCulture,
                    "SELECT * FROM CIM_ResourcePool WHERE ResourceType=\"{0}\" AND " +
                    "OtherResourceType=\"{1}\"",
                    resourceType, resourceSubtype);
            }
            else
            {
                poolQueryWql = string.Format(CultureInfo.InvariantCulture,
                    "SELECT * FROM CIM_ResourcePool WHERE ResourceType=\"{0}\" AND " +
                    "ResourceSubType=\"{1}\"",
                    resourceType, resourceSubtype);
            }

            SelectQuery poolQuery = new SelectQuery(poolQueryWql);

            using (ManagementObjectSearcher poolSearcher = new ManagementObjectSearcher(scope, poolQuery))
            {
                return poolSearcher.Get();
            }
        }

        /// <summary>
        /// Gets the array of Msvm_StorageAllocationSettingData of VHDs associated with the virtual machine.
        /// </summary>
        /// <param name="virtualMachine">The virtual machine object.</param>
        /// <returns>Array of Msvm_StorageAllocationSettingData of VHDs associated with the virtual machine.</returns>
        public static
        ManagementObject[]
        GetVhdSettings(
            ManagementObject virtualMachine)
        {
            // Get the virtual machine settings (Msvm_VirtualSystemSettingData object).
            using (ManagementObject vssd = WmiUtilities.GetVirtualMachineSettings(virtualMachine))
            {
                return GetVhdSettingsFromVirtualMachineSettings(vssd);
            }
        }

        /// <summary>
        /// Gets the array of Msvm_StorageAllocationSettingData of VHDs associated with the given virtual
        /// machine settings.
        /// </summary>
        /// <param name="virtualMachineSettings">A ManagementObject representing the settings of a virtual
        /// machine or snapshot.</param>
        /// <returns>Array of Msvm_StorageAllocationSettingData of VHDs associated with the given settings.</returns>
        public static
        ManagementObject[]
        GetVhdSettingsFromVirtualMachineSettings(
            ManagementObject virtualMachineSettings)
        {
            const UInt16 SASDResourceTypeLogicalDisk = 31;

            List<ManagementObject> sasdList = new List<ManagementObject>();

            //
            // Get all the SASDs (Msvm_StorageAllocationSettingData)
            // and look for VHDs.
            //
            using (ManagementObjectCollection sasdCollection =
                virtualMachineSettings.GetRelated("Msvm_StorageAllocationSettingData",
                    "Msvm_VirtualSystemSettingDataComponent",
                    null, null, null, null, false, null))
            {
                foreach (ManagementObject sasd in sasdCollection)
                {
                    if ((UInt16)sasd["ResourceType"] == SASDResourceTypeLogicalDisk)
                    {
                        sasdList.Add(sasd);
                    }
                    else
                    {
                        sasd.Dispose();
                    }
                }
            }

            if (sasdList.Count == 0)
            {
                return null;
            }
            else
            {
                return sasdList.ToArray();
            }
        }

        /// <summary>
        /// Gets the virtual system management service.
        /// </summary>
        /// <param name="scope">The scope to use when connecting to WMI.</param>
        /// <returns>The virtual system management service.</returns>
        public static ManagementObject
        GetVirtualMachineManagementService(
            ManagementScope scope)
        {
            using (ManagementClass managementServiceClass =
                new ManagementClass("Msvm_VirtualSystemManagementService"))
            {
                managementServiceClass.Scope = scope;

                ManagementObject managementService =
                    GetFirstObjectFromCollection(managementServiceClass.GetInstances());

                return managementService;
            }
        }

        /// <summary>
        /// Gets the virtual system management service setting data.
        /// </summary>
        /// <param name="scope">The scope to use when connecting to WMI.</param>
        /// <returns>The virtual system management service settings.</returns>
        public static ManagementObject
        GetVirtualMachineManagementServiceSettings(
            ManagementScope scope)
        {
            using (ManagementClass serviceSettingsClass =
                new ManagementClass("Msvm_VirtualSystemManagementServiceSettingData"))
            {
                serviceSettingsClass.Scope = scope;

                ManagementObject serviceSettings =
                    GetFirstObjectFromCollection(serviceSettingsClass.GetInstances());

                return serviceSettings;
            }
        }

        /// <summary>
        /// Gets the virtual system snapshot service.
        /// </summary>
        /// <param name="scope">The scope to use when connecting to WMI.</param>
        /// <returns>The virtual system snapshot service.</returns>
        public static ManagementObject
        GetVirtualMachineSnapshotService(
            ManagementScope scope)
        {
            using (ManagementClass snapshotServiceClass =
                new ManagementClass("Msvm_VirtualSystemSnapshotService"))
            {
                snapshotServiceClass.Scope = scope;

                ManagementObject snapshotService =
                    GetFirstObjectFromCollection(snapshotServiceClass.GetInstances());

                return snapshotService;
            }
        }
        
        /// <summary>
        /// Gets the first object in a collection of ManagementObject instances.
        /// </summary>
        /// <param name="collection">The collection of ManagementObject instances.</param>
        /// <returns>The first object in the collection</returns>
        public static ManagementObject
        GetFirstObjectFromCollection(
            ManagementObjectCollection collection)
        {
            if (collection.Count == 0)
            {
                throw new ArgumentException("The collection contains no objects", "collection");
            }

            foreach (ManagementObject managementObject in collection)
            {
                return managementObject;
            }

            return null;
        }

        /// <summary>
        /// Takes a WMI object path and escapes it so that it can be used inside a WQL query WHERE
        /// clause. This effectively means replacing '\' and '"' characters so they are treated
        /// like any other characters.
        /// </summary>
        /// <param name="objectPath">The object management path.</param>
        /// <returns>The escaped object management path.</returns>
        public static string
        EscapeObjectPath(
            string objectPath)
        {
            string escapedObjectPath = objectPath.Replace("\\", "\\\\");
            escapedObjectPath = escapedObjectPath.Replace("\"", "\\\"");

            return escapedObjectPath;
        }

        /// <summary>
        /// Verifies whether a job is completed.
        /// </summary>
        /// <param name="jobStateObj">An object that represents the JobState of the job.</param>
        /// <returns>True if the job is completed, False otherwise.</returns>
        private static bool
        IsJobComplete(
            object jobStateObj)
        {
            JobState jobState = (JobState)((ushort)jobStateObj);

            return (jobState == JobState.Completed) || 
                (jobState == JobState.CompletedWithWarnings) ||(jobState == JobState.Terminated) ||
                (jobState == JobState.Exception) || (jobState == JobState.Killed);
        }

        /// <summary>
        /// Verifies whether a job succeeded.
        /// </summary>
        /// <param name="jobStateObj">An object representing the JobState of the job.</param>
        /// <returns><c>true</c>if the job succeeded; otherwise, <c>false</c>.</returns>
        private static bool
        IsJobSuccessful(
            object jobStateObj)
        {
            JobState jobState = (JobState)((ushort)jobStateObj);

            return (jobState == JobState.Completed) || (jobState == JobState.CompletedWithWarnings);
        }
    }
}

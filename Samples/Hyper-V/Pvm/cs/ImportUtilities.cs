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
    using System.Collections.Generic;
    using Microsoft.Samples.HyperV.Common;

    static class ImportUtilities
    {
        /// <summary>
        /// Imports a given virtual machine definition file into a planned virtual machine.
        /// Optionally imports snapshot definitions if a snapshot data root is provided.
        /// </summary>
        /// <param name="VmFilePath">The path to the virtual machine definition file.</param>
        /// <param name="snapshotFolderPath">The path to the folder that contains the 
        /// snapshots for the VM being imported.</param>
        /// <param name="newId">Boolean value that specifies whether to generate a new id 
        /// for the imported Virtual Machine.</param>
        /// <returns>ManagementObject that represents the imported PVM, or null if the 
        /// PVM can't be found.</returns>
        internal static ManagementObject
        ImportVm(
            string vmDefinitionPath,
            string snapshotFolderPath,
            bool newId)
        {
            ManagementScope scope = new ManagementScope(@"root\virtualization\v2");

            //
            // Retrieve the Virtual Machine Management Service.
            //
            using (ManagementObject managementService = WmiUtilities.GetVirtualMachineManagementService(scope))
            using (ManagementBaseObject inParams = 
                managementService.GetMethodParameters("ImportSystemDefinition"))
            {
                //
                // Call the import method using the supplied arguments.
                //

                inParams["SystemDefinitionFile"] = vmDefinitionPath;
                inParams["SnapshotFolder"] = snapshotFolderPath;
                inParams["GenerateNewSystemIdentifier"] = newId;

                using (ManagementBaseObject outParams = 
                    managementService.InvokeMethod("ImportSystemDefinition", inParams, null))
                {
                    return GetImportedPvm(outParams);
                }
            }
        }

        /// <summary>
        /// Given output parameters from a call to ImportSystemDefinition, returns the imported PVM.
        /// Returns null if the PVM wasn't found. Throws if the outparameters indicate an error.
        /// </summary>
        /// <param name="outputParameters">Output parameters from a call to 
        /// ImportSystemDefinition.</param>
        /// <returns>ManagementObject that represents the imported PVM, or 
        /// null if the PVM can't be found.</returns>
        private static ManagementObject
        GetImportedPvm(
            ManagementBaseObject outputParameters)
        {
            ManagementObject pvm = null;
            ManagementScope scope = new ManagementScope(@"root\virtualization\v2");

            if (WmiUtilities.ValidateOutput(outputParameters, scope))
            {
                if ((uint)outputParameters["ReturnValue"] == 0)
                {
                    pvm = new ManagementObject((string)outputParameters["ImportedSystem"]);
                }
                
                if ((uint)outputParameters["ReturnValue"] == 4096)
                {
                    using (ManagementObject job = 
                        new ManagementObject((string)outputParameters["Job"]))
                    using (ManagementObjectCollection pvmCollection = 
                        job.GetRelated("Msvm_PlannedComputerSystem",
                            "Msvm_AffectedJobElement", null, null, null, null, false, null))
                    {
                        pvm = WmiUtilities.GetFirstObjectFromCollection(pvmCollection);
                    }

                }
            }

            return pvm;
        }

        /// <summary>
        /// Imports any snapshots in the provided data root that match the named planned virtual
        /// machine.
        /// </summary>
        /// <param name="pvmName">The name of the Planned VM to try and find snapshots for.</param>
        /// <param name="snapshotFolderPath">The folder in which to look for snapshots.</param>
        /// <returns>The list of imported snapshot objects.</returns>
        internal static IList<ManagementObject>
        ImportSnasphotDefinitions(
            string pvmName,
            string snapshotFolderPath
            )
        {
            List<ManagementObject> importedSnapshots = new List<ManagementObject>();

            ManagementScope scope = new ManagementScope(@"root\virtualization\v2");

            using (ManagementObject pvm = WmiUtilities.GetPlannedVirtualMachine(pvmName, scope))
            using (ManagementObject managementService = WmiUtilities.GetVirtualMachineManagementService(scope))
            using (ManagementBaseObject inParams =
                managementService.GetMethodParameters("ImportSnapshotDefinitions"))
            {
                inParams["PlannedSystem"] = pvm.Path;
                inParams["SnapshotFolder"] = snapshotFolderPath;

                using (ManagementBaseObject outParams =
                    managementService.InvokeMethod("ImportSnapshotDefinitions", inParams, null))
                {
                    if (WmiUtilities.ValidateOutput(outParams, scope))
                    {
                        foreach (string snapPath in (string[])outParams["ImportedSnapshots"])
                        {
                            ManagementObject snapshot = new ManagementObject(snapPath);

                            importedSnapshots.Add(snapshot);
                        }
                    }
                }
            }

            return importedSnapshots;
        }

        /// <summary>
        /// Finds the first Planned VM matching pvmName and validates it, displaying
        /// any warnings produced.
        /// </summary>
        /// <param name="pvmName">The name of the PVM to be validated.</param>
        internal static void
        ValidatePvm(
            string pvmName
            )
        {
            ManagementScope scope = new ManagementScope(@"root\virtualization\v2");

            using (ManagementObject pvm = WmiUtilities.GetPlannedVirtualMachine(pvmName, scope))
            using (ManagementObject managementService = WmiUtilities.GetVirtualMachineManagementService(scope))
            using (ManagementBaseObject inParams = 
                managementService.GetMethodParameters("ValidatePlannedSystem"))
            {
                inParams["PlannedSystem"] = pvm.Path;

                Console.WriteLine("Validating Planned Virtual Machine \"{0}\" ({1})...",
                        pvm["ElementName"], pvm["Name"]);

                using (ManagementBaseObject outParams = 
                    managementService.InvokeMethod("ValidatePlannedSystem", inParams, null))
                {
                    if (WmiUtilities.ValidateOutput(outParams, scope))
                    {
                        using (ManagementObject job = 
                            new ManagementObject((string)outParams["Job"]))
                        {
                            WmiUtilities.PrintMsvmErrors(job);
                        }
                    }
                }
            }
        }

        /// <summary>
        /// Finds the first planned VM matching pvmName and uses the RealizePlannedSystem
        /// method to convert the VM to a realized machine.
        /// </summary>
        /// <param name="pvmName">The name of the PVM to be realized.</param>
        /// <returns>The realized virtual machine.</returns>
        internal static ManagementObject
        RealizePvm(
            string pvmName
            )
        {
            ManagementObject vm = null;
            ManagementScope scope = new ManagementScope(@"root\virtualization\v2");

            using (ManagementObject pvm = WmiUtilities.GetPlannedVirtualMachine(pvmName, scope))
            using (ManagementObject managementService = WmiUtilities.GetVirtualMachineManagementService(scope))
            using (ManagementBaseObject inParams =
                managementService.GetMethodParameters("RealizePlannedSystem"))
            {
                inParams["PlannedSystem"] = pvm.Path;

                Console.WriteLine("Realizing Planned Virtual Machine \"{0}\" ({1})...",
                        pvm["ElementName"], pvm["Name"]);

                using (ManagementBaseObject outParams =
                    managementService.InvokeMethod("RealizePlannedSystem", inParams, null))
                {
                    if (WmiUtilities.ValidateOutput(outParams, scope, true, true))
                    {
                        using (ManagementObject job =
                            new ManagementObject((string)outParams["Job"]))
                        using (ManagementObjectCollection pvmCollection =
                            job.GetRelated("Msvm_ComputerSystem",
                                "Msvm_AffectedJobElement", null, null, null, null, false, null))
                        {
                            vm = WmiUtilities.GetFirstObjectFromCollection(pvmCollection);
                        }
                    }
                }
            }

            return vm;
        }

        /// <summary>
        /// Finds the first Planned VM matching pvmName and removes it.
        /// </summary>
        /// <param name="pvmName">The name of the PVM to be removed.</param>
        internal static void
        RemovePvm(
            string pvmName
            )
        {
            ManagementScope scope = new ManagementScope(@"root\virtualization\v2");

            using (ManagementObject pvm = WmiUtilities.GetPlannedVirtualMachine(pvmName, scope))
            using (ManagementObject managementService = WmiUtilities.GetVirtualMachineManagementService(scope))
            using (ManagementBaseObject inParams =
                managementService.GetMethodParameters("DestroySystem"))
            {
                inParams["AffectedSystem"] = pvm.Path;

                Console.WriteLine("Removing Planned Virtual Machine \"{0}\" ({1})...",
                        pvm["ElementName"], pvm["Name"]);

                using (ManagementBaseObject outParams =
                    managementService.InvokeMethod("DestroySystem", inParams, null))
                {
                    WmiUtilities.ValidateOutput(outParams, scope);
                }
            }
        }
    }
}

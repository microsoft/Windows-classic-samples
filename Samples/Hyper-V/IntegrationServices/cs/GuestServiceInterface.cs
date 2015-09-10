// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft. All rights reserved.

namespace Microsoft.Samples.HyperV.IntegrationServices
{
    using System.Management;
    using System.Globalization;
    using Microsoft.Samples.HyperV.Common;

    class GuestServiceInterface
    {
        /// <summary>
        /// Get the Msvm_GuestServiceInterfaceComponent WMI object associated with a 
        /// virtual machine.
        /// </summary>
        /// <param name="scope">Management scope object for the host machine.</param>
        /// <param name="vmName">Name of the virtual machine.</param>
        /// <returns>Msvm_GuestServiceInterfaceComponent WMI object instance.</returns>
        private static ManagementObject
        GetGuestServiceInterfaceComponent(
            ManagementScope scope,
            string vmName)
        {
            ManagementObject guestServiceInterfaceComponent;

            using (ManagementObject virtualMachine = WmiUtilities.GetVirtualMachine(
                vmName,
                scope))
            {
                using (ManagementObjectCollection settingsCollection =
                    virtualMachine.GetRelated(
                        "Msvm_GuestServiceInterfaceComponent",
                        "Msvm_SystemDevice",
                        null,
                        null,
                        null,
                        null,
                        false,
                        null))
                {
                    guestServiceInterfaceComponent = WmiUtilities.GetFirstObjectFromCollection(
                        settingsCollection);
                }
            }

            return guestServiceInterfaceComponent;
        }

        /// <summary>
        /// Get the related Msvm_GuestFileService for a Msvm_GuestServiceInterfaceComponent.
        /// </summary>
        /// <param name="guestServiceInterfaceComponent">Msvm_GuestServiceInterfaceComponent WMI 
        /// object instance.</param>
        /// <returns>Msvm_GuestFileService WMI object instance.</returns>
        private static ManagementObject
        GetGuestFileService(
            ManagementObject guestServiceInterfaceComponent)
        {
            ManagementObject guestFileService;

            using (ManagementObjectCollection guestFileServices =
                guestServiceInterfaceComponent.GetRelated(
                    "Msvm_GuestFileService",
                    "Msvm_RegisteredGuestService",
                    null,
                    null,
                    null,
                    null,
                    false,
                    null))
            {
                guestFileService = WmiUtilities.GetFirstObjectFromCollection(
                    guestFileServices);
            }

            return guestFileService;
        }

        /// <summary>
        /// Helper method to create a WMI object for the CopyFileToGuest WMI method parameter.
        /// </summary>
        /// <param name="scope">Management scope object for the host machine.</param>
        /// <param name="sourceFileNamePath">Source file name path.</param>
        /// <param name="destinationFileNamePath">Destination file name path.</param>
        /// <param name="overwriteExisting">Overwrite existing file.</param>
        /// <param name="createFullPath">Create the full path.</param>
        /// <returns>Msvm_CopyFileToGuestSettingData WMI object instance.</returns>
        private static ManagementObject
        CreateCopyFileToGuestSettingData(
            ManagementScope scope,
            string sourceFileNamePath,
            string destinationFileNamePath,
            bool overwriteExisting,
            bool createFullPath)
        {
            ManagementObject wmiCopyFileToGuestSettingData;

            //
            // Get an instance of the Msvm_CopyFileToGuestSettingData object.
            //

            ManagementPath settingDataPath =
                new ManagementPath("Msvm_CopyFileToGuestSettingData");

            ManagementClass copyFileToGuestSettingDataClass =
                new ManagementClass(
                    scope,
                    settingDataPath,
                    null);

            wmiCopyFileToGuestSettingData = copyFileToGuestSettingDataClass.CreateInstance();

            //
            // Set the values and return the WMI string representation.
            //

            wmiCopyFileToGuestSettingData["SourcePath"] = sourceFileNamePath;
            wmiCopyFileToGuestSettingData["DestinationPath"] = destinationFileNamePath;
            wmiCopyFileToGuestSettingData["OverwriteExisting"] = overwriteExisting;
            wmiCopyFileToGuestSettingData["CreateFullPath"] = createFullPath;

            return wmiCopyFileToGuestSettingData;
        }

        /// <summary>
        /// Copy a file into a virtual machine.
        /// </summary>
        /// <param name="hostMachine">Hyper-V host machine name.</param>
        /// <param name="vmName">Virtual machine name.</param>
        /// <param name="sourceFileName">Source file name path.</param>
        /// <param name="destinationFileName">Destination file name path.</param>
        /// <param name="overwriteExisting">Overwrite existing file.</param>
        /// <param name="createFullPath">Create the full path.</param>
        internal void
        CopyFileToGuest(
            string hostMachine,
            string vmName,
            string sourceFileName,
            string destinationFileName,
            bool overwriteExisting,
            bool createFullPath)
        {
            //
            // Get the ManagementScope for the Hyper-V host.
            //

            string managementPath = string.Format(
                CultureInfo.CurrentCulture,
                @"\\{0}\root\virtualization\v2",
                hostMachine);

            ManagementScope scope = new ManagementScope(
                managementPath,
                null);

            ManagementObject guestFileService;

            //
            // Get the Msvm_GuestServiceInterfaceComponent instance. 
            //

            using (ManagementObject guestServiceInterfaceComponent =
                    GetGuestServiceInterfaceComponent(scope, vmName))
            {

                //
                // Get the Msvm_GuestFileService instance. 
                //

                guestFileService = GetGuestFileService(
                    guestServiceInterfaceComponent);
            }

            //
            // Create parameters for the CopyFilesToGuest method.
            //

            string[] copyFileToGuestSettingDataText = new string[1];

            using (ManagementObject wmiCopyFileToGuestSettingData = 
                CreateCopyFileToGuestSettingData(
                    scope,
                    sourceFileName,
                    destinationFileName,
                    overwriteExisting,
                    createFullPath))
            {
                copyFileToGuestSettingDataText[0] = wmiCopyFileToGuestSettingData.GetText(
                    TextFormat.CimDtd20);
            }

            using (guestFileService)
            {
                //
                // Invoke the CopyFilesToGuest method.
                //

                using (ManagementBaseObject inParams =
                    guestFileService.GetMethodParameters("CopyFilesToGuest"))
                {
                    inParams["CopyFileToGuestSettings"] = copyFileToGuestSettingDataText;

                    using (ManagementBaseObject outParams = guestFileService.InvokeMethod(
                        "CopyFilesToGuest",
                        inParams,
                        null))
                    {
                        //
                        // Validate the output.
                        //

                        WmiUtilities.ValidateOutput(
                            outParams,
                            scope,
                            true,
                            true);
                    }
                }
            }
        }
    }
}

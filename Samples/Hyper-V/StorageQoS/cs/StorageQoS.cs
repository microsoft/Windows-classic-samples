// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft. All rights reserved.

namespace Microsoft.Samples.HyperV.StorageQoS
{
    using System;
    using System.IO;
    using System.Globalization;
    using System.Management;
    using Microsoft.Samples.HyperV.Common;
    using System.Collections.Generic;

    class SQoS
    {
        // Constants required for enumerating SASDs
        private const string VHDResourceTypeDisplayName = "VHD";
        private const string VHDResourceType = "31";
        private const string VHDResourceSubType = "Microsoft:Hyper-V:Virtual Hard Disk";
        
        // constants required to check operation status of a disk
        private const UInt16 OperationalStatusOK = 2;
        private const UInt16 OperationalStatusInsufficientThroughput = 32788;

        //constant required to enumerate QoS events
        private const int OutOfPolicyMsgId = 32930;
        private const int InPolicyMsgId = 32931;

        public enum QoSEvent
        {
            InPolicy = 0,
            OutOfPolicy
        };


        private void
        ApplySASD(
            string hostMachine,
            string vmName,
            string[] sasd)
        {
            ManagementScope scope = new ManagementScope(
                  @"\\" + hostMachine + @"\root\virtualization\v2", null);

            // Get the vm information information.
            using (ManagementObject service = WmiUtilities.GetVirtualMachineManagementService(scope))
            using (ManagementObject vm = WmiUtilities.GetVirtualMachine(vmName, scope))
            using (ManagementBaseObject inParams = service.GetMethodParameters("ModifyResourceSettings"))
            {
                inParams["Resourcesettings"] = sasd;
                Console.WriteLine("    Applying new QoS settings...");
                ManagementBaseObject outParams = service.InvokeMethod("ModifyResourceSettings", inParams, null);
                if (WmiUtilities.ValidateOutput(outParams, scope, false, true))
                {
                    Console.WriteLine("    Succeeded");
                }
            }
        }


        /// <summary>
        /// Gets the VHDss SQoS Attributes
        /// </summary>
        /// <param name="hostMachine">The host name of the computer on which
        /// the VM is running.</param>
        /// <param name="vmName">The VM name.</param>
        /// <param name="vhdName">The vhd name.</param>
        public void
        GetSQoSAttributes(
            string hostMachine,
            string vmName,
            string vhdName)
        {
            ManagementScope scope = new ManagementScope(
                @"\\" + hostMachine + @"\root\virtualization\v2", null);
            
            // Get the vm information information.
            using (ManagementObject service = WmiUtilities.GetVirtualMachineManagementService(scope))
            using (ManagementObject vm = WmiUtilities.GetVirtualMachine(vmName, scope))
            using (ManagementObject vmSettings = WmiUtilities.GetVirtualMachineSettings(vm))
            {
                // Get the VHD SASDs for the VM. This will not include the snapshot VHDs.
                ManagementObject[] hardDiskSettings = WmiUtilities.GetVhdSettingsFromVirtualMachineSettings(vmSettings);

                if (hardDiskSettings != null)
                {
                    foreach (ManagementObject hardDiskSettingData in hardDiskSettings)
                    using (hardDiskSettingData)
                    {
                        string vmVhdPath = ((string[])hardDiskSettingData["HostResource"])[0];
                        string vmVhdName = Path.GetFileName(vmVhdPath);

                        // If the passed in vhdName is empty print all VHDs QoS attributes.
                        if (vhdName == null || String.Compare(vmVhdName, vhdName, StringComparison.OrdinalIgnoreCase) == 0)
                        {
                            Console.WriteLine("VHD: {0}", vmVhdName);
                            //Print out the SQoS setting on this SASD and exit
                            UInt64 min = 0;
                            UInt64 max = 0;
                            try
                            {
                                max = (UInt64)hardDiskSettingData["IOPSLimit"];
                                min = (UInt64)hardDiskSettingData["IOPSReservation"];
                                Console.WriteLine("    Maximum: {0}", max);
                                Console.WriteLine("    Minimum: {0}", min);
                            }
                            catch (System.Management.ManagementException)
                            {
                                // This property is not available on Windows 8 and below
                                Console.WriteLine("Storage QoS Not supported on this host ({0})", hostMachine);
                            }
                        }
                    }
                }
                else
                {
                    Console.WriteLine("No VHDs found associated with the VM.");
                }
            }
        }

        /// <summary>
        /// Sets the Minimum and Maximum normalized IOPS associated with a VMs VHD
        /// </summary>
        /// <param name="hostMachine">The host name of the computer on which
        /// the VM is running.</param>
        /// <param name="vmName">The VM name.</param>
        /// <param name="vhdName">The vhd path.</param>
        /// <param name="Option">"-Max" or "-Min"</param>
        /// <param name="IOPS" >Normalized IOPS</param>
        public void
        SetSQoSAttributes(
            string hostMachine,
            string vmName,
            string vhdName,
            string option,
            UInt64 iops)
        {
            ManagementScope scope = new ManagementScope(
                @"\\" + hostMachine + @"\root\virtualization\v2", null);

            // Get the vm information information.
            using (ManagementObject service = WmiUtilities.GetVirtualMachineManagementService(scope))
            using (ManagementObject vm = WmiUtilities.GetVirtualMachine(vmName, scope))
            using (ManagementObject vmSettings = WmiUtilities.GetVirtualMachineSettings(vm))
            {

                // Get the VHD SASDs for the VM. This will not include the snapshot VHDs.
                ManagementObject[] hardDiskSettings = WmiUtilities.GetVhdSettingsFromVirtualMachineSettings(vmSettings);

                if (hardDiskSettings != null)
                {
                    foreach (ManagementObject hardDiskSettingData in hardDiskSettings)
                    using (hardDiskSettingData)
                    {
                        string[] sasd = new string[1];
                        string vmVhdPath = ((string[])hardDiskSettingData["HostResource"])[0];
                        string vmVhdName = Path.GetFileName(vmVhdPath);
                        if (String.Compare(vmVhdName, vhdName, StringComparison.OrdinalIgnoreCase) == 0)
                        {
                            Console.WriteLine("VHD: {0}", vmVhdName);
                            //Set the SASD with the limit/Reservation and Apply
                            try
                            {

                                if (option.Equals("-Max", StringComparison.OrdinalIgnoreCase))
                                {
                                    hardDiskSettingData["IOPSLimit"] = iops;
                                }
                                else
                                {
                                    hardDiskSettingData["IOPSReservation"] = iops;
                                }
                                sasd[0] = hardDiskSettingData.GetText(TextFormat.CimDtd20);
                                ApplySASD(hostMachine, vmName, sasd);
                            }
                            catch (MissingMemberException)
                            {
                                // This property is not available on Windows 8 and below
                                Console.WriteLine("Storage QoS Not supported on this host({0})", hostMachine);
                            }
                        }
                    }
                }
                else
                {
                    Console.WriteLine("No VHDs found associated with the VM. Skipping VHDs...");
                }
            }
        }


        private String 
        GetEventIDString(
            int messageID)
        {
            
            String result = "";
            switch (messageID)
            {
                case OutOfPolicyMsgId:
                    result = QoSEvent.OutOfPolicy.ToString();
                    break;

                case InPolicyMsgId:
                    result = QoSEvent.InPolicy.ToString();
                    break;

                default:
                    break;
            }    
            return result;
        }

        
        private void
        PrintOutOfPolicyVHDsInPool(
            ManagementObject resourcePool)
        {
            //Get Logical disks in the pool
            ManagementObjectCollection logicalDisks = resourcePool.GetRelated(
                "Msvm_LogicalDisk",
                "Msvm_ElementAllocatedFromPool",
                null, null, null, null, false, null);

            foreach (ManagementObject logicalDisk in logicalDisks)
            using (logicalDisk)
            {
                //Check the operational status on the logical disk
                UInt16[] opStatus = (UInt16[])logicalDisk["OperationalStatus"];
                if (opStatus[0] != OperationalStatusOK)
                {                            
                    foreach (UInt16 opState in opStatus)
                    {
                        //Check for the specific QOS related opcode
                        if (opState == OperationalStatusInsufficientThroughput)
                        {
                            // Get the SASD associated with the logical disk to get the vhd name.
                            using (ManagementObjectCollection sasds = logicalDisk.GetRelated(
                                "Msvm_StorageAllocationSettingData",
                                "Msvm_SettingsDefineState",
                                null, null, null, null, false, null))
                            // There is only one sasd associated with a logical disk.
                            foreach (ManagementObject sasd in sasds)
                            using (sasd)
                            {
                                string vhdPath = ((string[])sasd["HostResource"])[0];
                                string vhdName = Path.GetFileName(vhdPath);
                                Console.WriteLine("    VHD Not in Policy: {0}", vhdName);
                            }
                        }
                    }
                }
            }
        }

       
        private void
        QoSEventHandler(
            object sender,
            EventArrivedEventArgs eventArgs)
        {
            // Get the event ID from the event.
            String eventIDStr = GetEventIDString(Int32.Parse(
                eventArgs.NewEvent.Properties["MessageID"].Value.ToString(),
                CultureInfo.CurrentCulture));
            try
            {
                // Get the resource pool specified in the event.
                ManagementObject resourcePool = new ManagementObject();
                resourcePool.Path.Path = (String)eventArgs.NewEvent.Properties["AlertingManagedElement"].Value.ToString();
                resourcePool.Get();

                if (resourcePool.Properties["PoolID"].Value.ToString() == "")
                {
                    Console.WriteLine("Processing event {0} for Primordial pool", eventIDStr);
                }
                else
                {
                    Console.WriteLine("Processing event {0} for Resource pool '{1}'", eventIDStr, resourcePool.Properties["PoolID"].Value.ToString());
                }

                if (eventIDStr.Equals(QoSEvent.OutOfPolicy.ToString()))
                {
                    PrintOutOfPolicyVHDsInPool(resourcePool);
                }
            }
            catch (System.Management.ManagementException e)
            {
                Console.WriteLine("exception: {0}", e.Message);
            }
        }
        

        public void
        MonitorSQoSEvents(
            string hostMachine)
        {
            try
            {
                string wmiQuery;
                ManagementEventWatcher watcher;

                ManagementScope scope = new ManagementScope(
                    @"\\" + hostMachine + @"\root\virtualization\v2", null);
                
                ManagementObject service = WmiUtilities.GetVirtualMachineManagementService(scope);
                
                //build the query to monitor the QoS event
                wmiQuery = "Select * From Msvm_StorageAlert";

                watcher = new ManagementEventWatcher(scope, new EventQuery(wmiQuery));
                watcher.EventArrived += new EventArrivedEventHandler(QoSEventHandler);
                watcher.Start();
                Console.WriteLine("Please press ENTER to exit monitoring...\n");
                Console.ReadLine();
                watcher.Stop();
            }
            catch (Exception e)
            {
                Console.WriteLine("Exception {0} Trace {1}", e.Message, e.StackTrace);
            }
        }
    }
}
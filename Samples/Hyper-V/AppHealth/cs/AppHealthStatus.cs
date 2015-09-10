// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft. All rights reserved.

namespace Microsoft.Samples.HyperV.AppHealth
{
    using System;
    using System.Management;
    using Microsoft.Samples.HyperV.Common;

    class AppHealthStatus
    {
        private UInt16 OperationalStatusOk = 2;
        private UInt16 OperationalStatusApplicationCriticalState = 32782;

        /// <summary>
        /// Gets the applications status in the VM.
        /// </summary>
        /// <param name="hostMachine">The host name of the computer on which
        /// the VM is running.</param>
        /// <param name="vmName">The VM name.</param>
        public
        void
        GetAppHealthStatus(
            string hostMachine,
            string vmName
            )
        {
            ManagementScope scope = new ManagementScope(
                @"\\" + hostMachine + @"\root\virtualization\v2", null);

            ManagementObject heartBeatComponent = null;

            // Get the VM object and its heart beat component.
            using (ManagementObject vm = WmiUtilities.GetVirtualMachine(vmName, scope))
            using (ManagementObjectCollection heartBeatCollection =
                vm.GetRelated("Msvm_HeartbeatComponent", "Msvm_SystemDevice",
                    null, null, null, null, false, null))
            {
                foreach (ManagementObject element in heartBeatCollection)
                {
                    heartBeatComponent = element;
                    break;
                }
            }

            if (heartBeatComponent == null)
            {
                Console.WriteLine("The VM is not running.");
                return;
            }

            using (heartBeatComponent)
            {
                UInt16[] operationalStatus = (UInt16[])heartBeatComponent["OperationalStatus"];
                UInt16 vmStatus = operationalStatus[0];

                if (vmStatus != OperationalStatusOk)
                {
                    Console.WriteLine("The VM heartbeat status is not OK");
                    return;
                }

                if (operationalStatus.Length != 2)
                {
                    Console.WriteLine("The required Integration Components are not running " +
                        "or not installed.");
                    return;
                }

                UInt16 appStatus = operationalStatus[1];
                if (appStatus == OperationalStatusOk)
                {
                    Console.WriteLine("The VM applications health status: OK");
                }
                else if (appStatus == OperationalStatusApplicationCriticalState)
                {
                    Console.WriteLine("The VM applications health status: Critical");
                }
                else
                {
                    throw new ManagementException("Unknown application health status");
                }
            }
        }
    }
}
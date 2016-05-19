// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

using System;
using System.Collections.Generic;
using System.Text;
using System.Runtime.InteropServices;
using Microsoft.storage.EhStorage;

namespace Microsoft.Samples.EnhancedStorage.EhStorManaged
{
    class Program
    {
        void EnumEhStorDevices()
        {
            try
            {
                PortableDeviceManagerImp manager = new PortableDeviceManagerImp();
                Console.WriteLine("{0} Windows Portable Device(s) found in the system", manager.deviceCount);

                for (Int32 n = 0; n < manager.deviceCount; n++)
                {
                    Console.WriteLine("[" + n.ToString() + "]" +
                                    "\n\tManufacturer:     " + manager.deviceManufacturer(n) +
                                    "\n\tDescription:      " + manager.deviceDescription(n) +
                                    "\n\tFriendly Name:    " + manager.deviceFriendlyName(n) +
                                    "\n\tPNP ID:           " + manager.deviceID(n));
                }
            }
            catch (COMException exception)
            {
                Console.WriteLine("COM exception: code {1}, {0}", exception.Message, exception.ErrorCode);
            }
            catch (Exception exception)
            {
                Console.WriteLine(exception.Message);
            }
        }

        static void Main(string[] args)
        {
            Program ThisProgram = new Program();
            ThisProgram.EnumEhStorDevices();
        }
    }
}

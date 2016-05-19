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
    public class PortableDeviceManagerImp
    {
        const uint ERROR_INSUFFICIENT_BUFFER = 0x8007007A;

        // get number of devices available in system
        public Int32 deviceCount
        {
            get
            {
                Int32 count = 0;
                _manager.GetDevices(null, ref count);
                return count;
            }
        }

        // get string ID of device specified
        public string deviceID(Int32 index)
        {
            Int32 PnPDeviceIDsNum = deviceCount;

            if (index < 0 || index >= PnPDeviceIDsNum)
            {
                return null;
            }

            string[] ids = new String[PnPDeviceIDsNum];
            _manager.GetDevices(ids, ref PnPDeviceIDsNum);

            return ids[index];
        }

        // get device manufacturer string
        public string deviceManufacturer(Int32 index)
        {
            if (index < 0 || index >= deviceCount)
            {
                return null;
            }

            UInt32 chars = 100;
            while (true)
            {
                StringBuilder result = new StringBuilder((int)chars + 1);
                try
                {
                    _manager.GetDeviceManufacturer(deviceID(index), result, ref chars);
                    return result.ToString();
                }
                catch (COMException exception)
                {
                    if ((uint)exception.ErrorCode == ERROR_INSUFFICIENT_BUFFER)
                    {
                        chars += 100;
                    }
                    else
                    {
                        throw exception;
                    }
                }
            }
        }

        // get device description string
        public string deviceDescription(Int32 index)
        {
            if (index < 0 || index >= deviceCount)
            {
                return null;
            }

            UInt32 chars = 100;
            while (true)
            {
                StringBuilder result = new StringBuilder((int)chars + 1);
                try
                {
                    _manager.GetDeviceDescription(deviceID(index), result, ref chars);
                    return result.ToString();
                }
                catch (COMException exception)
                {
                    if ((uint)exception.ErrorCode == ERROR_INSUFFICIENT_BUFFER)
                    {
                        chars += 100;
                    }
                    else
                    {
                        throw exception;
                    }
                }
            }
        }

        // get device friendly name
        public string deviceFriendlyName(Int32 index)
        {
            if (index < 0 || index >= deviceCount)
            {
                return null;
            }

            UInt32 chars = 100;
            while (true)
            {
                StringBuilder result = new StringBuilder((int)chars + 1);
                try
                {
                    _manager.GetDeviceFriendlyName(deviceID(index), result, ref chars);
                    return result.ToString();
                }
                catch (COMException exception)
                {
                    if ((uint)exception.ErrorCode == ERROR_INSUFFICIENT_BUFFER)
                    {
                        chars += 100;
                    }
                    else
                    {
                        return "Error retreiving Friendly Name 0x" + exception.ErrorCode.ToString("X");
                    }
                }
            }
        }

        private IPortableDeviceManager _manager = new PortableDeviceManager();
    }
}

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
using System.Runtime.InteropServices.ComTypes;
using System.Collections;

namespace Microsoft.storage.EhStorage
{
    public class EhStorAPIHelper
    {
        // convert tripple pointer (IEnhancedStorageACT ***) to managed array IEnhancedStorageACT[]
        // or IEnhancedStorageSilo ** to IEnhancedStorageSilo[] whatever
        // C# Generics allow us to use any type of data here
        // reffer to http://msdn.microsoft.com/en-us/library/aa720369.aspx
        public static void IntPtrToArray<resInterface>(UInt32 elementCount, IntPtr tripplePointer, out resInterface[] result)
        {
            //get the output array of pointers
            IntPtr[] inPointers = new IntPtr[elementCount];
            Marshal.Copy(tripplePointer, inPointers, 0, (int)elementCount);
            // declare and create output array
            result = new resInterface[elementCount];

            for (int i = 0; i < elementCount; i++)
            {
                result[i] = (resInterface)Marshal.GetObjectForIUnknown(inPointers[i]);
            }

            // free up the incoming unmanaged array
            Marshal.FreeCoTaskMem(tripplePointer);
        }
    }

    public class StorageACTEnumerator
    {
        public UInt32 ACTCount
        {
            get
            {
                IEnhancedStorageACT[] IEnhancedStorageACTs;
                UInt32 actCount;

                GetACTs(out IEnhancedStorageACTs, out actCount);

                return actCount;
            }
        }

        public StorageACT GetACT(UInt32 index)
        {
            IEnhancedStorageACT[] IEnhancedStorageACTs;
            UInt32 actCount;

            GetACTs(out IEnhancedStorageACTs, out actCount);

            if (index >= actCount)
            {
                throw new SystemException("index has incorrect value");
            }

            return new StorageACT(IEnhancedStorageACTs[index]);
        }

        public IEnhancedStorageACT GetMatchingACT(string volume)
        {
            return _enumACT.GetMatchingACT(volume);
        }

        private void GetACTs(out IEnhancedStorageACT[] IEnhancedStorageACTs, out UInt32 actCount)
        {
            IntPtr listACTsPtrs;

            IEnhancedStorageACTs = null;
            _enumACT.GetACTs(out listACTsPtrs, out actCount);
            if (actCount > 0)
            {
                EhStorAPIHelper.IntPtrToArray(actCount, listACTsPtrs, out IEnhancedStorageACTs);
            }
        }

        IEnumEnhancedStorageACT _enumACT = new EnumEnhancedStorageACT();
    }

    public class StorageACT
    {
        public StorageACT()
        {
            _act = new EnhancedStorageACT();
        }

        public StorageACT(IEnhancedStorageACT a)
        {
            _act = a;
        }

        public void Authorize(UInt32 hwndParent, UInt32 flags)
        {
            _act.Authorize(hwndParent, flags);
        }

        public void Unauthorize()
        {
            _act.Unauthorize();
        }

        public ACT_AUTHORIZATION_STATE AuthorizationState
        {
            get
            {
                return _act.GetAuthorizationState();
            }
        }

        public string MatchingVolume
        {
            get
            {
                return _act.GetMatchingVolume();
            }
        }

        public string UniqueIdentity
        {
            get
            {
                return _act.GetUniqueIdentity();
            }
        }

        public StorageACTSilo[] silos
        {
            get
            {
                IEnhancedStorageSilo[] enhancedStorageSilos;
                UInt32 siloCount;
                StorageACTSilo[] result;

                GetSilos(out enhancedStorageSilos, out siloCount);
                result = new StorageACTSilo[siloCount];

                for (UInt32 i = 0; i < siloCount; i ++ )
                {
                    result[i] = new StorageACTSilo(enhancedStorageSilos[i]);
                }

                return result;
            }
        }

        public UInt32 siloCount
        {
            get
            {
                IEnhancedStorageSilo[] enhancedStorageSilos;
                UInt32 siloCount;

                GetSilos(out enhancedStorageSilos, out siloCount);

                return siloCount;
            }
        }


        private void GetSilos(out IEnhancedStorageSilo[] enhancedStorageSilos, out UInt32 siloCount)
        {
            IntPtr listSilos;

            enhancedStorageSilos = null;
            _act.GetSilos(out listSilos, out siloCount);
            if (siloCount > 0)
            {
                EhStorAPIHelper.IntPtrToArray(siloCount, listSilos, out enhancedStorageSilos);
            }
        }

        IEnhancedStorageACT _act;
    }

    public class StorageACTSilo
    {
        public StorageACTSilo()
        {
            _silo = new EnhancedStorageSilo();
        }

        public StorageACTSilo(IEnhancedStorageSilo s)
        {
            _silo = s;
        }

        public SILO_INFO info
        {
            get
            {
                return _silo.GetInfo();
            }
        }
        public string devicePath
        {
            get
            {
                return _silo.GetDevicePath();
            }
        }

        public void SendCommand(Byte command, Byte[] commandBuffer, UInt32 expectedResponseBufferSize, out Byte[] responseBuffer)
        {
            // 1. align command buffer to 512 bytes boundary
            Byte[] commandBufferAlign = new Byte[(commandBuffer.Length % 512 == 0) ? commandBuffer.Length : (commandBuffer.Length / 512 + 1) * 512];
            // 2. create response buffer 
            responseBuffer = new Byte[expectedResponseBufferSize];
            UInt32 responseBufferSize = (UInt32)responseBuffer.Length;
            UInt32 commandBufferLength = (UInt32)commandBufferAlign.Length;
            commandBuffer.CopyTo(commandBufferAlign, 0);
            // 3. send command to silo
            _silo.SendCommand(command, commandBufferAlign, commandBufferLength, responseBuffer, ref responseBufferSize);
        }

        public StorageACTSiloAction[] siloActions
        {
            get
            {
                IEnhancedStorageSiloAction[] eh_StorageSiloActions;
                UInt32 actionCount;
                StorageACTSiloAction[] result;

                GetActions(out eh_StorageSiloActions, out actionCount);
                result = new StorageACTSiloAction[actionCount];

                for (UInt32 i = 0; i < actionCount; i++)
                {
                    result[i] = new StorageACTSiloAction(eh_StorageSiloActions[i]);
                }

                return result;
            }
        }

        public UInt32 actionCount
        {
            get
            {
                IEnhancedStorageSiloAction[] eh_StorageSiloActions;
                UInt32 actionCount;

                GetActions(out eh_StorageSiloActions, out actionCount);

                return actionCount;
            }
        }

        private void GetActions(out IEnhancedStorageSiloAction[] enhancedStorageSiloActions, out UInt32 actionsCount)
        {
            IntPtr listActions;

            enhancedStorageSiloActions = null;
            _silo.GetActions(out listActions, out actionsCount);
            if (actionsCount > 0)
            {
                EhStorAPIHelper.IntPtrToArray(actionsCount, listActions, out enhancedStorageSiloActions);
            }
        }

        IEnhancedStorageSilo _silo;
    }

    public class StorageACTSiloAction
    {
        public StorageACTSiloAction()
        {
            _action = new EnhancedStorageSiloAction();
        }

        public StorageACTSiloAction(IEnhancedStorageSiloAction s)
        {
            _action = s;
        }

        public string name
        {
            get
            {
                return _action.GetName();
            }
        }

        public string description
        {
            get
            {
                return _action.GetDescription();
            }
        }

        public void Invoke()
        {
            _action.Invoke();
        }

        IEnhancedStorageSiloAction _action;
    }

    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    public struct SECURITY_PROTOCOL_HEADER
    {
        public UInt32 payloadSize;
        public UInt16 Reserved1;
        public Byte Status;
        public UInt16 Reserved2;
    };
}

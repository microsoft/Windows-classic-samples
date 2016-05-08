// --------------------------------------------------------------------
//
//  Copyright (c) Microsoft Corporation.  All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// -------------------------------------------------------------------- 
namespace Microsoft.Samples.MessageQueuing.LargeMessageQueue
{
    using System;
    using System.Messaging;
    using System.Runtime.InteropServices;

    // Provide Interop service
    internal static class NativeMethods
    {
        // Applications can purge messages from a queue opened with receive access 
        // http://msdn.microsoft.com/en-us/library/ms703188(VS.85).aspx
        // Queue Access constants.
        private const int QUEUE_ACCESS_RECEIVE = 1;
        private const int QUEUE_ACCESS_MOVE = 4;
        private const int QUEUE_ACCESS_PEEK = 32;
        private const int QUEUE_ACCESS_ADMIN = 128;

        // Queue Shared Mode constants.
        private const int QUEUE_SHARED_MODE_DENY_NONE = 0;
        private const int QUEUE_SHARED_MODE_DENY_RECEIVE = 1;

        // Queue Action constants
        private const uint QUEUE_ACTION_PEEK_CURRENT = 0x80000000;
        private const uint QUEUE_ACTION_PEEK_NEXT = 0x80000001;

        // http://msdn.microsoft.com/en-us/library/ms699817(VS.85).aspx
        [DllImport("C:\\Windows\\System32\\mqrt.dll", CharSet = CharSet.Unicode)]
        internal static extern int MQOpenQueue(string lpwcsFormatName, int dwAccess, int dwShareMode, ref IntPtr phQueue);

        // http://msdn.microsoft.com/en-us/library/ms701502(VS.85).aspx
        [DllImport("C:\\Windows\\System32\\mqrt.dll")]
        internal static extern int MQMoveMessage(IntPtr sourceQueue, IntPtr targetQueue, long lookupID, IntPtr pTransaction);

        // http://msdn.microsoft.com/en-us/library/ms707071(VS.85).aspx
        [DllImport("C:\\Windows\\System32\\mqrt.dll", CharSet = CharSet.Unicode)]
        internal static extern int MQMarkMessageRejected(IntPtr hQueue, long ullLookupId);

        // Open sub-queue. Creates sub-queue if not already exists
        internal static void OpenSubQueue(MessageQueue mainQ, string correlationId)
        {
            int status = 0;
            IntPtr targetHandle = IntPtr.Zero;
            string subQPath = "DIRECT=OS:" + mainQ.MachineName + @"\" + mainQ.QueueName + ";" + Utilities.GetSubQueueName(correlationId);

            status = MQOpenQueue(subQPath, QUEUE_ACCESS_MOVE, QUEUE_SHARED_MODE_DENY_NONE, ref targetHandle);
            if (status < 0)
            {
                throw new InvalidOperationException();
            }
        }

        // Open and move message from main queue to sub-queue
        internal static void MoveLargeMessage(MessageQueue mainQ, string subQueuePath, long lookupIDToMove)
        {
            int status = 0;
            IntPtr targetHandle = IntPtr.Zero;
            IntPtr internalTransaction = IntPtr.Zero;

            status = MQOpenQueue(subQueuePath, QUEUE_ACCESS_MOVE, QUEUE_SHARED_MODE_DENY_NONE, ref targetHandle);
            if (status < 0)
            {
                throw new InvalidOperationException();
            }

            status = MQMoveMessage(mainQ.ReadHandle, targetHandle, lookupIDToMove, internalTransaction);
            if (status < 0)
            {
                throw new InvalidOperationException();
            }
        }

        // send message to sender's DLQ if the property was set while sending message
        internal static void MarkMessageRejected(string queueFormatName, long lookupID)
        {
            IntPtr targetHandle = IntPtr.Zero;
            int status = MQOpenQueue(queueFormatName, QUEUE_ACCESS_RECEIVE, QUEUE_SHARED_MODE_DENY_NONE, ref targetHandle);
            if (status < 0)
            {
                throw new InvalidOperationException();
            }

            status = MQMarkMessageRejected(targetHandle, lookupID);
            if (status < 0)
            {
                throw new InvalidOperationException();
            }
        }
    }
}
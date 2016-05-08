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

    // Paramters used for internal purpose
    internal class Parameters
    {
        // Maximum possible fragment size in bytes 
        // (allow some bytes [from exact 4MB = 4194304 bytes] for the final BinaryMessageFormatter of the sample API)
        // Typically, BinaryMessageFormatter overhead is 220 bytes
        internal const int MAX_MESSAGE_SIZE = 4000000;

        // Status queue location
        internal const string STATUS_QUEUE = @"PRIVATE$\statusq";

        // Default identifier for large message (empty guid\LARGE_MESSAGE_ID_DEFAULT)
        internal const int LARGE_MESSAGE_ID_DEFAULT = 9999;

        // AppSpecific value for header fragment
        internal const int HEADER_FRAGMENT_ID = -9998;

        // AppSpecific value for trailer fragment
        internal const int TRAILER_FRAGMENT_ID = -9999;

        private Parameters()
        {
        }

        // Determine action to be performed on the messages in queue
        internal enum ReceiveAction
        {
            // Represents complete sequence (all fragments available)
            Complete = 0,

            // Represents pending state (fragments still to be found)
            Pending = 1,

            // Restart sequence processing (initial state)
            Restart = 2,

            // Represents sequence should be deleted (fragment holes in the sequence)
            Delete = 3,

            // Redundant state
            None = 4
        }
    }
}

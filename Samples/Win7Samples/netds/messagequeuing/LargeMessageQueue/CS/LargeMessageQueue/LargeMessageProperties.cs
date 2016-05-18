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

    // Provide interface to large message properties in header/trailer fragment
    internal class LargeMessageProperties
    {
        // Fragment count property that is put in the control fragments
        private int fragmentCount;

        // Application-provided AppSpecific property that is put in the control fragments
        private int appSpecific;

        // Application-provided CorrelationId property that is put in the control fragments
        private string correlationId;

        internal int FragmentCount
        {
            get { return this.fragmentCount; }
            set { this.fragmentCount = value; }
        }

        internal int AppSpecific
        {
            get { return this.appSpecific; }
            set { this.appSpecific = value; }
        }

        internal string CorrelationId
        {
            get { return this.correlationId; }
            set { this.correlationId = value; }
        }
    }
}

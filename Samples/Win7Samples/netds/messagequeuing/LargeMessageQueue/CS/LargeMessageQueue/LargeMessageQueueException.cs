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
    using System.Runtime.Serialization;
    using System.Security.Permissions;

    // Is thrown if large message fragments are missing
    [Serializable]
    public class LargeMessageQueueException : Exception
    {
        // CorrelationId that identifies the largeSequenceId of the fragments
        private string correlationId;

        public LargeMessageQueueException()
        {
        }

        public LargeMessageQueueException(string correlationId)
        {
            this.correlationId = correlationId;
        }

        public LargeMessageQueueException(string correlationId, string message)
            : base(message)
        {
            this.correlationId = correlationId;
        }

        public LargeMessageQueueException(string message, Exception innerException)
            : base(message, innerException)
        {
        }

        protected LargeMessageQueueException(SerializationInfo info, StreamingContext context)
            : base(info, context)
        {
            if (info != null)
            {
                this.correlationId = info.GetString("correlationId");
            }
        }

        public string CorrelationId
        {
            get { return this.correlationId; }
        }

        [SecurityPermissionAttribute(SecurityAction.Demand, SerializationFormatter = true)]
        public override void GetObjectData(SerializationInfo info, StreamingContext context)
        {
            base.GetObjectData(info, context);

            if (info != null)
            {
                info.AddValue("correlationId", this.correlationId);
            }
        }
    }
}

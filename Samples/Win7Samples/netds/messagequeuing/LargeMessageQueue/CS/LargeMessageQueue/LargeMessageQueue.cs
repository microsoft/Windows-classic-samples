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
    using System.IO; 
    using System.Globalization;
    using System.Text;

    // Provide large message send/receive functionality
    public class LargeMessageQueue
    {
        // Queue reference set by the application
        private MessageQueue queue;

        // Fragment size set by the application
        private int fragmentSize;

        // Stream object used to contain the stream/formatted-object, that is fragmented into the body/bodystream
        private Stream stream;

        // Message object that points to the application-provided message or newly created to contain the 
        // object passed by the application
        private Message message;

        // Body and BodyStream handling changes based on this flag
        private bool useBody;

        // Constructor
        public LargeMessageQueue(MessageQueue queue, int fragmentSize)
        {
            if (queue == null)
            {
                throw new ArgumentNullException("queue", "Queue object cannot be null");
            }

            this.queue = queue;

            if ((fragmentSize < 0) || (fragmentSize > Parameters.MAX_MESSAGE_SIZE))
            {
                throw new ArgumentOutOfRangeException("fragmentSize", "Fragment Size cannot be less than zero and greater than " + Parameters.MAX_MESSAGE_SIZE);
            }
            else if (fragmentSize == 0)
            {
                this.fragmentSize = Parameters.MAX_MESSAGE_SIZE;
            }
            else
            {
                this.fragmentSize = fragmentSize;
            }
        }

        public void Close()
        {
            this.useBody = false;
            this.queue = null;
            this.message = null;
            this.fragmentSize = 0;

            if (this.stream != null)
            {
                this.stream.Close();
                this.stream = null;
            }
        }

        public int FragmentSize
        {
            get { return this.fragmentSize; }
        }

        // Peek overload methods similar to normal System.Messaging.MessageQueue.Peek methods
        public Message Peek()
        {
            return this.PeekGeneric(null, MessageQueue.InfiniteTimeout);
        }

        public Message Peek(TimeSpan timeout)
        {
            return this.PeekGeneric(null, timeout);
        }

        // PeekByCorrelationId overload methods similar to normal System.Messaging.MessageQueue.PeekByCorrelationId 
        // methods. The methods are used reading the response messages in the request-response sequence
        public Message PeekByCorrelationId(string correlationId)
        {
            return this.PeekGeneric(correlationId, MessageQueue.InfiniteTimeout);
        }

        public Message PeekByCorrelationId(string correlationId, TimeSpan timeout)
        {
            return this.PeekGeneric(correlationId, timeout);
        }

        // Receive overload methods similar to normal System.Messaging.MessageQueue.Receive methods
        public Message Receive()
        {
            return this.ReceiveGeneric(null, MessageQueue.InfiniteTimeout, null, MessageQueueTransactionType.None);
        }

        public Message Receive(TimeSpan timeout)
        {
            return this.ReceiveGeneric(null, timeout, null, MessageQueueTransactionType.None);
        }

        public Message Receive(MessageQueueTransaction transaction)
        {
            return this.ReceiveGeneric(null, MessageQueue.InfiniteTimeout, transaction, MessageQueueTransactionType.None);
        }

        public Message Receive(MessageQueueTransactionType transactionType)
        {
            return this.ReceiveGeneric(null, MessageQueue.InfiniteTimeout, null, transactionType);
        }

        public Message Receive(TimeSpan timeout, MessageQueueTransaction transaction)
        {
            return this.ReceiveGeneric(null, timeout, transaction, MessageQueueTransactionType.None);
        }

        public Message Receive(TimeSpan timeout, MessageQueueTransactionType transactionType)
        {
            return this.ReceiveGeneric(null, timeout, null, transactionType);
        }

        // ReceiveByCorrelationId overload methods similar to normal System.Messaging.MessageQueue.ReceiveByCorrelationId 
        // methods. The methods are used reading the response messages in the request-response sequence
        public Message ReceiveByCorrelationId(string correlationId)
        {
            return this.ReceiveGeneric(correlationId, MessageQueue.InfiniteTimeout, null, MessageQueueTransactionType.None);
        }

        public Message ReceiveByCorrelationId(string correlationId, TimeSpan timeout)
        {
            return this.ReceiveGeneric(correlationId, timeout, null, MessageQueueTransactionType.None);
        }

        public Message ReceiveByCorrelationId(string correlationId, MessageQueueTransaction transaction)
        {
            return this.ReceiveGeneric(correlationId, MessageQueue.InfiniteTimeout, transaction, MessageQueueTransactionType.None);
        }

        public Message ReceiveByCorrelationId(string correlationId, MessageQueueTransactionType transactionType)
        {
            return this.ReceiveGeneric(correlationId, MessageQueue.InfiniteTimeout, null, transactionType);
        }

        public Message ReceiveByCorrelationId(string correlationId, TimeSpan timeout, MessageQueueTransaction transaction)
        {
            return this.ReceiveGeneric(correlationId, timeout, transaction, MessageQueueTransactionType.None);
        }

        public Message ReceiveByCorrelationId(string correlationId, TimeSpan timeout, MessageQueueTransactionType transactionType)
        {
            return this.ReceiveGeneric(correlationId, timeout, null, transactionType);
        }

        // Send overload methods similar to normal System.Messaging.MessageQueue.Send methods
        public void Send(object value)
        {
            this.SendGeneric(value, null, null, MessageQueueTransactionType.None, false);
        }

        public void Send(object value, string label)
        {
            this.SendGeneric(value, label, null, MessageQueueTransactionType.None, false);
        }

        public void Send(object value, MessageQueueTransaction transaction)
        {
            this.SendGeneric(value, null, transaction, MessageQueueTransactionType.None, false);
        }

        public void Send(object value, MessageQueueTransactionType transactionType)
        {
            this.SendGeneric(value, null, null, transactionType, false);
        }

        public void Send(object value, string label, MessageQueueTransaction transaction)
        {
            this.SendGeneric(value, label, transaction, MessageQueueTransactionType.None, false);
        }

        public void Send(object value, string label, MessageQueueTransactionType transactionType)
        {
            this.SendGeneric(value, label, null, transactionType, false);
        }

        // Send overload methods for STREAMING similar to normal System.Messaging.MessageQueue.Send methods
        public void Send(Stream value)
        {
            this.SendGeneric(value, null, null, MessageQueueTransactionType.None, true);
        }

        public void Send(Stream value, string label)
        {
            this.SendGeneric(value, label, null, MessageQueueTransactionType.None, true);
        }

        public void Send(Stream value, MessageQueueTransaction transaction)
        {
            this.SendGeneric(value, null, transaction, MessageQueueTransactionType.None, true);
        }

        public void Send(Stream value, MessageQueueTransactionType transactionType)
        {
            this.SendGeneric(value, null, null, transactionType, true);
        }

        public void Send(Stream value, string label, MessageQueueTransaction transaction)
        {
            this.SendGeneric(value, label, transaction, MessageQueueTransactionType.None, true);
        }

        public void Send(Stream value, string label, MessageQueueTransactionType transactionType)
        {
            this.SendGeneric(value, label, null, transactionType, true);
        }

        // Reassembles the entire stream from fragment's body/bodystream
        private void AssembleFragments()
        {
            bool nullBody = false;
            try
            {
                if ((this.message.Body == null) && (this.message.BodyStream.Length == 0))
                {
                    nullBody = true;
                }

                this.useBody = true;
            }
            catch
            {
                this.useBody = true;
                nullBody = true;
                try
                {
                    if (this.message.BodyStream.Length != 0)
                    {
                        this.useBody = false;
                        nullBody = false;
                    }
                }
                catch
                {
                }
            }

            if (nullBody)
            {
                this.stream = null;
            }
            else
            {
                if (this.useBody)
                {
                    if (this.message.Body != null)
                    {
                        byte[] b = null;

                        if (this.message.Body is Stream)
                        {
                            b = Utilities.GetBytesFromStream((Stream)this.message.Body);
                        }
                        else
                        {
                            b = (byte[])this.message.Body;
                        }

                        this.stream.Write(b, 0, b.Length);
                        this.useBody = true;
                    }
                    else
                    {
                        Stream s = this.message.BodyStream;
                        byte[] b = new byte[s.Length];
                        s.Read(b, 0, b.Length);
                        s.Position = 0;
                        this.stream.Write(b, 0, b.Length);
                        this.useBody = false;
                    }
                }
            }
        }

        // Returns true if the large message sequence in the sub-queue is completed
        private bool CheckCompleteMessage(MessageQueue subQueue)
        {
            Message firstMessageInSubQueue = subQueue.PeekByLookupId(MessageLookupAction.First, 0);
            firstMessageInSubQueue.Formatter = new BinaryMessageFormatter();
            if (firstMessageInSubQueue.AppSpecific == Parameters.HEADER_FRAGMENT_ID)
            {
                Message lastMessageInSubQueue = subQueue.PeekByLookupId(MessageLookupAction.Last, 0);
                if (lastMessageInSubQueue.AppSpecific == Parameters.TRAILER_FRAGMENT_ID)
                {
                    return true;
                }
            }

            return false;
        }

        // throws exception if hole is found in the large message sequence
        // If not found, message is moved to the subqueue if moveMessage flag is true
        private int CheckReceiveMessage(MessageQueue subQueue, string correlationId, string largeSequenceId)
        {
            LargeMessageProperties largeMessageProperties = null;
            IMessageFormatter formatter = new BinaryMessageFormatter();

            // If AppSpecific is zero, it is first fragment
            if (this.message.AppSpecific != Parameters.HEADER_FRAGMENT_ID)
            {
                try
                {
                    Message lastMessageInSubQueue = subQueue.PeekByLookupId(MessageLookupAction.Last, 0);
                    lastMessageInSubQueue.Formatter = formatter;

                    if (this.message.AppSpecific == Parameters.TRAILER_FRAGMENT_ID)
                    {
                        // If message in main queue is a trailer fragment, check if the last message in subqueue
                        // corresponds to the last expected message with application data
                        largeMessageProperties = this.GetLargeMessageHeader(this.message);
                        if (lastMessageInSubQueue.AppSpecific != largeMessageProperties.FragmentCount)
                        {
                            // Fragment(s) in the middle (before the trailer fragment) missing, send delete status message
                            this.SendToStatusQueue(correlationId, (int)Parameters.ReceiveAction.Delete, largeSequenceId);
                            return (int)Parameters.ReceiveAction.Restart;
                        }
                    }
                    else if (((this.message.AppSpecific != (lastMessageInSubQueue.AppSpecific + 1)) && 
                        (lastMessageInSubQueue.AppSpecific != Parameters.HEADER_FRAGMENT_ID)) ||
                        ((this.message.AppSpecific != 1) && (lastMessageInSubQueue.AppSpecific == Parameters.HEADER_FRAGMENT_ID)))
                    {
                        // Fragment(s) in the middle missing, send delete status message
                        this.SendToStatusQueue(correlationId, (int)Parameters.ReceiveAction.Delete, largeSequenceId);
                        return (int)Parameters.ReceiveAction.Restart;
                    }
                }
                catch (InvalidOperationException)
                {
                    // No message in subqueue, send delete status message
                    this.SendToStatusQueue(correlationId, (int)Parameters.ReceiveAction.Delete, largeSequenceId);
                    return (int)Parameters.ReceiveAction.Restart;
                }
            }
            else
            {
                largeMessageProperties = this.GetLargeMessageHeader(this.message);
            }

            // Move message to sub-queue
            try
            {
                NativeMethods.MoveLargeMessage(this.queue, subQueue.FormatName, this.message.LookupId);
            }
            catch (InvalidOperationException)
            {
                // look for another large message sequence
                return ((int)Parameters.ReceiveAction.Restart);
            }

            if (this.message.AppSpecific == Parameters.TRAILER_FRAGMENT_ID)
            {
                // All fragments available in subqueue
                return ((int)Parameters.ReceiveAction.Complete);
            }

            // More fragments are expected
            return ((int)Parameters.ReceiveAction.Pending);
        }

        // Return the large message properties embedded in the control (header/trailer) fragments
        private LargeMessageProperties GetLargeMessageHeader(Message headerMessage)
        {
            byte[] b = null;
            if (headerMessage.Body != null)
            {
                if (headerMessage.Body is Stream)
                {
                    b = Utilities.GetBytesFromStream((Stream)headerMessage.Body);
                }
                else
                {
                    b = (byte[])headerMessage.Body;
                }

                this.useBody = true;
            }
            else if (headerMessage.BodyStream != null)
            {
                Stream s = headerMessage.BodyStream;
                b = new byte[s.Length];
                s.Read(b, 0, b.Length);
                s.Position = 0;

                this.useBody = false;
            }

            LargeMessageProperties largeMessageProperties = new LargeMessageProperties();
            byte[] fragmentCountBytes = new byte[4];
            byte[] appSpecificBytes = new byte[4];
            byte[] correlationIdLengthBytes = new byte[4];

            System.Buffer.BlockCopy(b, 0, fragmentCountBytes, 0, 4);
            largeMessageProperties.FragmentCount = BitConverter.ToInt32(fragmentCountBytes, 0);

            System.Buffer.BlockCopy(b, 4, appSpecificBytes, 0, 4);
            largeMessageProperties.AppSpecific = BitConverter.ToInt32(appSpecificBytes, 0);

            System.Buffer.BlockCopy(b, 8, correlationIdLengthBytes, 0, 4);
            int correlationIdLength = BitConverter.ToInt32(correlationIdLengthBytes, 0);

            if (correlationIdLength != 0)
            {
                byte[] correlationIdBytes = new byte[correlationIdLength];
                System.Buffer.BlockCopy(b, 12, correlationIdBytes, 0, correlationIdLength);
                largeMessageProperties.CorrelationId = Encoding.Unicode.GetString(correlationIdBytes);
            }
            else
            {
                largeMessageProperties.CorrelationId = Utilities.GetEmptyCorrelationIdStr();
            }

            return largeMessageProperties;
        }

        // Provide reference to the sub-queue
        private MessageQueue GetSubQueue(string largeSequenceId)
        {
            // NOTE Sub Queue name limit is 31 characters. So Guid bytes are used instead of its string representation
            string subQueueName = Utilities.GetSubQueueName(largeSequenceId);

            string subQueuePath = "FORMATNAME:DIRECT=OS:" + this.queue.MachineName + @"\" + this.queue.QueueName + ";" + subQueueName;
            MessageQueue subQueue = new MessageQueue(subQueuePath);
            subQueue.MessageReadPropertyFilter.CorrelationId = true;
            subQueue.MessageReadPropertyFilter.AppSpecific = true;
            return subQueue;
        }

        // Modified PeekByCorrelationId that peeks forward for correlation id
        private Message PeekByCorrelationId(string correlationId, TimeSpan timeout, Cursor cursor, PeekAction action)
        {
            string compare = string.Empty;
            if (!string.IsNullOrEmpty(correlationId))
            {
                compare = correlationId;
            }

            Message m = this.queue.Peek(timeout, cursor, action);
            while (m != null)
            {
                if (String.Compare(m.CorrelationId, 0, Utilities.GetEmptyCorrelationIdStr(), 0, m.CorrelationId.Length, StringComparison.OrdinalIgnoreCase) == 0)
                {
                    if (string.IsNullOrEmpty(compare))
                    {
                        return m;
                    }
                }
                else
                {
                    if (String.Compare(m.CorrelationId, 0, compare, 0, m.CorrelationId.Length, StringComparison.OrdinalIgnoreCase) == 0)
                    {
                        return m;
                    }
                }

                // Continue search, peek next message
                m = this.queue.Peek(timeout, cursor, PeekAction.Next);
            }

            return null;
        }

        // Missing fragments are marked as rejected in a separate transaction and are sent to sender's DLQ on commit.
        private void PerformHouseKeeping(string correlationId)
        {
            string subQueueName = Utilities.GetSubQueueName(correlationId);
            string subQueuePath = "FormatName:DIRECT=OS:" + this.queue.MachineName + @"\" + this.queue.QueueName + ";" + subQueueName;
            MessageQueue subQueue = new MessageQueue(subQueuePath);

            Message messageToReject = null;

            // MarkMessageRejected works only with transaction
            MessageQueueTransaction rejectTransaction = new MessageQueueTransaction();
            rejectTransaction.Begin();

            // ReceiveById to clear the first header fragment whose Id is put in all other fragment's correlation id
            // This is in a separate try-catch so that it proceeds with marking other fragments in case first fragment 
            // is the one that is lost
            try
            {
                messageToReject = subQueue.ReceiveById(correlationId, rejectTransaction);
                NativeMethods.MarkMessageRejected(subQueue.FormatName, messageToReject.LookupId);
            }
            catch (MessageQueueException)
            {
                // Don't do anything 
            }
            catch (InvalidOperationException)
            {
                // Don't do anything 
            }
            
            // Marks other fragments in subqueue as rejected
            try
            {
                while (true)
                {
                    messageToReject = subQueue.ReceiveByCorrelationId(correlationId, rejectTransaction);
                    NativeMethods.MarkMessageRejected(subQueue.FormatName, messageToReject.LookupId);
                }
            }
            catch (MessageQueueException)
            {
                // Don't do anything and just come out of the loop
            }
            catch (InvalidOperationException)
            {
                // Don't do anything and just come out of the loop
            }

            // Safe reject in case message becomes available in main queue
            try
            {
                messageToReject = this.queue.ReceiveById(correlationId, rejectTransaction);
                NativeMethods.MarkMessageRejected(subQueue.FormatName, messageToReject.LookupId);
            }
            catch (MessageQueueException)
            {
                // Don't do anything 
            }
            catch (InvalidOperationException)
            {
                // Don't do anything 
            }

            // Mark other fragments in main queue as rejected
            try
            {
                while (true)
                {
                    messageToReject = this.queue.ReceiveByCorrelationId(correlationId, rejectTransaction);
                    NativeMethods.MarkMessageRejected(this.queue.FormatName, messageToReject.LookupId);
                }
            }
            catch (MessageQueueException)
            {
                // Don't do anything and just come out of the loop
            }
            catch (InvalidOperationException)
            {
                // Don't do anything and just come out of the loop
            }
            finally
            {
                subQueue.Dispose();
                subQueue.Close();

                rejectTransaction.Commit();
                rejectTransaction.Dispose();
            }
        }

        // Restore message properties before passing the message to application
        private void PrepareLargeMessage(LargeMessageProperties largeMessageProperties)
        {
            if (this.stream != null)
            {
                this.stream.Position = 0;
                if (this.useBody)
                {
                    byte[] fullBytes = new byte[this.stream.Length];
                    this.stream.Read(fullBytes, 0, fullBytes.Length);
                    if (this.message.Body is Stream)
                    {
                        this.message.BodyStream.Dispose();
                        ((Stream)this.message.Body).Dispose();

                        this.message.BodyStream = new MemoryStream(fullBytes, 0, fullBytes.Length);
                        this.message.Body = this.message.BodyStream;
                    }
                    else
                    {
                        // The body has to be set to null so that the formatter read method (deserialize) is invoked
                        this.message.Body = null;

                        try
                        {
                            this.message.BodyStream.Dispose();
                        }
                        catch
                        {
                            // Don't do anything
                        }

                        this.message.BodyStream = new MemoryStream(fullBytes, 0, fullBytes.Length);
                    }
                }
                else
                {
                    try
                    {
                        this.message.BodyStream.Dispose();
                    }
                    catch
                    {
                        // Don't do anything
                    }

                    this.message.BodyStream = this.stream;
                }
            }
            else
            {
                this.message.Body = null;
                this.message.BodyStream = new MemoryStream();
            }

            // Setting the fields to that passed from the application
            this.message.AppSpecific = largeMessageProperties.AppSpecific;
            this.message.CorrelationId = largeMessageProperties.CorrelationId;
            
            // Setting the message Label to the application message label from the sender side
            string messageLabel = this.message.Label;
            try
            {
                this.message.Label = messageLabel.Substring(0, messageLabel.LastIndexOf(".part.", StringComparison.OrdinalIgnoreCase));
            }
            catch (ArgumentOutOfRangeException)
            {
                this.message.Label = messageLabel;
            }
        }

        // The message body/bodystream is embed into a LargeMessageBody object
        private void PrepareLargeMessageFragment()
        {
            if (this.stream != null)
            {
                byte[] b = new byte[this.fragmentSize];
                this.stream.Read(b, 0, this.fragmentSize);
                if (this.useBody)
                {
                    if (this.message.Body is Stream)
                    {
                        this.message.Body = Utilities.GetStreamFromBytes(b);
                    }
                    else
                    {
                        this.message.Body = b;
                    }
                }
                else
                {
                    this.message.BodyStream = Utilities.GetStreamFromBytes(b);
                }
            }
            else
            {
                this.message.Body = null;
                this.message.BodyStream = null;
            }
        }

        // Create large message body for control (header/trailer) fragments
        private void PrepareLargeMessageFragment(int fragmentCount, int applicationAppSpecific, string applicationCorrelationId)
        {
            byte[] fragmentCountBytes = BitConverter.GetBytes(fragmentCount);
            byte[] appSpecificBytes = BitConverter.GetBytes(applicationAppSpecific);
            byte[] correlationIdBytes = Encoding.Unicode.GetBytes(applicationCorrelationId);
            byte[] correlationIdLengthBytes = BitConverter.GetBytes(correlationIdBytes.Length);

            byte[] b = new byte[sizeof(int) + sizeof(int) + sizeof(int) + correlationIdBytes.Length];
            System.Buffer.BlockCopy(fragmentCountBytes, 0, b, 0, fragmentCountBytes.Length);
            System.Buffer.BlockCopy(appSpecificBytes, 0, b, fragmentCountBytes.Length, appSpecificBytes.Length);
            System.Buffer.BlockCopy(correlationIdLengthBytes, 0, b, fragmentCountBytes.Length + appSpecificBytes.Length, correlationIdLengthBytes.Length);
            System.Buffer.BlockCopy(correlationIdBytes, 0, b, fragmentCountBytes.Length + appSpecificBytes.Length + correlationIdLengthBytes.Length, correlationIdBytes.Length);

            if (this.useBody)
            {
                if (this.stream != null)
                {
                    if (this.message.Body is Stream)
                    {
                        this.message.Body = Utilities.GetStreamFromBytes(b);
                    }
                    else
                    {
                        this.message.Body = b;
                    }
                }
                else
                {
                    this.message.Body = b;
                }
            }
            else
            {
                this.message.BodyStream = Utilities.GetStreamFromBytes(b);
            }
        }

        // Receives message (first/application-specified correlationId) from status queue
        // On delete flag set to true, the method receives message without transaction context
        private Message ReceiveFromStatusQueue(long lookupId, MessageQueueTransaction transaction, MessageQueueTransactionType transactionType, bool delete)
        {
            bool transactionTypeFlag = (transactionType == MessageQueueTransactionType.Automatic) ? true : false;
            string statusQueuePath = "FormatName:DIRECT=OS:" + this.queue.MachineName + @"\" + Parameters.STATUS_QUEUE;
            MessageQueue statusQueue = new MessageQueue(statusQueuePath);
            try
            {
                bool canRead = statusQueue.CanRead;
                bool canWrite = statusQueue.CanWrite;
                if ((!canRead) || (!canWrite))
                {
                    throw new LargeMessageQueueException(null, 
                        "Transactional status queue should have both read and write access for this user. Current status is: Read=" + 
                        canRead + ", Write=" + canWrite);
                }
            }
            catch (MessageQueueException mqe)
            {
                throw new LargeMessageQueueException("Transactional status queue not available", mqe);
            }

            statusQueue.Formatter = new BinaryMessageFormatter();
            statusQueue.MessageReadPropertyFilter.AppSpecific = true;
            statusQueue.MessageReadPropertyFilter.CorrelationId = true;

            Message statusMessage = null;
            try
            {
                if (lookupId == 0)
                {
                    if (transactionTypeFlag)
                    {
                        statusMessage = statusQueue.ReceiveByLookupId(MessageLookupAction.First, 0, transactionType);
                    }
                    else
                    {
                        statusMessage = statusQueue.ReceiveByLookupId(MessageLookupAction.First, 0, transaction);
                    }
                }
                else
                {
                    if (delete)
                    {
                        statusMessage = statusQueue.ReceiveByLookupId(lookupId);
                    }
                    else
                    {
                        if (transactionTypeFlag)
                        {
                            statusMessage = statusQueue.ReceiveByLookupId(MessageLookupAction.Current, lookupId, transactionType);
                        }
                        else
                        {
                            statusMessage = statusQueue.ReceiveByLookupId(MessageLookupAction.Current, lookupId, transaction);
                        }
                    }
                }
            }
            catch (MessageQueueException)
            {
                // If status queue is empty, receive will throw MessageQueueException
                throw new InvalidOperationException();
            }

            statusQueue.Dispose();
            statusQueue.Close();
            return statusMessage;
        }

        // Peeks message (first/application-specified correlationId) from status queue
        private Message PeekFromStatusQueue(string correlationId)
        {
            string statusQueuePath = "FormatName:DIRECT=OS:" + this.queue.MachineName + @"\" + Parameters.STATUS_QUEUE;
            MessageQueue statusQueue = new MessageQueue(statusQueuePath);
            statusQueue.Formatter = new BinaryMessageFormatter();
            statusQueue.MessageReadPropertyFilter.AppSpecific = true;
            statusQueue.MessageReadPropertyFilter.CorrelationId = true;

            Message statusMessage = null;
            try
            {
                if (correlationId == null)
                {
                    statusMessage = statusQueue.PeekByLookupId(MessageLookupAction.First, 0);
                }
                else
                {
                    statusMessage = statusQueue.PeekByCorrelationId(correlationId, TimeSpan.Zero);
                }
            }
            catch (MessageQueueException)
            {
                // If status queue is empty, peek will throw MessageQueueException
                throw new InvalidOperationException();
            }

            statusQueue.Dispose();
            statusQueue.Close();
            return statusMessage;
        }

        // this.message is set to the first message in queue that is not same as large message id
        // If queue is empty, peek will throw MessageQueueException
        private void GetNextMessageSequenceInQueue(MessageQueue mq, string largeSequenceId)
        {
            int nextSequenceFlag = 0;
            try
            {
                if (this.message == null)
                {
                    this.message = mq.PeekByLookupId(MessageLookupAction.First, 0);
                    nextSequenceFlag = 1;
                }
                else
                {
                    do
                    {
                        string currentLargeSequenceId = null;
                        long prevLookupId = this.message.LookupId;
                        this.message = mq.PeekByLookupId(MessageLookupAction.Next, prevLookupId);
                        if (this.message.AppSpecific == Parameters.HEADER_FRAGMENT_ID)
                        {
                            currentLargeSequenceId = this.message.Id;
                        }
                        else
                        {
                            currentLargeSequenceId = this.message.CorrelationId;
                        }

                        nextSequenceFlag = String.Compare(currentLargeSequenceId, 0, largeSequenceId, 0, currentLargeSequenceId.Length, StringComparison.OrdinalIgnoreCase);
                    } while (nextSequenceFlag == 0);
                }
            }
            catch (MessageQueueException)
            {
                throw new InvalidOperationException();
            }
        }

        // Main method that performs the peek operation
        // If normal message, returns message to application
        // If large message, performs reassembly and return message to application
        // This method throws LargeMessageQueueException and does not delete fragments if holes are found
        private Message PeekGeneric(string correlationId, TimeSpan timeout)
        {
            // Holds the initial state of the filter value. this is done so that this filter
            // is reverted back to its initial state (initial = when it entered the API)
            bool receiveFilterCorrelationId = this.queue.MessageReadPropertyFilter.CorrelationId;
            bool receiveFilterAppSpecific = this.queue.MessageReadPropertyFilter.AppSpecific;
            bool receiveFilterBody = this.queue.MessageReadPropertyFilter.Body;
            bool receiveFilterLookupId = this.queue.MessageReadPropertyFilter.LookupId;
            this.queue.MessageReadPropertyFilter.CorrelationId = true;
            this.queue.MessageReadPropertyFilter.AppSpecific = true;
            this.queue.MessageReadPropertyFilter.Body = true;
            this.queue.MessageReadPropertyFilter.LookupId = true;

            try
            {
                this.stream = new MemoryStream();

                // Holds the message Id of first fragment
                string largeSequenceId = string.Empty;

                LargeMessageProperties largeMessageProperties = null;
                MessageQueue subQueue = null;

                // The overall send formatting is BinaryMessageFormatter, so the receive should also be that
                IMessageFormatter formatter = new BinaryMessageFormatter();

                int checkPeekMessageStatus = (int)Parameters.ReceiveAction.Restart;
                int statusMessageAppSpecific = (int)Parameters.ReceiveAction.None;
                Message statusMessage = null;
                Cursor mainQueueCursor = null;
                Cursor subQueueCursor = null;

                do
                {
                    // First gets the status message from the status queue, reference the sub-queue using the 
                    // identifier in status message, perform corresponding peek operation
                    if (checkPeekMessageStatus == (int)Parameters.ReceiveAction.Restart)
                    {
                        mainQueueCursor = this.queue.CreateCursor();
                        try
                        {
                            // throws exception if status queue is empty
                            statusMessage = this.PeekFromStatusQueue(correlationId);
                            statusMessage.Formatter = formatter;
                            statusMessageAppSpecific = statusMessage.AppSpecific;
                            largeSequenceId = statusMessage.CorrelationId;

                            if (statusMessageAppSpecific == (int)Parameters.ReceiveAction.Complete)
                            {
                                checkPeekMessageStatus = (int)Parameters.ReceiveAction.None;
                            }
                            else
                            {
                                throw new LargeMessageQueueException(largeSequenceId, "Fragments missing");
                            }
                        }
                        catch (InvalidOperationException)
                        {
                            this.message = this.PeekFromMsmq(correlationId, timeout, mainQueueCursor, PeekAction.Current);
                            this.message.Formatter = formatter;

                            // If normal message, return single message to application
                            if (this.message.CorrelationId == Utilities.GetEmptyCorrelationIdStr())
                            {
                                return this.message;
                            }

                            if (this.message.AppSpecific == Parameters.HEADER_FRAGMENT_ID)
                            {
                                largeSequenceId = this.message.Id;
                                checkPeekMessageStatus = (int)Parameters.ReceiveAction.Pending;
                            }
                            else
                            {
                                largeSequenceId = this.message.CorrelationId;
                                subQueue = this.GetSubQueue(largeSequenceId);
                                subQueueCursor = subQueue.CreateCursor();
                                try
                                {
                                    // Throws exception if subqueue is empty
                                    this.message = subQueue.Peek(TimeSpan.Zero, subQueueCursor, PeekAction.Current);
                                    this.message.Formatter = formatter;
                                }
                                catch (MessageQueueException)
                                {
                                    throw new LargeMessageQueueException(largeSequenceId, "Fragments from header to " + 
                                        (this.message.AppSpecific - 1) + " missing");
                                }
                                catch (InvalidOperationException)
                                {
                                    throw new LargeMessageQueueException(largeSequenceId, "Fragments from header to " +
                                        (this.message.AppSpecific - 1) + " missing");
                                }

                                if (this.message.AppSpecific == Parameters.HEADER_FRAGMENT_ID)
                                {
                                    checkPeekMessageStatus = (int)Parameters.ReceiveAction.Pending;
                                }
                                else
                                {
                                    checkPeekMessageStatus = (int)Parameters.ReceiveAction.Restart;
                                }
                            }
                        }
                    }

                    if (checkPeekMessageStatus != (int)Parameters.ReceiveAction.Restart)
                    {
                        if (subQueue != null)
                        {
                            try
                            {
                                while (true)
                                {
                                    this.message = subQueue.Peek(TimeSpan.Zero, subQueueCursor, PeekAction.Next);
                                    this.message.Formatter = formatter;
                                }
                            }
                            catch (MessageQueueException)
                            {
                                // Don't do anything and just come out of the loop
                            }
                            catch (InvalidOperationException)
                            {
                                // Don't do anything and just come out of the loop
                            }
                        }

                        try
                        {
                            // the previous AppSpecific is the last message peek from the subqueue
                            int previousAppSpecific = this.message.AppSpecific;
                            while (checkPeekMessageStatus != (int)Parameters.ReceiveAction.Complete)
                            {
                                this.message = this.PeekByCorrelationId(largeSequenceId, timeout, mainQueueCursor, PeekAction.Next);
                                this.message.Formatter = formatter;

                                if (this.message.AppSpecific == Parameters.TRAILER_FRAGMENT_ID)
                                {
                                    // If message in main queue is a trailer fragment, check if the last message in subqueue
                                    // corresponds to the last expected message with application data
                                    largeMessageProperties = this.GetLargeMessageHeader(this.message);
                                    if (previousAppSpecific != largeMessageProperties.FragmentCount)
                                    {
                                        throw new LargeMessageQueueException(largeSequenceId, "Fragments from " + 
                                            previousAppSpecific + " to trailer missing");
                                    }

                                    checkPeekMessageStatus = (int)Parameters.ReceiveAction.Complete;
                                }
                                else if (((this.message.AppSpecific != (previousAppSpecific + 1)) &&
                                    (previousAppSpecific != Parameters.HEADER_FRAGMENT_ID)) ||
                                    ((this.message.AppSpecific != 1) && (previousAppSpecific == Parameters.HEADER_FRAGMENT_ID)))
                                {
                                    if (previousAppSpecific == Parameters.HEADER_FRAGMENT_ID)
                                    {
                                        throw new LargeMessageQueueException(largeSequenceId, "Fragments from 1 to " + 
                                            (this.message.AppSpecific - 1) + " missing");
                                    }
                                    else
                                    {
                                        throw new LargeMessageQueueException(largeSequenceId, "Fragments from " + (previousAppSpecific + 1) + 
                                            " to " + (this.message.AppSpecific - 1) + 
                                            " missing");
                                    }
                                }
                                else
                                {
                                    this.AssembleFragments();
                                }

                                previousAppSpecific = this.message.AppSpecific;
                            }
                        }
                        catch (MessageQueueException)
                        {
                            // Don't do anything 
                        }
                        catch (InvalidOperationException)
                        {
                            // Don't do anything 
                        }

                        // Last fragment has the control values
                        if (this.message.AppSpecific == Parameters.TRAILER_FRAGMENT_ID)
                        {
                            // All message fragments assembled
                            this.PrepareLargeMessage(largeMessageProperties);
                            checkPeekMessageStatus = (int)Parameters.ReceiveAction.Complete;
                        }
                        else
                        {
                            throw new LargeMessageQueueException(largeSequenceId, "Fragments after " + 
                                this.message.AppSpecific + " to trailer missing");
                        }
                    }

                    if (checkPeekMessageStatus == (int)Parameters.ReceiveAction.Restart)
                    {
                        try
                        {
                            this.stream.Dispose();
                        }
                        catch (NullReferenceException)
                        {
                            // Don't do anything 
                        }

                        try
                        {
                            mainQueueCursor.Close();
                            subQueueCursor.Close();
                        }
                        catch (NullReferenceException)
                        {
                            // Don't do anything 
                        }

                        this.stream = new MemoryStream();
                    }
                } while (checkPeekMessageStatus != (int)Parameters.ReceiveAction.Complete);

                return this.message;
            }
            finally
            {
                if (this.stream != null)
                {
                    this.stream.Close();
                }

                this.queue.MessageReadPropertyFilter.CorrelationId = receiveFilterCorrelationId;
                this.queue.MessageReadPropertyFilter.AppSpecific = receiveFilterAppSpecific;
                this.queue.MessageReadPropertyFilter.LookupId = receiveFilterLookupId;
                this.queue.MessageReadPropertyFilter.Body = receiveFilterBody;
            }
        }

        // Main method that performs the receive operation
        // If normal message, returns message to application
        // If large message, performs reassembly and return message to application
        // This method throws LargeMessageQueueException and  delete fragments if holes are found
        private Message ReceiveGeneric(string correlationId, TimeSpan timeout, MessageQueueTransaction transaction, MessageQueueTransactionType transactionType)
        {
            MessageQueueTransaction internalTransaction = transaction;
            bool transactionFlag = (internalTransaction != null) ? true : false;
            MessageQueueTransaction deleteTransaction = null;

            // Holds the initial state of the filter value. this is done so that this filter
            // is reverted back to its initial state (initial = when it entered the API)
            bool receiveFilterCorrelationId = this.queue.MessageReadPropertyFilter.CorrelationId;
            bool receiveFilterAppSpecific = this.queue.MessageReadPropertyFilter.AppSpecific;
            bool receiveFilterBody = this.queue.MessageReadPropertyFilter.Body;
            bool receiveFilterLookupId = this.queue.MessageReadPropertyFilter.LookupId;
            this.queue.MessageReadPropertyFilter.CorrelationId = true;
            this.queue.MessageReadPropertyFilter.AppSpecific = true;
            this.queue.MessageReadPropertyFilter.Body = true;
            this.queue.MessageReadPropertyFilter.LookupId = true;

            try
            {
                // Creates an internal transaction if there was no transaction from the application
                if (!transactionFlag)
                {
                    internalTransaction = new MessageQueueTransaction();
                    internalTransaction.Begin();
                }

                // Holds the message Id of first fragment
                string largeSequenceId = string.Empty;

                LargeMessageProperties largeMessageProperties = null;
                MessageQueue subQueue = null;

                // The overall send formatting is BinaryMessageFormatter, so the receive should also be that
                IMessageFormatter formatter = new BinaryMessageFormatter();

                int checkPeekMessageStatus = (int)Parameters.ReceiveAction.Restart;
                int statusMessageAppSpecific = (int)Parameters.ReceiveAction.None;

                do
                {
                    // First gets the status message from the status queue, reference the sub-queue using the 
                    // identifier in status message, perform corresponding receive/reject operation
                    this.stream = new MemoryStream();
                    bool lookForStatusMessage = true;
                    Message statusMessage = null;

                    while (lookForStatusMessage)
                    {
                        try
                        {
                            // API firsts peeks from status queue and then receives/deletes the status message.
                            // This is done to clear junk status messages that may have come to occupy the status queue
                            statusMessage = this.PeekFromStatusQueue(correlationId);
                            statusMessage.Formatter = formatter;
                            statusMessageAppSpecific = statusMessage.AppSpecific;
                            largeSequenceId = (string)statusMessage.Body;
                            if (statusMessageAppSpecific == (int)Parameters.ReceiveAction.Delete)
                            {
                                try
                                {
                                    deleteTransaction = new MessageQueueTransaction();
                                    deleteTransaction.Begin();

                                    statusMessage = this.ReceiveFromStatusQueue(statusMessage.LookupId, deleteTransaction, MessageQueueTransactionType.None, false);
                                    statusMessage.Formatter = formatter;
                                    throw new LargeMessageQueueException(largeSequenceId, "Fragments missing");
                                }
                                catch (InvalidOperationException)
                                {
                                    deleteTransaction.Abort();
                                    deleteTransaction.Dispose();
                                    lookForStatusMessage = true;
                                }
                            }
                            else
                            {
                                checkPeekMessageStatus = (int)Parameters.ReceiveAction.None;
                            }

                            try
                            {
                                subQueue = this.GetSubQueue(largeSequenceId);

                                // throws exception if subqueue empty
                                this.message = subQueue.PeekByLookupId(MessageLookupAction.Last, 0);
                                this.message.Formatter = formatter;
                                try
                                {
                                    statusMessage = this.ReceiveFromStatusQueue(statusMessage.LookupId, internalTransaction, transactionType, false);
                                    statusMessage.Formatter = formatter;
                                    lookForStatusMessage = false;
                                }
                                catch (InvalidOperationException)
                                {
                                    lookForStatusMessage = true;
                                }
                            }
                            catch (InvalidOperationException)
                            {
                                // Comes here when GetSubQueue throws exception. Just do a descructive receive and start status queue peek again
                                // this exception occurs when status queue contains junk data (status message for nonexistent subqueue)
                                try
                                {
                                    statusMessage = this.ReceiveFromStatusQueue(statusMessage.LookupId, internalTransaction, transactionType, true);
                                    statusMessage.Formatter = formatter;
                                }
                                catch (InvalidOperationException)
                                { 
                                    // The status message to be received destructively has not been removed from queue. Don't do anything
                                }

                                lookForStatusMessage = true;
                            }
                        }
                        catch (InvalidOperationException)
                        {
                            lookForStatusMessage = false;

                            if (correlationId == null)
                            {
                                try
                                {
                                    this.GetNextMessageSequenceInQueue(this.queue, largeSequenceId);
                                }
                                catch (InvalidOperationException)
                                {
                                    // Empty main queue
                                    this.message = this.queue.Peek(timeout);
                                    this.message.Formatter = formatter;
                                }
                            }
                            else
                            {
                                // If correlationId does not exist in queue, throw it to application
                                this.PeekFromMsmq(correlationId, timeout, this.queue.CreateCursor(), PeekAction.Current);
                            }

                            this.message.Formatter = formatter;

                            // If normal message, return single message to application
                            if (this.message.CorrelationId == Utilities.GetEmptyCorrelationIdStr())
                            {
                                this.ReceiveFromMsmq(this.queue, this.message.LookupId, internalTransaction, transactionType);
                                return this.message;
                            }

                            if (this.message.AppSpecific == Parameters.HEADER_FRAGMENT_ID)
                            {
                                largeSequenceId = this.message.Id;
                            }
                            else
                            {
                                largeSequenceId = this.message.CorrelationId;
                            }

                            subQueue = this.GetSubQueue(largeSequenceId);
                        }

                        checkPeekMessageStatus = (int)Parameters.ReceiveAction.None;
                    }

                    while ((checkPeekMessageStatus != (int)Parameters.ReceiveAction.Complete) && (checkPeekMessageStatus != (int)Parameters.ReceiveAction.Restart))
                    {
                        if (statusMessageAppSpecific == (int)Parameters.ReceiveAction.Complete)
                        {
                            checkPeekMessageStatus = (int)Parameters.ReceiveAction.Complete;
                        }
                        else
                        {
                            checkPeekMessageStatus = this.CheckReceiveMessage(subQueue, correlationId, largeSequenceId);
                        }

                        if (checkPeekMessageStatus == (int)Parameters.ReceiveAction.Complete)
                        {
                            if (statusMessageAppSpecific == (int)Parameters.ReceiveAction.None)
                            {
                                statusMessageAppSpecific = (int)Parameters.ReceiveAction.Complete;
                                this.SendToStatusQueue(correlationId, statusMessageAppSpecific, largeSequenceId);
                                checkPeekMessageStatus = (int)Parameters.ReceiveAction.Restart;
                            }
                            else if (statusMessageAppSpecific == (int)Parameters.ReceiveAction.Complete)
                            {
                                try
                                {
                                    while (true)
                                    {
                                        this.message = this.ReceiveFromMsmq(subQueue, TimeSpan.Zero, internalTransaction, transactionType);
                                        this.message.Formatter = formatter;

                                        // First fragment has the control values
                                        if (this.message.AppSpecific == Parameters.HEADER_FRAGMENT_ID)
                                        {
                                            largeSequenceId = this.message.Id;
                                        }
                                        else if (this.message.AppSpecific == Parameters.TRAILER_FRAGMENT_ID)
                                        {
                                            largeMessageProperties = this.GetLargeMessageHeader(this.message);

                                            // All message fragments assembled
                                            this.PrepareLargeMessage(largeMessageProperties);

                                            if (!transactionFlag)
                                            {
                                                internalTransaction.Commit();
                                            }

                                            return this.message;
                                        }
                                        else
                                        {
                                            this.AssembleFragments();
                                        }
                                    }
                                }
                                finally
                                { 
                                }
                            }
                            else if (statusMessageAppSpecific == (int)Parameters.ReceiveAction.Delete)
                            {
                                throw new LargeMessageQueueException(largeSequenceId, "Fragments missing");
                            }
                        }
                        else if (checkPeekMessageStatus == (int)Parameters.ReceiveAction.Pending)
                        {
                            try
                            {
                                this.message = this.queue.PeekByCorrelationId(largeSequenceId, timeout);
                                this.message.Formatter = formatter;
                            }
                            catch (InvalidOperationException)
                            {
                                // This exception is thrown if more messages available in queue but not with largeSequenceId

                                // Checking again because the exception could be because another application may 
                                // have moved the successive fragment to subqueue
                                if (!this.CheckCompleteMessage(subQueue))
                                {
                                    throw new LargeMessageQueueException(largeSequenceId, "Fragment after " + 
                                        this.message.AppSpecific + " missing");
                                }
                                else
                                {
                                    // look for another large message sequence
                                    checkPeekMessageStatus = (int)Parameters.ReceiveAction.Restart;
                                }
                            }
                            catch (MessageQueueException)
                            {
                                // This exception is thrown if no messages available in queue

                                // Checking again because the exception could be because another application may 
                                // have moved the successive fragment to subqueue
                                if (!this.CheckCompleteMessage(subQueue))
                                {
                                    throw new LargeMessageQueueException(largeSequenceId, "Fragment after " + 
                                        this.message.AppSpecific + " missing");
                                }
                                else
                                {
                                    // look for another large message sequence
                                    checkPeekMessageStatus = (int)Parameters.ReceiveAction.Restart;
                                }
                            }
                        }
                    }

                    this.stream.Close();
                } while (checkPeekMessageStatus == (int)Parameters.ReceiveAction.Restart);

                return null;
            }
            catch (LargeMessageQueueException lqe)
            {
                if (!string.IsNullOrEmpty(lqe.CorrelationId))
                {
                    this.PerformHouseKeeping(lqe.CorrelationId);

                    if (deleteTransaction != null)
                    {
                        deleteTransaction.Commit();
                        deleteTransaction.Dispose();
                    }
                }

                if (!transactionFlag)
                {
                    internalTransaction.Abort();
                }

                throw;
            }
            catch (Exception)
            {
                if (!transactionFlag)
                {
                    internalTransaction.Abort();
                }

                if (deleteTransaction != null)
                {
                    deleteTransaction.Abort();
                    deleteTransaction.Dispose();
                }

                throw;
            }
            finally
            {
                if (this.stream != null)
                {
                    this.stream.Close();
                }

                this.queue.MessageReadPropertyFilter.CorrelationId = receiveFilterCorrelationId;
                this.queue.MessageReadPropertyFilter.AppSpecific = receiveFilterAppSpecific;
                this.queue.MessageReadPropertyFilter.LookupId = receiveFilterLookupId;
                this.queue.MessageReadPropertyFilter.Body = receiveFilterBody;
            }
        }

        // This method peeks the fragments by calling the overloaded Peek methods of the System.Messaging API
        private Message PeekFromMsmq(string correlationId, TimeSpan timeout, Cursor cursor, PeekAction action)
        {
            if (correlationId == null)
            {
                this.message = this.queue.Peek(timeout, cursor, action);
            }
            else
            {
                this.message = this.PeekByCorrelationId(correlationId, timeout, cursor, action);
            }

            return this.message;
        }

        private Message ReceiveFromMsmq(MessageQueue mq, TimeSpan timeout, MessageQueueTransaction transaction, MessageQueueTransactionType transactionType)
        {
            bool timeoutFlag = (timeout != MessageQueue.InfiniteTimeout) ? true : false;
            bool transactionTypeFlag = (transactionType == MessageQueueTransactionType.Automatic) ? true : false;
            if (timeoutFlag)
            {
                if (transactionTypeFlag)
                {
                    this.message = mq.Receive(timeout, transactionType);
                }
                else
                {
                    this.message = mq.Receive(timeout, transaction);
                }
            }
            else if (transactionTypeFlag)
            {
                this.message = mq.Receive(transactionType);
            }
            else
            {
                this.message = mq.Receive(transaction);
            }

            return this.message;
        }

        private Message ReceiveFromMsmq(MessageQueue mq, long lookupId, MessageQueueTransaction transaction, MessageQueueTransactionType transactionType)
        {
            bool transactionTypeFlag = (transactionType == MessageQueueTransactionType.Automatic) ? true : false;
            if (transactionTypeFlag)
            {
                this.message = mq.ReceiveByLookupId(MessageLookupAction.Current, lookupId, transactionType);
            }
            else
            {
                this.message = mq.ReceiveByLookupId(MessageLookupAction.Current, lookupId, transaction);
            }

            return this.message;
        }

        // Main methods to fragment large message and send to queue
        private void SendGeneric(object obj, string label, MessageQueueTransaction transaction, MessageQueueTransactionType transactionType, bool streaming)
        {
            MessageQueueTransaction internalTransaction = transaction;
            bool transactionFlag = (internalTransaction != null) ? true : false;
            bool objIsMessage = false;
            object oldBody = null;

            // Holds the initial state of the filter value. this is done so that this filter
            // is reverted back to its initial state (initial = when it entered the API)
            bool sendFilterCorrelationId = this.queue.MessageReadPropertyFilter.CorrelationId;
            bool sendFilterAppSpecific = this.queue.MessageReadPropertyFilter.AppSpecific;
            bool sendFilterBody = this.queue.MessageReadPropertyFilter.Body;
            bool sendFilterLookupId = this.queue.MessageReadPropertyFilter.LookupId;
            this.queue.MessageReadPropertyFilter.CorrelationId = true;
            this.queue.MessageReadPropertyFilter.AppSpecific = true;
            this.queue.MessageReadPropertyFilter.Body = true;
            this.queue.MessageReadPropertyFilter.LookupId = true;

            this.stream = new MemoryStream();
            if (this.fragmentSize == 0)
            {
                this.fragmentSize = Parameters.MAX_MESSAGE_SIZE;
            }

            try
            {
                Message m = obj as Message;
                if (m != null)
                {
                    objIsMessage = true;
                    try
                    {
                        oldBody = m.Body;
                    }
                    catch
                    {
                        // Don't do anything 
                    }
                }
                else
                {
                    oldBody = obj;
                }
            }
            catch (NullReferenceException)
            {
                // Don't do anything
            }

            try
            {
                // Creates an internal transaction if there was no transaction from the application
                if (!transactionFlag)
                {
                    internalTransaction = new MessageQueueTransaction();
                    internalTransaction.Begin();
                }

                this.SetMessageProperties(obj, label);
                string applicationLabel = this.message.Label;
                string applicationCorrelationId = this.message.CorrelationId;
                int applicationAppSpecific = this.message.AppSpecific;

                int fragmentCount = 0;
                if (fragmentCount == 0)
                {
                    // Preparing header fragment
                    this.PrepareLargeMessageFragment(fragmentCount, applicationAppSpecific, applicationCorrelationId);
                    this.message.AppSpecific = Parameters.HEADER_FRAGMENT_ID;
                    this.message.Label = applicationLabel + ".part." + this.message.AppSpecific;
                    if (applicationCorrelationId.Length == 0)
                    {
                        this.message.CorrelationId = Utilities.GetLargeMessageIdStr(true, null);
                    }

                    this.SendToMsmq(label, internalTransaction, transactionType);

                    // The correlation id of all following fragments is set to the id of first message just sent.
                    this.message.CorrelationId = this.message.Id;
                }

                bool moreStreamAvailable = true;
                bool pause = false;
                while (moreStreamAvailable)
                {
                    bool sendNullBodyOrBodyStream = false;
                    if ((this.stream == null) || (this.stream.Length == 0))
                    {
                        sendNullBodyOrBodyStream = true;
                        streaming = false;
                    }
                    else
                    {
                        sendNullBodyOrBodyStream = false;
                    }

                    if (streaming)
                    {
                        this.stream = new MemoryStream();
                        int b = 0;
                        try
                        {
                            while ((!pause) && ((b = ((Stream)obj).ReadByte()) != -1))
                            {
                                this.stream.WriteByte((byte)b);
                                if (this.stream.Length == this.fragmentSize)
                                {
                                    pause = true;
                                }
                            }
                        }
                        catch (ObjectDisposedException)
                        {
                            streaming = false;
                        }

                        if (b == -1)
                        {
                            streaming = false;
                        }

                        this.stream.Position = 0;
                    }

                    long sizeRemaining = 0;
                    if (this.stream != null)
                    {
                        sizeRemaining = this.stream.Length;
                    }

                    // Get each fragment, and send. (+1) is for sending the large message header
                    while (sizeRemaining > 0 || sendNullBodyOrBodyStream)
                    {
                        // Set to false the first time loop is executed
                        sendNullBodyOrBodyStream = false;

                        // The size of the last part of the object might differ as an object doesn't split equally
                        if (sizeRemaining < (long)this.fragmentSize)
                        {
                            this.fragmentSize = (int)sizeRemaining;
                        }

                        // Calculate the remaining size
                        sizeRemaining = sizeRemaining - (long)this.fragmentSize;

                        this.PrepareLargeMessageFragment();

                        // Setting the field to the fragment number
                        this.message.AppSpecific = (++fragmentCount);

                        // Set the message label to label + fragment number
                        this.message.Label = applicationLabel + ".part." + this.message.AppSpecific;

                        this.SendToMsmq(label, internalTransaction, transactionType);
                    }

                    if (streaming)
                    {
                        moreStreamAvailable = true;
                        pause = false;
                        this.stream.Dispose();
                    }
                    else
                    {
                        moreStreamAvailable = false;
                    }
                }

                // Preparing trailer fragment
                this.PrepareLargeMessageFragment(fragmentCount, applicationAppSpecific, applicationCorrelationId);
                this.message.AppSpecific = Parameters.TRAILER_FRAGMENT_ID;
                this.message.Label = applicationLabel + ".part." + this.message.AppSpecific;

                this.SendToMsmq(label, internalTransaction, transactionType);

                // Restoring initial values to the message object
                this.message.CorrelationId = applicationCorrelationId;
                this.message.Label = applicationLabel;
                this.message.AppSpecific = applicationAppSpecific;

                if (!transactionFlag)
                {
                    internalTransaction.Commit();
                }
            }
            catch
            {
                // Exception catch or MessageQueueException catch
                if (!transactionFlag)
                {
                    internalTransaction.Abort();
                }

                throw;
            }
            finally
            {
                if (objIsMessage)
                {
                    this.message.Body = oldBody;
                    this.message.BodyStream = this.stream;
                }
                else
                {
                    obj = oldBody;
                    try
                    {
                        this.message = null;
                        this.stream.Close();
                    }
                    catch (NullReferenceException)
                    {
                        // Don't do anything
                    }
                }

                this.queue.MessageReadPropertyFilter.CorrelationId = sendFilterCorrelationId;
                this.queue.MessageReadPropertyFilter.AppSpecific = sendFilterAppSpecific;
                this.queue.MessageReadPropertyFilter.Body = sendFilterBody;
                this.queue.MessageReadPropertyFilter.LookupId = sendFilterLookupId;
            }
        }

        // This method sends the fragments by calling the overloaded Send methods of the System.Messaging API
        private void SendToMsmq(string label, MessageQueueTransaction transaction, MessageQueueTransactionType transactionType)
        {
            bool labelFlag = (label != null) ? true : false;
            bool transactionTypeFlag = (transactionType == MessageQueueTransactionType.Automatic) ? true : false;

            if (labelFlag)
            {
                if (transactionTypeFlag)
                {
                    this.queue.Send(this.message, label, transactionType);
                }
                else
                {
                    this.queue.Send(this.message, label, transaction);
                }
            }
            else if (transactionTypeFlag)
            {
                this.queue.Send(this.message, transactionType);
            }
            else
            {
                this.queue.Send(this.message, transaction);
            }
        }

        // Sends status message to status queue.
        // 0 - subqueue complete, 1 - subqueue ready for delete
        private void SendToStatusQueue(string correlationId, int subQueueStatus, string largeSequenceId)
        {
            string statusQueuePath = "FormatName:DIRECT=OS:" + this.queue.MachineName + @"\" + Parameters.STATUS_QUEUE;
            MessageQueue statusQueue = new MessageQueue(statusQueuePath);
            try
            {
                bool canRead = statusQueue.CanRead;
                bool canWrite = statusQueue.CanWrite;
                if ((!canRead) || (!canWrite))
                {
                    throw new LargeMessageQueueException(null, 
                        "Transactional status queue should have both read and write access for this user. Current status is: Read=" + 
                        canRead + ", Write=" + canWrite);
                }
            }
            catch (MessageQueueException mqe)
            {
                throw new LargeMessageQueueException("Transactional status queue not available", mqe);
            } 
            
            Message statusMessage = new Message();
            statusMessage.AppSpecific = subQueueStatus;
            statusMessage.Formatter = new BinaryMessageFormatter();
            statusMessage.Body = largeSequenceId;
            if (correlationId != null)
            {
                statusMessage.CorrelationId = correlationId;
            }

            statusQueue.Send(statusMessage, MessageQueueTransactionType.Single);

            statusQueue.Dispose();
            statusQueue.Close();
        }

        // If the obj is Message, then the formatter is retrived if one is provided by the application.
        // The application formatter is used to serialize the entire object.
        // If the application did not provide a formatter, then XMLMessageFormatter (default) is used by this API
        // If the object is not a Message, then a message is created, default formatter is used and the object is set
        // to the message body (The same sequence is followed in the internal MSMQ API)
        // IMPORTANT
        // If the obj, which is not a message, is passed to MSMQ, MSMQ creates a message, applies default formatter 
        // and put the obj in the body. However, in this case, we will not get the message id since the message is
        // created internally. In order to get the message id after send, the API performs the same operations
        private void SetMessageProperties(object obj, string label)
        {
            bool nullBody = false;
            this.message = obj as Message;
            if (this.message != null)
            {
                if (this.message.Formatter == null)
                {
                    this.message.Formatter = new XmlMessageFormatter();
                }
            }
            else
            {
                this.message = new Message();
                this.message.Formatter = new XmlMessageFormatter();
                if (obj == null)
                {
                    nullBody = true;                    
                }
                else
                {
                    this.message.Body = obj;
                }
            }

            try
            {
                if ((this.message.Body == null) && (this.message.BodyStream.Length == 0))
                {
                    nullBody = true;
                }
                
                this.useBody = true;
            }
            catch
            {
                this.useBody = true;
                nullBody = true;
                try
                {
                    if (this.message.BodyStream.Length != 0)
                    {
                        this.useBody = false;
                        nullBody = false;
                    }
                }
                catch
                {
                    // Don't do anything
                }
            }

            if (nullBody)
            {
                this.stream = null;
            }
            else
            {
                if (this.useBody)
                {
                    if (this.message.Body is Stream)
                    {
                        this.stream = (Stream)this.message.Body;
                    }
                    else
                    {
                        // If the object is not a stream, then the entire object is serialized by application-specified formatter
                        // This is done to handle custom formatter which may deal with individual object (class) properties
                        Message m = new Message();
                        m.Formatter = this.message.Formatter;
                        this.message.Formatter.Write(m, this.message.Body); 
                        this.stream = m.BodyStream;
                        this.stream.Position = 0;
                    }

                    this.useBody = true;
                }
                else
                {
                    this.stream = (Stream)this.message.BodyStream;
                    this.stream.Position = 0;
                    this.useBody = false;
                }
            }

            // The final formatting is set here before transmitting on the wire
            this.message.Formatter = new BinaryMessageFormatter();

            if (label != null)
            {
                this.message.Label = label;
            }
        }
    }
}
// 
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//

using System;
using System.Threading;
using Microsoft.Management.Infrastructure;
using Microsoft.Management.Infrastructure.Options;

namespace SampleDotNetClient
{
    internal class TestObserver<T> : IObserver<T>, IDisposable
    {
        private readonly ManualResetEventSlim doneEvent = new ManualResetEventSlim(false);

        public void OnNext(T value)
        {
            CimInstance instance = value as CimInstance;
            if (instance != null)
            {
                SampleCimOperation.PrintCimInstance(instance);
                return;
            }

            CimMethodResult methodResult = value as CimMethodResult;
            if (methodResult != null)
            {
                SampleCimOperation.PrintCimMethodResult(methodResult);
                return;
            }

            CimMethodStreamedResult methodStreamResult = value as CimMethodStreamedResult;
            if (methodStreamResult != null)
            {
                SampleCimOperation.PrintCimMethodStreamResult(methodStreamResult);
            }

            CimSubscriptionResult subscriptionResult = value as CimSubscriptionResult;
            if (subscriptionResult != null)
            {
                SampleCimOperation.PrintCimInstance(subscriptionResult.Instance);
                return;
            }
        }

        public void OnError(Exception error)
        {
            CimException cimException = error as CimException;
            if (cimException != null)
            {
                SampleCimOperation.PrintCimException(cimException);
            }
            else
            {
                throw error;
            }

            this.doneEvent.Set();
        }

        public void OnCompleted()
        {
            this.doneEvent.Set();
        }

        public void WaitForCompletion()
        {
            this.doneEvent.Wait();
        }

        #region IDisposable Members

        /// <summary>
        /// Releases resources associated with this object
        /// </summary>
        public void Dispose()
        {
            this.Dispose(true);
            GC.SuppressFinalize(this);
        }

        /// <summary>
        /// Releases resources associated with this object
        /// </summary>
        private void Dispose(bool disposing)
        {
            if (disposed)
            {
                return;
            }

            if (disposing)
            {
                doneEvent.Dispose();
            }

            disposed = true;
        }

        private bool disposed = true;

        #endregion IDisposable Members
    }

    internal class CimExtension
    {
        private CimExtension() { }
        public static void WriteMessage(uint channel, string message)
        {
            Console.WriteLine("Got WriteMessage callback");
            Console.WriteLine("Channel: {0} , Message: {1}", channel, message);
        }

        public static void WriteProgress(
            string activity,
            string currentOperation,
            string statusDescription,
            uint percentageCompleted,
            uint secondsRemaining)
        {
            Console.WriteLine("Got WriteProgress callback");
            Console.WriteLine(
                "CurrentOperation: {0} , StatusDescription: {1} , PercentageCompleted: {2} , SecondsRemaining {3}.",
                currentOperation, 
                statusDescription, 
                percentageCompleted, 
                secondsRemaining);
        }

        public static CimResponseType WriteError(CimInstance instance)
        {
            Console.WriteLine("Got WriteError callback");
            Console.WriteLine("Printing WriteError instance ...");
            SampleCimOperation.PrintCimInstance(instance);

            // Prompt the user to get proper Cim Response Type
            return CimResponseType.Yes;
        }

        public static CimResponseType PromptUser(string message, CimPromptType prompt)
        {
            Console.WriteLine("Got PromptUser callback");
            Console.WriteLine("Message : {0}", message);

            // Prompt the user to get proper Cim Response Type
            return CimResponseType.Yes;
        }
    }
}

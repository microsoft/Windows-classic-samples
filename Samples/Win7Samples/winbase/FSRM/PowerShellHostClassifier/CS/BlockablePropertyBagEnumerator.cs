// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved


using System;
using System.Collections;
using System.Threading;
using System.Management.Automation;

namespace Microsoft.Samples.Fsrm.PowerShellHostClassifier
{
    /// <summary>
    /// A class that allows us to ensure powershell gets one object at a time
    /// and to find out when powershell finshes processing one object and requests the next
    /// </summary>
    public class BlockablePropertyBagEnumerator : IEnumerator, IDisposable
    {
        bool m_inputEnded;
        PSObject m_nextValue;
        PSObject m_currentValue;

        ManualResetEvent m_DataRequestedWait;
        ManualResetEvent m_DataAddedWait;

        #region IEnumerator Members

        /// <summary>
        /// Get the current Value
        /// </summary>
        object IEnumerator.Current
        {
            get
            {
                return m_currentValue;
            }
        }

        
        /// <summary>
        /// Move the to the next object, will block untill we have more data to use
        /// </summary>
        /// <returns>True if MoveNext did not traverse past end of elements, false if it did</returns>
        bool IEnumerator.MoveNext()
        {
            // trigger we are waiting for data
            m_DataRequestedWait.Set();

            // wait for data to be added
            m_DataAddedWait.WaitOne();

            m_DataAddedWait.Reset();
            m_DataRequestedWait.Reset();

            // now that we have recieved data lets set it to current
            // this data can be no more data if input has ended
            m_currentValue = m_nextValue;
            return !m_inputEnded;
        }

        /// <summary>
        /// Not Implemented
        /// </summary>
        public void Reset()
        {
            throw new NotImplementedException("The method or operation is not implemented.");
        }


        #endregion

        /// <summary>
        /// Constructor
        /// </summary>
        public BlockablePropertyBagEnumerator()
        {
            m_DataRequestedWait = new ManualResetEvent( false );
            m_DataAddedWait = new ManualResetEvent( false );
        }

        /// <summary>
        /// Destructor
        /// </summary>
        ~BlockablePropertyBagEnumerator()
        {
            Dispose(false);
        }

        /// <summary>
        /// Dispose Method
        /// </summary>
        public void Dispose()
        {
            Dispose(true);
            GC.SuppressFinalize(this);
        }

        /// <summary>
        /// Dispose Method
        /// </summary>
        /// <param name="disposing">If should free managed objects</param>
        protected virtual void Dispose(bool disposing)
        {
            if (disposing)
            {
                m_DataRequestedWait.Close();
                m_DataAddedWait.Close();
            }
        }

        /// <summary>
        /// Allow Subsequent / current blocked MoveNext calls to state no more items
        /// </summary>
        public void EndInput()
        {
            m_inputEnded = true;
            m_nextValue = null;
            m_DataAddedWait.Set();
        }

        /// <summary>
        /// Insert data into the next value of the enumerator
        /// </summary>
        /// <param name="Data"></param>
        public void InsertData(PSObject Data)
        {
            // At this time powershell MUST be contending on DataAddedWait
            m_DataRequestedWait.Reset();
            m_nextValue = Data;
            m_DataAddedWait.Set();
        }

        /// <summary>
        /// Reset the blocakble enum into a state when it had no data
        /// </summary>
        public void ResetBlockableEnum()
        {
            m_DataRequestedWait.Reset();
            m_DataAddedWait.Reset();
            m_inputEnded = false;
            m_nextValue = null;
            m_currentValue = null;
        }

        /// <summary>
        /// An ManualResetEvent Handle that states if data has been requested
        /// </summary>
        public ManualResetEvent RequestedDataWaitHandle
        {
            get 
            { 
                return m_DataRequestedWait; 
            }
        }
    }
}

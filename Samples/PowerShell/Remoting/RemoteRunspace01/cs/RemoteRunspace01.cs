// <copyright file="RemoteRunspace01.cs" company="Microsoft Corporation">
// Copyright (c) 2009 Microsoft Corporation. All rights reserved.
// </copyright>
// DISCLAIMER OF WARRANTY: The software is licensed “as-is.” You 
// bear the risk of using it. Microsoft gives no express warranties, 
// guarantees or conditions. You may have additional consumer rights 
// under your local laws which this agreement cannot change. To the extent 
// permitted under your local laws, Microsoft excludes the implied warranties 
// of merchantability, fitness for a particular purpose and non-infringement.

namespace Microsoft.Samples.PowerShell.Runspaces
{
    using System;
    using System.Management.Automation;             // Windows PowerShell namespace.
    using System.Management.Automation.Runspaces;   // Windows PowerShell namespace.

    /// <summary>
    /// This class contains the Main enrty point for the application.
    /// </summary>
    internal class RemoteRunspace01
    {
        /// <summary>
        /// This sample shows how to use WSManConnectionInfo class to set
        /// various timeouts and how to establish a remote connection.
        /// </summary>
        /// <param name="args">This parameter is not used.</param>
        public static void Main(string[] args)
        {
            // Create a WSManConnactionInfo object using the default constructor 
            // to connect to the "localHost". The WSManConnectionInfo object can 
            // also specify connections to remote computers.
            WSManConnectionInfo connectionInfo = new WSManConnectionInfo();
            
            // Set the OpertationTimeout property. The OperationTimeout is used to tell 
            // Windows PowerShell how long to wait (in milliseconds) before timing out 
            // for any operation. This includes sending input data to the remote computer, 
            // receiving output data from the remote computer, and more. The user can 
            // change this timeout depending on whether the connection is to a computer 
            // in the data center or across a slow WAN.
            connectionInfo.OperationTimeout = 4 * 60 * 1000; // 4 minutes.
 
            // Set the OpenTimeout property. OpenTimeout is used to tell Windows PowerShell 
            // how long to wait (in milliseconds) before timing out while establishing a 
            // remote connection. The user can change this timeout depending on whether the 
            // connection is to a computer in the data center or across a slow WAN.
            connectionInfo.OpenTimeout = 1 * 60 * 1000; // 1 minute.

            // Create a remote runspace using the connection information.
            using (Runspace remoteRunspace = RunspaceFactory.CreateRunspace(connectionInfo))
            {
                // Establish the connection by calling the Open() method to open the runspace. 
                // The OpenTimeout value set previously will be applied while establishing 
                // the connection. Establishing a remote connection involves sending and 
                // receiving some data, so the OperationTimeout will also play a role in this process.
                 remoteRunspace.Open();

                // Add the code to run commands in the remote runspace here. The 
                // OperationTimeout value set previously will play a role here because 
                // running commands involves sending and receiving data.
 
                // Close the connection by calling the Close() method to close the remote 
                // runspace. The Dispose() method (called by using primitive) will call 
                // the Close() method if it is not already called.
                remoteRunspace.Close();
            }
        }
    }
}


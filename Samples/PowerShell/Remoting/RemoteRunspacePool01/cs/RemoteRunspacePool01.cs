// <copyright file="RemoteRunspacePool01.cs" company="Microsoft Corporation">
// Copyright (c) 2009 Microsoft Corporation. All rights reserved.
// </copyright>
// DISCLAIMER OF WARRANTY: The software is licensed “as-is.” You 
// bear the risk of using it. Microsoft gives no express warranties, 
// guarantees or conditions. You may have additional consumer rights 
// under your local laws which this agreement cannot change. To the extent 
// permitted under your local laws, Microsoft excludes the implied warranties 
// of merchantability, fitness for a particular purpose and non-infringement.

namespace Samples
{
    using System;
    using System.Management.Automation;            // Windows PowerShell namespace.
    using System.Management.Automation.Runspaces;  // Windows PowerShell namespace.
 
    /// <summary>
    /// This class contains the Main enrty point for the application.
    /// </summary>
    internal class RemoteRunspacePool01
    {
        /// <summary>
        /// This sample shows how to construct a remote RunspacePool and run
        /// multiple commands concurrently using the runspaces of the pool.
        /// </summary>
        /// <param name="args">Parameter is not used.</param>
         public static void Main(string[] args)
        {
            // Create a WSManConnectionInfo object using the default constructor to 
            // connexct to the "localhost". The WSManConnectionInfo object can also 
            // specify connections to remote computers.
            WSManConnectionInfo connectionInfo = new WSManConnectionInfo();

            // Create a remote runspace pool that uses the WSManConnectionInfo object.  
            // The minimum runspaces value of 1 and maximum runspaces value of 2 allows
            // Windows PowerShell to open a maximum of 2 runspaces at the same time so 
            // that two commands can be run concurrently.
            using (RunspacePool remoteRunspacePool =
                RunspaceFactory.CreateRunspacePool(1, 2, connectionInfo))
            {
                // Call the Open() method to open a runspace and establish the connection. 
                remoteRunspacePool.Open();
                
                // Call the Create() method to create a pipeline, call the AddCommand(string) 
                // method to add the "get-process" command, and then call the BeginInvoke() 
                // method to run the command asynchronously using the runspace pool. 
                PowerShell gpsCommand = PowerShell.Create().AddCommand("get-process");
                gpsCommand.RunspacePool = remoteRunspacePool;
                IAsyncResult gpsCommandAsyncResult = gpsCommand.BeginInvoke();
                
                // The previous call does not block the current thread because it is 
                // running asynchronously. Because the remote runspace pool can open two 
                // runspaces, the second command can be run.
                PowerShell getServiceCommand = PowerShell.Create().AddCommand("get-service");
                getServiceCommand.RunspacePool = remoteRunspacePool;
                IAsyncResult getServiceCommandAsyncResult = getServiceCommand.BeginInvoke();
                 
                // When you are ready to handle the output, wait for the command to complete 
                // before extracting results. A call to the EndInvoke() method will block and return 
                // the output.
                PSDataCollection<PSObject> gpsCommandOutput = gpsCommand.EndInvoke(gpsCommandAsyncResult);
                
                // Process the output as needed.
                if ((gpsCommandOutput != null) && (gpsCommandOutput.Count > 0))
                {
                    Console.WriteLine("The first output from running get-process command: ");
                    Console.WriteLine(
                        "Process Name: {0} Process Id: {1}",
                        gpsCommandOutput[0].Properties["ProcessName"].Value,
                        gpsCommandOutput[0].Properties["Id"].Value);
                    Console.WriteLine();
                }
             
                // Now process the output from second command. As discussed previously, wait 
                // for the command to complete before extracting the results.
                PSDataCollection<PSObject> getServiceCommandOutput = getServiceCommand.EndInvoke(
                    getServiceCommandAsyncResult);
           
                // Process the output of the second command as needed.
                if ((getServiceCommandOutput != null) && (getServiceCommandOutput.Count > 0))
                {
                    Console.WriteLine("The first output from running get-service command: ");
                    Console.WriteLine(
                        "Service Name: {0} Description: {1} State: {2}",
                        getServiceCommandOutput[0].Properties["ServiceName"].Value,
                        getServiceCommandOutput[0].Properties["DisplayName"].Value,
                        getServiceCommandOutput[0].Properties["Status"].Value);
                }
        
                // Once done with running all the commands, close the remote runspace pool.
                // The Dispose() (called by using primitive) will call Close(), if it
                // is not already called.
                remoteRunspacePool.Close();
            }
        }
    }
}


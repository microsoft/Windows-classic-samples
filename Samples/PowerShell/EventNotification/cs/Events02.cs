// <copyright file="Events02.cs" company="Microsoft Corporation">
// Copyright (c) 2009 Microsoft Corporation. All rights reserved.
// </copyright>
// DISCLAIMER OF WARRANTY: The software is licensed “as-is.” You 
// bear the risk of using it. Microsoft gives no express warranties, 
// guarantees or conditions. You may have additional consumer rights 
// under your local laws which this agreement cannot change. To the extent 
// permitted under your local laws, Microsoft excludes the implied warranties 
// of merchantability, fitness for a particular purpose and non-infringement.

namespace Sample
{
    using System;
    using System.Management.Automation;
    using System.Management.Automation.Runspaces;

    /// <summary>
    /// This class contains the Main entry point for the application.
    /// </summary>
    internal class Events02
    {
        /// <summary>
        /// Count of events received by the local computer.
        /// </summary>
        private static int eventsReceived = 0;

        /// <summary>
        /// The main entry point where the PowerShell API is used to run commands and 
        /// where a handler for PSEventReceived events is defined. 
        /// </summary>
        private static void Main()
        {
            // Create a WSManConnectionInfo object using the default constructor 
            // to connect to the "localHost". This sample uses a connection to the 
            // local computer for simplicity. The WSManConnectionInfo object can 
            // also specify connections to remote computers.
            RunspaceConnectionInfo connectionInfo = new WSManConnectionInfo();

            // Create a remote runspace.
            using (Runspace runspace = RunspaceFactory.CreateRunspace(connectionInfo))
            {
                runspace.Open();

                // The PSEventReceived event is raised on the local computer when an event is 
                // generated on the remote computer.
                runspace.Events.ReceivedEvents.PSEventReceived += Events02.OnPSEventReceived;

                // Using a PowerShell object, create a pipeline to run a script on the remote
                // computer. Note that the PowerShell object uses the remote runspace.
                PowerShell powerShell = PowerShell.Create();
                powerShell.Runspace = runspace;

                // Add the script to the pipeline. The script will generate
                // the events that the local computer will receive.
                powerShell.AddScript(
                @"
                    #
                    # This script is used as a sample to generate PowerShell events. It uses
                    # an instance of System.IO.FileSystemWatcher, which monitors the TEMP 
                    # directory and raises an event when a change is detected.
                    #
                    $tempPath = [System.IO.Path]::GetTempPath()

                    $fileSystemWatcher = New-Object System.IO.FileSystemWatcher $tempPath

                    #
                    # The Register-ObjectEvent is used to indicate to PowerShell that we want to
                    # be notified when the FileSystemWatcher raises the 'Created' event (which,
                    # in this case, indicates that a file was created under the TEMP directory).
                    #
                    # Note that this script will be executed on the remote machine. The 
                    # '-forward' parameter is required for this sample to forward the
                    # events generated on the remote computer to the client computer.
                    #
                    Register-ObjectEvent -InputObject $fileSystemWatcher -EventName 'Created' -forward

                    #
                    # The following loop creates 5 files under the TEMP directory at 1-second
                    # intervals. This will make the FileSystemWatcher raise its Created event.
                    #
                    for ($i = 0 ; $i -lt 5; $i++)
                    {
                        $tempFile = [System.IO.Path]::GetTempFileName()

                        Set-Content $tempFile 'A temporary file'

                        sleep 1
                    }
                ");

                // Invoke the pipeline that contains the script in the specified runspace.
                powerShell.Invoke();

                // Wait until we receive notifications for the 5 files created by the script.
                while (Events02.eventsReceived < 5)
                {
                    System.Threading.Thread.Sleep(1000);
                }

                // Unsubscribe from the PSEventReceived event to stop receiving the events from
                // the remote computer.
                runspace.Events.ReceivedEvents.PSEventReceived -= Events02.OnPSEventReceived;
            }
        }

        /// <summary>
        /// Handler for the PSEventReceived event; note that this method will be invoked when the remote
        /// computer generates a Windows PowerShell event.
        /// </summary>
        /// <param name="sender">Event sender.</param>
        /// <param name="e">Event arguments.</param>
        private static void OnPSEventReceived(object sender, PSEventArgs e)
        {
            // The information about the original event (in this case FileSystemWatcher.Created) is stored
            // in the SourceArgs property of the PSEventArgs argument of this method.
            //
            // PSEventArgs[0] will be the original event sender (in this case the remote instance of 
            // System.IO.FileSystemWatcher) and PSEventArgs[1] will be the original arguments to the event 
            // (in this case an instance of System.IO.FileSystemEventArgs). Note that these two
            // objects originated on the remote computer and the local computer receives deserialized
            // representations of the objects. The deserialized objects are of type PSObject.
            PSObject fileSystemEventArgs = (PSObject)e.SourceArgs[1];

            // For the purposes of this sample, we only output a message indicating that a file was created
            // on the remote machine, and the time it was created. Since the PSObject is only an instance
            // of a deserialized FileSystemEventArgs object, we need to use the Properties collection to 
            // access the name of the file (i.e. FileSystemEventArgs.Name).
            string fileName = (string)fileSystemEventArgs.Properties["Name"].Value;

            Console.WriteLine("File '{0}' was created on computer '{1}' at {2}", fileName, e.ComputerName, e.TimeGenerated);

            // Increment the count of events received.  
            Events02.eventsReceived++;
        }
    }
}


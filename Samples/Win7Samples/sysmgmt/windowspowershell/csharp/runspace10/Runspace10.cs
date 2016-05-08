//
// Copyright (c) 2006 Microsoft Corporation. All rights reserved.
// 
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
//

using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Diagnostics;
using System.Management.Automation;
using System.Management.Automation.Runspaces;

namespace Microsoft.Samples.PowerShell.Runspaces
{
    using PowerShell = System.Management.Automation.PowerShell;
    
    #region GetProcCommand

    /// <summary>
    /// Class that implements the GetProcCommand
    /// </summary>
    [Cmdlet(VerbsCommon.Get, "Proc")]
    public class GetProcCommand : Cmdlet
    {
        #region Cmdlet Overrides

        /// <summary>
        /// For each of the requested process names, retrieve and write
        /// the associated processes.
        /// </summary>
        protected override void ProcessRecord()
        {
            // Get the current processes
            Process[] processes = Process.GetProcesses();

            // Write the processes to the pipeline making them available
            // to the next Cmdlet.  The "true" tells the system to
            // enumerate the array, and send one process at a time to
            // the pipeline.
            WriteObject(processes, true);
        }

        #endregion Overrides

    } //GetProcCommand

    #endregion GetProcCommand

    class Runspace10
    {
        /// <summary>
        /// Sample adds a cmdlet to InitialSessionState and then uses 
        /// modified InitialSessionState to create the Runspace.
        /// </summary>
        /// <param name="args">Unused</param>
        static void Main(string[] args)
        {
            //Create a default InitialSessionState 
            InitialSessionState iss = InitialSessionState.CreateDefault();
            //Add get-proc cmdlet to the session state
            SessionStateCmdletEntry ssce = new SessionStateCmdletEntry("get-proc", typeof(GetProcCommand), null);
            iss.Commands.Add(ssce);
            
            // Create a runspace. 
            // (Note that no PSHost instance is supplied in the constructor so the
            // default PSHost implementation is used. See the Hosting topics for
            // more information on creating your own PSHost class.)

            Runspace myRunSpace = RunspaceFactory.CreateRunspace(iss);
            myRunSpace.Open();

            PowerShell powershell = PowerShell.Create();
            powershell.Runspace = myRunSpace;

            // Create a pipeline with get-proc command (from this assembly)
            powershell.AddCommand("get-proc");

            Collection<PSObject> results = powershell.Invoke();

            Console.WriteLine("Process              HandleCount");
            Console.WriteLine("--------------------------------");

            // Print out each result object...
            foreach (PSObject result in results)
            {
                Console.WriteLine("{0,-20} {1}",
                    result.Members["ProcessName"].Value,
                    result.Members["HandleCount"].Value);
            }

            // Finally close the runspace
            // up any resources.
            myRunSpace.Close();

            System.Console.WriteLine("Hit any key to exit...");
            System.Console.ReadKey();
        }
    }
}


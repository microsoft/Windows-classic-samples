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
using System.Text;
using System.Management.Automation;
using System.Management.Automation.Runspaces;
using System.IO;

namespace Microsoft.Samples.PowerShell.Runspaces
{
    using PowerShell = System.Management.Automation.PowerShell;

    class Runspace05
    {
        /// <summary>
        /// This sample uses an InitialSessionState to create Runspace. It invokes
        /// a command from PowerShell snap-in present in the console file.
        /// </summary>
        /// <param name="args">Unused</param>
        /// <remarks>
        /// This sample assumes that user has the PowerShell binary module "GetProcessSample01.dll"
        /// produced in sample GetProcessSample01 copied to the current directory. 
        /// 
        /// 	This sample demonstrates the following:
	    ///         1. Creating a default instance of InitialSessionState
        ///         2. Using the InitialSessionState to create a runspace
        ///         3. Create a pipeline with the get-proc cmdlet available in the PowerShell binary module
        ///         4. Using PSObject to extract and display properties from the objects
        ///            returned by this command
        /// </remarks>
        static void Main(string[] args)
        {
            InitialSessionState iss = InitialSessionState.CreateDefault();
            PSSnapInException warning;
            iss.ImportPSSnapIn("GetProcPSSnapIn01", out warning);
            
            // Create a runspace. 
            // (Note that no PSHost instance is supplied to the CreateRunspace
            // function so the default PSHost implementation is used. See the 
            // Hosting topics for more information on creating your own PSHost class.)

            Runspace myRunSpace = RunspaceFactory.CreateRunspace(iss);
            myRunSpace.Open();

            // Create a PowerShell with get-proc command. 
            PowerShell powershell = PowerShell.Create().AddCommand("GetProcPSSnapIn01\\get-proc");
            powershell.Runspace = myRunSpace;

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


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
using System.Management.Automation.Host;
using System.Management.Automation.Runspaces;

namespace Microsoft.Samples.PowerShell.Runspaces
{
    using PowerShell = System.Management.Automation.PowerShell;

    class Runspace08
    {
        /// <summary>
        /// This sample uses the PowerShell class. It builds a pipeline that executes
        /// the get-process cmdlet piped into sort-object.  Parameters are added to a command.
        /// </summary>
        /// <param name="args">Unused</param>
        /// <remarks>
        /// 1. Creating a PowerShell object
        /// 2. Adding individual commands to that runspace
        /// 3. Adding parameters to the commands
        /// 4. Synchronously invoking the constructed pipeline.
        /// 5. Working with PSObject to extract properties from the objects returned.
        /// </remarks>
        static void Main(string[] args)
        {
            Collection<PSObject> results; // Holds the result of the pipeline execution.

            // (Note that no PSHost instance is supplied in the constructor so the
            // default PSHost implementation is used. See the Hosting topics for
            // more information on creating your own PSHost class.)

            // Create a PowerShell...
            PowerShell powershell = PowerShell.Create();

            // Use the using statement so we dispose of the Pipeline object
            // when we're done.

            using (powershell)
            {
                // Add the 'get-process' cmdlet(note that this is just the name
                // of a command, not a script.
                powershell.AddCommand("get-process");

                // Create a command object so we can set some parameters
                // for this command.
                // Sort in descending order...
                // By handlecount...
                powershell.AddCommand("sort-object").AddParameter("descending").AddParameter("property", "handlecount");

                // Execute the pipeline and save the objects returned.
                results = powershell.Invoke();
            }
            // Even after disposing of the powershell, we still need to set the
            // powershell variable to null so the garbage collector can clean it up.
            powershell = null;

            // Display the results of the execution 

            Console.WriteLine("Process              HandleCount");
            Console.WriteLine("--------------------------------");

            // Print out each result object...
            foreach (PSObject result in results)
            {
                Console.WriteLine("{0,-20} {1}",
                    result.Members["ProcessName"].Value,
                    result.Members["HandleCount"].Value);
            }

            System.Console.WriteLine("Hit any key to exit...");
            System.Console.ReadKey();
        }
    }
}


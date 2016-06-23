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

    class Runspace07
    {
        /// <summary>
        /// This sample uses the Runspace and PowerShell classes directly.
        /// It builds a pipeline that executes the get-process cmdlet
        /// piped into measure-object to count the number of processes
        /// running on the system.
        /// </summary>
        /// <param name="args">Unused</param>
        /// <remarks>
        /// This sample demonstrates the following:
        /// 1. Using the RunspaceFactory class to create a runspace.
        /// 2. Creating a PowerShell object
        /// 3. Adding individual commands to that runspace
        /// 4. Synchronously invoking the constructed pipeline.
        /// 5. Working with PSObject to extract properties from the objects returned.
        /// </remarks>
        static void Main(string[] args)
        {
            Collection<PSObject> result;     // Will hold the result
                                              // of the pipeline execution.

            // Create a runspace. We can't use the RunspaceInvoke class 
            // this time because we need to get at the underlying runspace 
            // to explicitly add the commands.
            // (Note that no PSHost instance is supplied in the constructor 
            // so the default PSHost implementation is used. See the 
            // Hosting topics for more information on creating your
            // own PSHost class.)

            Runspace myRunSpace = RunspaceFactory.CreateRunspace();
            myRunSpace.Open();

            // Create a pipeline 
            PowerShell powershell = PowerShell.Create();
            powershell.Runspace = myRunSpace;

            // Use the using statement so we dispose of the PowerShell object
            // when we're done.

            using (powershell)
            {
                // Add the 'get-process' cmdlet(note that this is just 
                // the name of a command, not a script.
                powershell.AddCommand("get-process");

                // Then add measure-object to count the number
                // of objects being returned
                powershell.AddCommand("measure-object");

                // Execute the pipeline and save the objects returned.
                result = powershell.Invoke();
            }
            // Even after disposing of the pipeLine, we still need to 
            // set the pipeLine variable to null so the garbage collector
            // can clean it up.
            powershell = null;

            // Display the results of the execution (checking that
            // everything is ok first.

            if (result == null || result.Count != 1)
            {
                throw new InvalidOperationException(
                    "pipeline.Invoke() returned the wrong number of objects");
            }
            PSMemberInfo count = result[0].Properties["Count"];
            if (count == null)
            {
                throw new InvalidOperationException(
                    "The object returned doesn't have a 'count' property");
            }

            Console.WriteLine(
                "Runspace07: 'get-process' returned {0} objects",
                count.Value);

            // Finally close the runspace and set all variables to null to free
            // up any resources.

            myRunSpace.Close();
            myRunSpace = null;

            System.Console.WriteLine("Hit any key to exit...");
            System.Console.ReadKey();
        }
    }
}


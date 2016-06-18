// <copyright file="Runspace03.cs" company="Microsoft Corporation">
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
    using System.Collections;
    using System.Management.Automation;
    using System.Management.Automation.Runspaces;
    using PowerShell = System.Management.Automation.PowerShell;

    /// <summary>
    /// This class contains the Main entry point for this host application.
    /// </summary>
    internal class Runspace03
    {
        /// <summary>
        /// This sample uses the PowerShell class to execute
        /// a script that retrieves process information for the
        /// list of process names passed into the script.
        /// It shows how to pass input objects to a script and
        /// how to retrieve error objects as well as the output objects.
        /// </summary>
        /// <param name="args">Parameter not used.</param>
        /// <remarks>
        /// This sample demonstrates the following:
        /// 1. Creating an instance of the PowerSHell class.
        /// 2. Using this instance to execute a string as a PowerShell script.
        /// 3. Passing input objects to the script from the calling program.
        /// 4. Using PSObject to extract and display properties from the objects
        ///    returned by this command.
        /// 5. Retrieving and displaying error records that were generated
        ///    during the execution of that script.
        /// </remarks>
        private static void Main(string[] args)
        {
            // Define a list of processes to look for
            string[] processNames = new string[] 
            {
              "lsass", "nosuchprocess", "services", "nosuchprocess2" 
            };

            // The script to run to get these processes. Input passed
            // to the script will be available in the $input variable.
            string script = "$input | get-process -name {$_}";

            // Create an instance of the PowerShell class.
            using (PowerShell powershell = PowerShell.Create())
            {
                powershell.AddScript(script);

                Console.WriteLine("Process              HandleCount");
                Console.WriteLine("--------------------------------");

                // Now invoke the PowerShell and display the objects that are
                // returned...
                foreach (PSObject result in powershell.Invoke(processNames))
                {
                    Console.WriteLine(
                        "{0,-20} {1}",
                        result.Members["ProcessName"].Value,
                        result.Members["HandleCount"].Value);
                }

                // Now process any error records that were generated while running the script.
                Console.WriteLine("\nThe following non-terminating errors occurred:\n");
                PSDataCollection<ErrorRecord> errors = powershell.Streams.Error;
                if (errors != null && errors.Count > 0)
                {
                    foreach (ErrorRecord err in errors)
                    {
                        System.Console.WriteLine("    error: {0}", err.ToString());
                    }
                }
            }

            System.Console.WriteLine("\nHit any key to exit...");
            System.Console.ReadKey();
        }
    }
}


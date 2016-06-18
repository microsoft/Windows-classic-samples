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
using System.Text;
using System.Management.Automation;
using System.Management.Automation.Host;
using System.Management.Automation.Runspaces;

namespace Microsoft.Samples.PowerShell.Runspaces
{
    using PowerShell = System.Management.Automation.PowerShell;

    class Runspace01
    {
        /// <summary>
        /// This sample uses the PowerShell class to execute
        /// the get-process cmdlet synchronously. The name and
        /// handlecount are then extracted from  the PSObjects
        /// returned and displayed.
        /// </summary>
        /// <param name="args">Unused</param>
        /// <remarks>
        /// This sample demonstrates the following:
        /// 1. Creating an instance of the PowerShell class.
        /// 2. Using this instance to invoke a PowerShell command.
        /// 3. Using PSObject to extract properties from the objects
        ///    returned by this command.
        /// </remarks>
        static void Main(string[] args)
        {
            // Create an instance of the PowerShell class.
            // This takes care of all building all of the other
            // data structures needed...
            PowerShell powershell = PowerShell.Create().AddCommand("get-process");

            Console.WriteLine("Process              HandleCount");
            Console.WriteLine("--------------------------------");

            // Now invoke the runspace and display the objects that are
            // returned...
            foreach (PSObject result in powershell.Invoke())
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


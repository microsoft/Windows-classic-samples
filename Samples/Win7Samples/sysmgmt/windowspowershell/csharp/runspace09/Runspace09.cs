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

    class Runspace09
    {
        /// <summary>
        /// This sample uses the PowerShell class to execute
        /// a script that generates the numbers from 1 to 10 with delays
        /// between each number. It uses the asynchronous capabilities of
        /// the pipeline to manage the execution of the pipeline and
        /// retrieve results as soon as they are available from a
        /// a script.
        /// </summary>
        /// <param name="args">Unused</param>
        /// <remarks>
        /// This sample demonstrates the following:
        /// 1. Creating instances of the PowerShell class.
        /// 2. Using these instances to execute a string as a PowerShell script.
        /// 3. Using the BeginInvoke method and the events on the PowerShell and
        ///    output pipe classes to process script output asynchronously.
        /// 4. Using the PowerShell Stop() method to interrupt an executing pipeline.
        /// </remarks>
        static void Main(string[] args)
        {
            Console.WriteLine("Print the numbers from 1 to 10. Hit any key to halt processing\n");

            PowerShell powershell = PowerShell.Create();

            // Create a pipeline with a script that generates the numbers from 1 to 10. One
            // number is generated every half second.
            powershell.AddScript("1..10 | foreach {$_ ; start-sleep -milli 500}");

            // Add the event handlers.  If we didn't care about hooking the DataAdded
            // event, we would let BeginInvoke create the output stream for us.
            PSDataCollection<PSObject> output = new PSDataCollection<PSObject>();
            output.DataAdded += new EventHandler<DataAddedEventArgs>(Output_DataAdded);
            powershell.InvocationStateChanged += new EventHandler<PSInvocationStateChangedEventArgs>(Powershell_InvocationStateChanged);
            
            IAsyncResult asyncResult = powershell.BeginInvoke<PSObject, PSObject>(null, output);

            // Wait for things to happen. If the user hits a key before the
            // pipeline has completed, then call the PowerShell Stop() method
            // to halt processing.
            Console.ReadKey();
            if (powershell.InvocationStateInfo.State != PSInvocationState.Completed)
            {
                // Stop the pipeline...
                Console.WriteLine("\nStopping the pipeline!\n");
                powershell.Stop();

                // Wait for the PowerShell state change messages to be displayed...
                System.Threading.Thread.Sleep(500);
                Console.WriteLine("\nPress a key to exit");
                Console.ReadKey();
            }
        }

        /// <summary>
        /// Output data added event handler. This event is called when
        /// there is data added to the output pipe. It reads the
        /// data available and displays it on the console.
        /// </summary>
        /// <param name="sender">The output pipe this event is associated with.</param>
        /// <param name="e">Unused</param>
        static void Output_DataAdded(object sender, DataAddedEventArgs e)
        {
            PSDataCollection<PSObject> myp = (PSDataCollection<PSObject>)sender;

            Collection<PSObject> results = myp.ReadAll();
            foreach (PSObject result in results)
            {
                Console.WriteLine(result.ToString());
            }
        }

        /// <summary>
        /// This event handler is called when the pipeline state is changed.
        /// If the state change is to Completed, it issues a message
        /// asking the user to exit the program.
        /// </summary>
        /// <param name="sender">Unused</param>
        /// <param name="e">The PowerShell state information.</param>
        static void Powershell_InvocationStateChanged(object sender, PSInvocationStateChangedEventArgs e)
        {
            Console.WriteLine("PowerShell state changed: state: {0}\n", e.InvocationStateInfo.State);
            if (e.InvocationStateInfo.State == PSInvocationState.Completed)
            {
                Console.WriteLine("Processing completed, press a key to exit!");
            }
        }
    }
}
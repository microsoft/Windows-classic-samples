// <copyright file="Host02.cs" company="Microsoft Corporation">
// Copyright (c) 2009 Microsoft Corporation. All rights reserved.
// </copyright>
// DISCLAIMER OF WARRANTY: The software is licensed “as-is.” You 
// bear the risk of using it. Microsoft gives no express warranties, 
// guarantees or conditions. You may have additional consumer rights 
// under your local laws which this agreement cannot change. To the extent 
// permitted under your local laws, Microsoft excludes the implied warranties 
// of merchantability, fitness for a particular purpose and non-infringement.

namespace Microsoft.Samples.PowerShell.Host
{
    using System;
    using System.Collections.Generic;
    using System.Globalization;
    using System.Management.Automation.Runspaces;
    using PowerShell = System.Management.Automation.PowerShell;

    /// <summary>
    /// This class contains the Main entry point for this host application.
    /// </summary>
    internal class Host02
    {
        /// <summary>
        /// Indicator to tell the host application that it should exit.
        /// </summary>
        private bool shouldExit;

        /// <summary>
        /// The exit code that the host application will use to exit.
        /// </summary>
        private int exitCode;

        /// <summary>
        /// Gets or sets a value indicating whether the host
        /// application should exit.
        /// </summary>
        public bool ShouldExit
        {
            get { return this.shouldExit; }
            set { this.shouldExit = value; }
        }
        
        /// <summary>
        /// Gets or sets the exit code that the host
        /// application will use when exiting.
        /// </summary>
        public int ExitCode
        {
            get { return this.exitCode; }
            set { this.exitCode = value; }
        }
       
        /// <summary>
        /// A sample application that uses the PowerShell runtime along with a host
        /// implementation to call get-process and display the results as you
        /// would see them in pwrsh.exe.
        /// </summary>
        /// <param name="args">Parameter not used.</param>
        private static void Main(string[] args)
        {
            // Set the current culture to German. We want this to be picked up when the MyHost
            // instance is created...
            System.Threading.Thread.CurrentThread.CurrentCulture = CultureInfo.GetCultureInfo("de-de");

            // Create the runspace, but this time we aren't using the RunspaceInvoke
            // class
            MyHost myHost = new MyHost(new Host02());
            using (Runspace myRunSpace = RunspaceFactory.CreateRunspace(myHost))
            {
                myRunSpace.Open();

                // Create a PowerShell to execute our commands...
                using (PowerShell powershell = PowerShell.Create())
                {
                    powershell.Runspace = myRunSpace;

                    // Add the script we want to run. The script does two things. It runs get-process with
                    // the output sorted by handle count, get-date piped to out-string so we can see the
                    // date being displayed in German...
                    powershell.AddScript(@"
                        get-process | sort handlecount
                        # This should display the date in German...
                        get-date | out-string
                        ");

                    // Now add the default outputter to the end of the pipe and indicate
                    // that it should handle both output and errors from the previous
                    // commands. This will result in the output being written using the PSHost
                    // and PSHostUserInterface classes instead of returning objects to the hosting
                    // application.
                    powershell.AddCommand("out-default");

                    powershell.Commands.Commands[0].MergeMyResults(PipelineResultTypes.Error, PipelineResultTypes.Output);

                    // Now just invoke the application - there won't be any objects returned -
                    // they're all consumed by out-default so we don't have to do anything more...
                    powershell.Invoke();
                }
            }

            System.Console.WriteLine("Hit any key to exit...");
            System.Console.ReadKey();
        }
    }
}


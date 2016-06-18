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
using System.Globalization;

namespace Microsoft.Samples.PowerShell.Host
{
    using PowerShell = System.Management.Automation.PowerShell;

    class Host02
    {
        /// <summary>
        /// Property that the PSHost implementation will use to tell the host
        /// application that it should exit.
        /// </summary>
        public bool ShouldExit
        {
            get { return shouldExit; }
            set { shouldExit = value; }
        }
        private bool shouldExit;

        /// <summary>
        /// Promperty that the PSHost implementation will use to tell the host
        /// application what code to use when exiting.
        /// </summary>
        public int ExitCode
        {
            get { return exitCode; }
            set { exitCode = value; }
        }
        private int exitCode;

        /// <summary>
        /// A sample application that uses the PowerShell runtime along with a host
        /// implementation to call get-process and display the results as you
        /// would see them in pwrsh.exe.
        /// </summary>
        /// <param name="args">Ignored</param>
        static void Main(string[] args)
        {
            // Set the current culture to German. We want this to be picked up when the MyHost
            // instance is created...
            System.Threading.Thread.CurrentThread.CurrentCulture = CultureInfo.GetCultureInfo("de-de");

            // Create the runspace, but this time we aren't using the RunspaceInvoke
            // class

            MyHost myHost = new MyHost(new Host02());
            Runspace myRunSpace = RunspaceFactory.CreateRunspace(myHost);
            myRunSpace.Open();

            // Create a PowerShell to execute our commands...
            PowerShell powershell = PowerShell.Create();
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

            System.Console.WriteLine("Hit any key to exit...");
            System.Console.ReadKey();
        }
    }
}


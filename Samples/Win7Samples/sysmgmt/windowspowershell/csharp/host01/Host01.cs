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

namespace Microsoft.Samples.PowerShell.Host
{
    using PowerShell = System.Management.Automation.PowerShell;

    class Host01
    {
        /// <summary>
        /// Property that the PSHost implementation will use to
        /// tell the host application that it should exit.
        /// </summary>
        public bool ShouldExit
        {
            get { return shouldExit; }
            set { shouldExit = value; }
        }
        private bool shouldExit;

        /// <summary>
        /// Promperty that the PSHost implementation will
        /// use to tell the host application what code to use
        /// when exiting.
        /// </summary>
        public int ExitCode
        {
            get { return exitCode; }
            set { exitCode = value; }
        }
        private int exitCode;

        /// <summary>
        /// This sample uses the PowerShell class to execute
        /// a script that calls exit. The host application looks at
        /// this and prints out the result.
        /// </summary>
        /// <param name="args">Unused</param>
        static void Main(string[] args)
        {
            // Create an instance of this class so that the engine will have
            // access to the ShouldExit and ExitCode parameters.
            Host01 me = new Host01();

            // Now create the host instance to use
            MyHost myHost = new MyHost(me);

            // Pass this in when creating the runspace and invoker...
            Runspace myRunSpace = RunspaceFactory.CreateRunspace(myHost);
            myRunSpace.Open();

            // Create a PowerShell to execute our commands...
            PowerShell powershell = PowerShell.Create();
            powershell.Runspace = myRunSpace;

            // Now use the runspace invoker to execute the script "exit (2+2)"
            string script = "exit (2+2)";
            powershell.AddScript(script);
            powershell.Invoke(script);

            // Check the flags and see if they were set propertly...
            Console.WriteLine(
                "ShouldExit={0} (should be True); ExitCode={1} (should be 4)",
                me.ShouldExit, me.ExitCode);

            // close the runspace...
            myRunSpace.Close();

            Console.WriteLine("Hit any key to exit...");
            Console.ReadKey();
        }
    }
}


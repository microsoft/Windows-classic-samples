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

    /// <summary>
    /// Simple PowerShell interactive console host listener implementation. This class
    /// implements a basic read-evaluate-print loop or 'listener' allowing you to
    /// interactively work with the PowerShell engine.
    /// </summary>
    class PSListenerConsoleSample
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
        /// Holds the instance of the PSHost implementation for this interpreter.
        /// </summary>
        private MyHost myHost;
        /// <summary>
        /// Holds the runspace for this interpeter.
        /// </summary>
        private Runspace myRunSpace;
        /// <summary>
        /// Holds a reference to the currently executing pipeline so it can be
        /// stopped by the control-C handler.
        /// </summary>
        private PowerShell currentPowerShell;

        /// <summary>
        /// Used to serialize access to instance data...
        /// </summary>
        private object instanceLock = new object();

        /// <summary>
        /// Create this instance of the console listener.
        /// </summary>
        PSListenerConsoleSample()
        {
            // Create the host and runspace instances for this interpreter. Note that
            // this application doesn't support console files so only the default snapins
            // will be available.
            myHost = new MyHost(this);
            myRunSpace = RunspaceFactory.CreateRunspace(myHost);
            myRunSpace.Open();
        }

        /// <summary>
        /// A helper class that builds and executes a pipeline that writes to the
        /// default output path. Any exceptions that are thrown are just passed to
        /// the caller. Since all output goes to the default outter, this method()
        /// won't return anything.
        /// </summary>
        /// <param name="cmd">The script to run</param>
        /// <param name="input">Any input arguments to pass to the script. If null
        /// then nothing is passed in.</param>
        void executeHelper(string cmd, object input)
        {
            // Just ignore empty command lines...
            if (String.IsNullOrEmpty(cmd))
                return;

            // Create the pipeline object and make it available
            // to the ctrl-C handle through the currentPowerShell instance
            // variable

            lock (instanceLock)
            {
                currentPowerShell = PowerShell.Create();
            }

            currentPowerShell.Runspace = myRunSpace;

            // Create a pipeline for this execution - place the result in the currentPowerShell
            // instance variable so it is available to be stopped.
            try
            {
                currentPowerShell.AddScript(cmd);

                // Now add the default outputter to the end of the pipe and indicate
                // that it should handle both output and errors from the previous
                // commands. This will result in the output being written using the PSHost
                // and PSHostUserInterface classes instead of returning objects to the hosting
                // application.

                currentPowerShell.AddCommand("out-default");
                currentPowerShell.Commands.Commands[0].MergeMyResults(PipelineResultTypes.Error, PipelineResultTypes.Output);



                // If there was any input specified, pass it in, otherwise just
                // execute the pipeline...

                if (input != null)
                {
                    currentPowerShell.Invoke(new object[] { input });
                }
                else
                {
                    currentPowerShell.Invoke();
                }
            }
            finally
            {
                // Dispose of the pipeline line and set it to null, locked because currentPowerShell
                // may be accessed by the ctrl-C handler...
                lock (instanceLock)
                {
                    currentPowerShell.Dispose();
                    currentPowerShell = null;
                }
            }
        }

        /// <summary>
        /// Basic script execution routine - any runtime exceptions are
        /// caught and passed back into the engine to display.
        /// </summary>
        /// <param name="cmd"></param>
        void Execute(string cmd)
        {
            try
            {
                // execute the command with no input...
                executeHelper(cmd, null);
            }
            catch (RuntimeException rte)
            {
                // An exception occurred that we want to display
                // using the display formatter. To do this we run
                // a second pipeline passing in the error record.
                // The runtime will bind this to the $input variable
                // which is why $input is being piped to out-default

                executeHelper("$input | out-default", rte.ErrorRecord);
            }
        }

        /// <summary>
        /// Method used to handle control-C's from the user. It calls the
        /// pipeline Stop() method to stop execution. If any exceptions occur
        /// they are printed to the console but otherwise ignored.
        /// </summary>
        /// <param name="sender">See ConsoleCancelEventHandler documentation</param>
        /// <param name="e">See ConsoleCancelEventHandler documentation</param>
        void HandleControlC(object sender, ConsoleCancelEventArgs e)
        {
            try
            {
                lock (instanceLock)
                {
                    if (currentPowerShell != null && currentPowerShell.InvocationStateInfo.State == PSInvocationState.Running)
                        currentPowerShell.Stop();
                }
                e.Cancel = true;
            }
            catch (Exception exception)
            {
                this.myHost.UI.WriteErrorLine(exception.ToString());
            }
        }

        /// <summary>
        /// Implements the basic listener loop. It sets up the ctrl-C handler, then
        /// reads a command from the user, executes it and repeats until the ShouldExit
        /// flag is set.
        /// </summary>
        private void Run()
        {
            // Set up the control-C handler.
            Console.CancelKeyPress += new ConsoleCancelEventHandler(HandleControlC);
            Console.TreatControlCAsInput = false;

            // loop reading commands to execute until ShouldExit is set by
            // the user calling "exit".
            while (!ShouldExit)
            {
                myHost.UI.Write(ConsoleColor.Cyan, ConsoleColor.Black, "\nPSConsoleSample: ");
                string cmd = Console.ReadLine();
                Execute(cmd);
            }

            // and exit with the desired exit code that was set by exit command.
            // This is set in the host by the MyHost.SetShouldExit() implementation.
            Environment.Exit(ExitCode);
        }

        /// <summary>
        /// Creates and initiates the listener instance...
        /// </summary>
        /// <param name="args">Ignored for now...</param>
        static void Main(string[] args)
        {
            // Display the welcome message...

            Console.Title = "PowerShell Console Host Sample Application";
            ConsoleColor oldFg = Console.ForegroundColor;
            Console.ForegroundColor = ConsoleColor.Cyan;
            Console.WriteLine("    PowerShell Console Host Interactive Sample");
            Console.WriteLine("    =====================================");
            Console.WriteLine("");
            Console.WriteLine("This is an example of a simple interactive console host using the PowerShell");
            Console.WriteLine("engine to interpret commands. Type 'exit' to exit.");
            Console.WriteLine("");
            Console.ForegroundColor = oldFg;

            // Create the listener and run it - this never returns...

            PSListenerConsoleSample listener = new PSListenerConsoleSample();
            listener.Run();
        }
    }
}


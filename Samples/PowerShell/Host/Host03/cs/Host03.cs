// <copyright file="Host03.cs" company="Microsoft Corporation">
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
    using System.Collections.ObjectModel;
    using System.Management.Automation;
    using System.Management.Automation.Runspaces;
    using PowerShell = System.Management.Automation.PowerShell;
    
    /// <summary>
    /// Simple PowerShell interactive console host listener implementation. This class
    /// implements a basic read-evaluate-print loop or 'listener' allowing you to
    /// interactively work with the PowerShell engine.
    /// </summary>
    internal class PSListenerConsoleSample
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
        private PSListenerConsoleSample()
        {
            // Create the host and runspace instances for this interpreter. Note that
            // this application doesn't support console files so only the default snapins
            // will be available.
            this.myHost = new MyHost(this);
            this.myRunSpace = RunspaceFactory.CreateRunspace(this.myHost);
            this.myRunSpace.Open();
        }
        
        /// <summary>
        /// Gets or sets a value indicating whether the host applcation
        /// should exit.
        /// </summary>
        public bool ShouldExit
        {
            get { return this.shouldExit; }
            set { this.shouldExit = value; }
        }
        
        /// <summary>
        /// Gets or sets the exit code that the host application will use 
        /// when exiting.
        /// </summary>
        public int ExitCode
        {
            get { return this.exitCode; }
            set { this.exitCode = value; }
        }

        /// <summary>
        /// Creates and initiates the listener instance.
        /// </summary>
        /// <param name="args">This parameter is not used.</param>
        private static void Main(string[] args)
        {
            // Display the welcome message...
            Console.Title = "PowerShell Console Host Sample Application";
            ConsoleColor oldFg = Console.ForegroundColor;
            Console.ForegroundColor = ConsoleColor.Cyan;
            Console.WriteLine("    PowerShell Console Host Interactive Sample");
            Console.WriteLine("    =====================================");
            Console.WriteLine(string.Empty);
            Console.WriteLine("This is an example of a simple interactive console host using the PowerShell");
            Console.WriteLine("engine to interpret commands. Type 'exit' to exit.");
            Console.WriteLine(string.Empty);
            Console.ForegroundColor = oldFg;

            // Create the listener and run it - this never returns...
            PSListenerConsoleSample listener = new PSListenerConsoleSample();
            listener.Run();
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
        private void executeHelper(string cmd, object input)
        {
            // Just ignore empty command lines...
            if (String.IsNullOrEmpty(cmd))
            {
                return;
            }

            // Create the pipeline object and make it available
            // to the ctrl-C handle through the currentPowerShell instance
            // variable
            lock (this.instanceLock)
            {
                this.currentPowerShell = PowerShell.Create();
            }

            this.currentPowerShell.Runspace = this.myRunSpace;

            // Create a pipeline for this execution - place the result in the currentPowerShell
            // instance variable so it is available to be stopped.
            try
            {
                this.currentPowerShell.AddScript(cmd);

                // Now add the default outputter to the end of the pipe and indicate
                // that it should handle both output and errors from the previous
                // commands. This will result in the output being written using the PSHost
                // and PSHostUserInterface classes instead of returning objects to the hosting
                // application.
                this.currentPowerShell.AddCommand("out-default");
                this.currentPowerShell.Commands.Commands[0].MergeMyResults(PipelineResultTypes.Error, PipelineResultTypes.Output);

                // If there was any input specified, pass it in, otherwise just
                // execute the pipeline.
                if (input != null)
                {
                    this.currentPowerShell.Invoke(new object[] { input });
                }
                else
                {
                    this.currentPowerShell.Invoke();
                }
            }
            finally
            {
                // Dispose of the pipeline line and set it to null, locked because currentPowerShell
                // may be accessed by the ctrl-C handler...
                lock (this.instanceLock)
                {
                    this.currentPowerShell.Dispose();
                    this.currentPowerShell = null;
                }
            }
        }

        /// <summary>
        /// An exception occurred that we want to display
        /// using the display formatter. To do this we run
        /// a second pipeline passing in the error record.
        /// The runtime will bind this to the $input variable
        /// which is why $input is being piped to out-string.
        /// We then call WriteErrorLine to make sure the error
        /// gets displayed in the correct error color.
        /// </summary>
        /// <param name="e">The exception to display</param>
        private void ReportException(Exception e)
        {
            if (e != null)
            {
                object error;
                IContainsErrorRecord icer = e as IContainsErrorRecord;
                if (icer != null)
                {
                    error = icer.ErrorRecord;
                }
                else
                {
                    error = (object)new ErrorRecord(e, "Host.ReportException", ErrorCategory.NotSpecified, null);
                }

                lock (this.instanceLock)
                {
                    this.currentPowerShell = PowerShell.Create();
                }

                this.currentPowerShell.Runspace = this.myRunSpace;

                try
                {
                    this.currentPowerShell.AddScript("$input").AddCommand("out-string");

                    // Don't merge errors, this function will swallow errors.
                    Collection<PSObject> result;
                    PSDataCollection<object> inputCollection = new PSDataCollection<object>();
                    inputCollection.Add(error);
                    inputCollection.Complete();
                    result = this.currentPowerShell.Invoke(inputCollection);

                    if (result.Count > 0)
                    {
                        string str = result[0].BaseObject as string;
                        if (!string.IsNullOrEmpty(str))
                        {
                            // out-string adds \r\n, we remove it
                            this.myHost.UI.WriteErrorLine(str.Substring(0, str.Length - 2));
                        }
                    }
                }
                finally
                {
                    // Dispose of the pipeline line and set it to null, locked because currentPowerShell
                    // may be accessed by the ctrl-C handler...
                    lock (this.instanceLock)
                    {
                        this.currentPowerShell.Dispose();
                        this.currentPowerShell = null;
                    }
                }
            }
        }

        /// <summary>
        /// Basic script execution routine - any runtime exceptions are
        /// caught and passed back into the engine to display.
        /// </summary>
        /// <param name="cmd">The parameter is not used.</param>
        private void Execute(string cmd)
        {
            try
            {
                // execute the command with no input...
                this.executeHelper(cmd, null);
            }
            catch (RuntimeException rte)
            {
                this.ReportException(rte);
            }
        }

        /// <summary>
        /// Method used to handle control-C's from the user. It calls the
        /// pipeline Stop() method to stop execution. If any exceptions occur
        /// they are printed to the console but otherwise ignored.
        /// </summary>
        /// <param name="sender">See sender property of ConsoleCancelEventHandler documentation</param>
        /// <param name="e">See e property of ConsoleCancelEventHandler documentation</param>
        private void HandleControlC(object sender, ConsoleCancelEventArgs e)
        {
            try
            {
                lock (this.instanceLock)
                {
                    if (this.currentPowerShell != null && this.currentPowerShell.InvocationStateInfo.State == PSInvocationState.Running)
                    {
                        this.currentPowerShell.Stop();
                    }
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
            Console.CancelKeyPress += new ConsoleCancelEventHandler(this.HandleControlC);
            Console.TreatControlCAsInput = false;

            // loop reading commands to execute until ShouldExit is set by
            // the user calling "exit".
            while (!this.ShouldExit)
            {
                this.myHost.UI.Write(ConsoleColor.Cyan, ConsoleColor.Black, "\nPSConsoleSample: ");
                string cmd = Console.ReadLine();
                this.Execute(cmd);
            }

            // and exit with the desired exit code that was set by exit command.
            // This is set in the host by the MyHost.SetShouldExit() implementation.
            Environment.Exit(this.ExitCode);
        }
    }
}


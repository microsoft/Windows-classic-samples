// <copyright file="Host06.cs" company="Microsoft Corporation">
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
    using System.Collections.ObjectModel;
    using System.Management.Automation;
    using System.Management.Automation.Host;
    using System.Management.Automation.Runspaces;
    using System.Text;
    using PowerShell = System.Management.Automation.PowerShell;
    
    /// <summary>
    /// This sample shows how to implement a basic read-evaluate-print 
    /// loop (or 'listener') that allowing you to interactively work 
    /// with the Windows PowerShell engine.
    /// </summary>
    internal class PSListenerConsoleSample
    {
        /// <summary>
        /// Used to read user input.
        /// </summary>
        internal ConsoleReadLine consoleReadLine = new ConsoleReadLine();

        /// <summary>
        /// Holds a reference to the runspace for this interpeter.
        /// </summary>
        internal Runspace myRunSpace;

        /// <summary>
        /// Indicator to tell the host application that it should exit.
        /// </summary>
        private bool shouldExit;

        /// <summary>
        /// The exit code that the host application will use to exit.
        /// </summary>
        private int exitCode;

        /// <summary>
        /// Holds a reference to the PSHost implementation for this interpreter.
        /// </summary>
        private MyHost myHost;

        /// <summary>
        /// Holds a reference to the currently executing pipeline so that it can be
        /// stopped by the control-C handler.
        /// </summary>
        private PowerShell currentPowerShell;

        /// <summary>
        /// Used to serialize access to instance data.
        /// </summary>
        private object instanceLock = new object();

        /// <summary>
        /// Gets or sets a value indicating whether the host application 
        /// should exit.
        /// </summary>
        public bool ShouldExit
        {
            get { return this.shouldExit; }
            set { this.shouldExit = value; }
        }
        
        /// <summary>
        /// Gets or sets a value indicating whether the host application 
        /// should exit.
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
            // Display the welcome message.
            Console.Title = "PowerShell Console Host Sample Application";
            ConsoleColor oldFg = Console.ForegroundColor;
            Console.ForegroundColor = ConsoleColor.Cyan;
            Console.WriteLine("    Windows PowerShell Console Host Application Sample");
            Console.WriteLine("    ==================================================");
            Console.WriteLine(string.Empty);
            Console.WriteLine("This is an example of a simple interactive console host uses ");
            Console.WriteLine("the Windows PowerShell engine to interpret commands.");
            Console.WriteLine("Type 'exit' to exit.");
            Console.WriteLine(string.Empty);
            Console.ForegroundColor = oldFg;

            // Create the listener and runs it. This method never returns.
            PSListenerConsoleSample listener = new PSListenerConsoleSample();
            listener.Run();
        }

        /// <summary>
        /// Initializes a new instance of the PSListenerConsoleSample class.
        /// </summary>
        public PSListenerConsoleSample()
        {
            // Create the host and runspace instances for this interpreter. 
            // Note that this application does not support console files so 
            // only the default snap-ins will be available.
            this.myHost = new MyHost(this);
            this.myRunSpace = RunspaceFactory.CreateRunspace(this.myHost);
            this.myRunSpace.Open();

            // Create a PowerShell object to run the commands used to create 
            // $profile and load the profiles.
            lock (this.instanceLock)
            {
                this.currentPowerShell = PowerShell.Create();
            }

            try
            {
                this.currentPowerShell.Runspace = this.myRunSpace;

                PSCommand[] profileCommands = Microsoft.Samples.PowerShell.Host.HostUtilities.GetProfileCommands("SampleHost06");
                foreach (PSCommand command in profileCommands)
                {
                    this.currentPowerShell.Commands = command;
                    this.currentPowerShell.Invoke();
                }
            }
            finally
            {
                // Dispose the PowerShell object and set currentPowerShell 
                // to null. It is locked because currentPowerShell may be 
                // accessed by the ctrl-C handler.
                lock (this.instanceLock)
                {
                    this.currentPowerShell.Dispose();
                    this.currentPowerShell = null;
                }
            }
        }

        /// <summary>
        /// A helper class that builds and executes a pipeline that writes 
        /// to the default output path. Any exceptions that are thrown are 
        /// just passed to the caller. Since all output goes to the default 
        /// outter, this method does not return anything.
        /// </summary>
        /// <param name="cmd">The script to run.</param>
        /// <param name="input">Any input arguments to pass to the script. 
        /// If null then nothing is passed in.</param>
        private void executeHelper(string cmd, object input)
        {
            // Ignore empty command lines.
            if (String.IsNullOrEmpty(cmd))
            {
                return;
            }

            // Create the pipeline object and make it available to the
            // ctrl-C handle through the currentPowerShell instance
            // variable.
            lock (this.instanceLock)
            {
                this.currentPowerShell = PowerShell.Create();
            }

            // Add a script and command to the pipeline and then run the pipeline. Place 
            // the results in the currentPowerShell variable so that the pipeline can be 
            // stopped.
            try
            {
                this.currentPowerShell.Runspace = this.myRunSpace;

                this.currentPowerShell.AddScript(cmd);

                // Add the default outputter to the end of the pipe and then call the 
                // MergeMyResults method to merge the output and error streams from the 
                // pipeline. This will result in the output being written using the PSHost
                // and PSHostUserInterface classes instead of returning objects to the host
                // application.
                this.currentPowerShell.AddCommand("out-default");
                this.currentPowerShell.Commands.Commands[0].MergeMyResults(PipelineResultTypes.Error, PipelineResultTypes.Output);

                // If there is any input pass it in, otherwise just invoke the
                // the pipeline.
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
                // Dispose the PowerShell object and set currentPowerShell to null. 
                // It is locked because currentPowerShell may be accessed by the 
                // ctrl-C handler.
                lock (this.instanceLock)
                {
                    this.currentPowerShell.Dispose();
                    this.currentPowerShell = null;
                }
            }
        }

        /// <summary>
        /// To display an exception using the display formatter, 
        /// run a second pipeline passing in the error record.
        /// The runtime will bind this to the $input variable,
        /// which is why $input is being piped to the Out-String
        /// cmdlet. The WriteErrorLine method is called to make sure 
        /// the error gets displayed in the correct error color.
        /// </summary>
        /// <param name="e">The exception to display.</param>
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
                   
                    // Do not merge errors, this function will swallow errors.
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
                            // Remove \r\n, which is added by the Out-String cmdlet.
                            this.myHost.UI.WriteErrorLine(str.Substring(0, str.Length - 2));
                        }
                    }
                }
                finally
                {
                    // Dispose of the pipeline and set it to null, locking it  because 
                    // currentPowerShell may be accessed by the ctrl-C handler.
                    lock (this.instanceLock)
                    {
                        this.currentPowerShell.Dispose();
                        this.currentPowerShell = null;
                    }
                }
            }
        }

        /// <summary>
        /// Basic script execution routine. Any runtime exceptions are
        /// caught and passed back to the Windows PowerShell engine to 
        /// display.
        /// </summary>
        /// <param name="cmd">Script to run.</param>
        private void Execute(string cmd)
        {
            try
            {
                // Run the command with no input.
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
        /// <param name="sender">See sender property documentation of  
        /// ConsoleCancelEventHandler.</param>
        /// <param name="e">See e property documentation of 
        /// ConsoleCancelEventHandler.</param>
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

            // Read commands and run them until the ShouldExit flag is set by
            // the user calling "exit".
            while (!this.ShouldExit)
            {
                string prompt;
                if (this.myHost.IsRunspacePushed)
                {
                    prompt = string.Format("\n[{0}] PSConsoleSample: ", this.myRunSpace.ConnectionInfo.ComputerName);
                }
                else
                {
                    prompt = "\nPSConsoleSample: ";
                }

                this.myHost.UI.Write(ConsoleColor.Cyan, ConsoleColor.Black, prompt);
                string cmd = this.consoleReadLine.Read();
                this.Execute(cmd);
            }

            // Exit with the desired exit code that was set by the exit command.
            // The exit code is set in the host by the MyHost.SetShouldExit() method.
            Environment.Exit(this.ExitCode);
        }
     }
}


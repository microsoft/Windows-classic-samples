// <copyright file="DebuggerSample.cs" company="Microsoft Corporation">
// Copyright (c) 2013 Microsoft Corporation. All rights reserved.
// </copyright>
// DISCLAIMER OF WARRANTY: The software is licensed “as-is.” You 
// bear the risk of using it. Microsoft gives no express warranties, 
// guarantees or conditions. You may have additional consumer rights 
// under your local laws which this agreement cannot change. To the extent 
// permitted under your local laws, Microsoft excludes the implied warranties 
// of merchantability, fitness for a particular purpose and non-infringement.

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Management.Automation;
using System.Management.Automation.Runspaces;
using System.Threading.Tasks;

namespace PSDebuggerSample
{
    public class DebuggerSample
    {
        private static void Main(string[] args)
        {
            Console.Title = "PowerShell Debugger Sample Application";
            Console.WriteLine("    Windows PowerShell Debugger Application Sample");
            Console.WriteLine("    ==============================================");
            Console.WriteLine();
            Console.WriteLine("This is a simple example of using the PowerShell Debugger API to set");
            Console.WriteLine("breakpoints in a script file, run the script and handle debugger events.");
            Console.WriteLine();

            // Create DebuggerSample class instance and begin debugging sample script.
            var debuggerSample = new DebuggerSample();
            debuggerSample.Run();
        }

        #region Private members

        /// <summary>
        /// Simple sample script that writes output, sets variables, and
        /// calls a workflow function.
        /// </summary>
        private string _script = @"
            workflow WorkflowF1
            {
                [CmdletBinding()]
                param(
                    [String]
                    $Title
                )

                Write-Output -InputObject $Title
                $PSProcess = Get-Process -Name PowerShell*
                $CurrentDate = [DateTime]::Now

                Write-Output $CurrentDate
                foreach ($item in $PSProcess)
                {
                    Write-Output -InputObject $item
                }
            }

            Write-Output ""Beginning Script""

            $WorkflowTitle = ""MyWorkflowTitle""

            WorkflowF1 -Title $WorkflowTitle
                    
            Write-Output ""Script Completed""
        ";

        private bool _showHelpMessage;

        /// <summary>
        /// Collection of all breakpoints on the runspace debugger.
        /// </summary>
        private Dictionary<int, Breakpoint> _breakPoints = new Dictionary<int, Breakpoint>();

        #endregion

        #region Public Methods

        /// <summary>
        /// Method to run the sample script and handle debugger events.
        /// </summary>
        public void Run()
        {
            Console.WriteLine("Starting PowerShell Debugger Sample");
            Console.WriteLine();

            // Create sample script file to debug.
            string fileName = "PowerShellSDKDebuggerSample.ps1";
            string filePath = System.IO.Path.Combine(Environment.CurrentDirectory, fileName);
            System.IO.File.WriteAllText(filePath, _script);

            using (Runspace runspace = RunspaceFactory.CreateRunspace())
            {
                // Open runspace and set debug mode to debug PowerShell scripts and 
                // Workflow scripts.  PowerShell script debugging is enabled by default,
                // Workflow debugging is opt-in.
                runspace.Open();
                runspace.Debugger.SetDebugMode(DebugModes.LocalScript);

                using (PowerShell powerShell = PowerShell.Create())
                {
                    powerShell.Runspace = runspace;

                    // Set breakpoint update event handler.  The breakpoint update event is
                    // raised whenever a break point is added, removed, enabled, or disabled.
                    // This event is generally used to display current breakpoint information.
                    runspace.Debugger.BreakpointUpdated += HandlerBreakpointUpdatedEvent;

                    // Set debugger stop event handler.  The debugger stop event is raised 
                    // whenever a breakpoint is hit, or for each script execution sequence point 
                    // when the debugger is in step mode.  The debugger remains stopped at the
                    // current execution location until the event handler returns.  When the
                    // event handler returns it should set the DebuggerStopEventArgs.ResumeAction
                    // to indicate how the debugger should proceed:
                    //  - Continue      Continue execution until next breakpoint is hit.
                    //  - StepInto      Step into function.
                    //  - StepOut       Step out of function.
                    //  - StepOver      Step over function.
                    //  - Stop          Stop debugging.
                    runspace.Debugger.DebuggerStop += HandleDebuggerStopEvent;

                    // Set initial breakpoint on line 10 of script.  This breakpoint
                    // will be in the script workflow function.
                    powerShell.AddCommand("Set-PSBreakpoint").AddParameter("Script", filePath).AddParameter("Line", 10);
                    powerShell.Invoke();
                
                    Console.WriteLine("Starting script file: " + filePath);
                    Console.WriteLine();

                    // Run script file.
                    powerShell.Commands.Clear();
                    powerShell.AddScript(filePath).AddCommand("Out-String").AddParameter("Stream", true);
                    var scriptOutput = new PSDataCollection<PSObject>();
                    scriptOutput.DataAdded += (sender, args) =>
                        {
                            // Stream script output to console.
                            foreach (var item in scriptOutput.ReadAll())
                            {
                                Console.WriteLine(item);
                            }
                        };
                    powerShell.Invoke<PSObject>(null, scriptOutput);
                }
            }

            // Delete the sample script file.
            if (System.IO.File.Exists(filePath))
            {
                System.IO.File.Delete(filePath);
            }

            Console.WriteLine("PowerShell Debugger Sample Complete");
            Console.WriteLine();
            Console.WriteLine("Press any key to exit.");
            Console.ReadKey(true);
        }

        #endregion

        #region Private Methods

        // Method to handle the Debugger DebuggerStop event.
        // The debugger will remain in debugger stop mode until this event
        // handler returns, at which time DebuggerStopEventArgs.ResumeAction should
        // be set to indicate how the debugger should proceed (Continue, StepInto,
        // StepOut, StepOver, Stop).
        // This handler should run a REPL (Read Evaluate Print Loop) to allow user
        // to investigate the state of script execution, by processing user commands
        // with the Debugger.ProcessCommand method.  If a user command releases the
        // debugger then the DebuggerStopEventArgs.ResumeAction is set and this 
        // handler returns.
        private void HandleDebuggerStopEvent(object sender, DebuggerStopEventArgs args)
        {
            Debugger debugger = sender as Debugger;
            DebuggerResumeAction? resumeAction = null;

            // Display messages pertaining to this debugger stop.
            WriteDebuggerStopMessages(args);

            // Simple REPL (Read Evaluate Print Loop) to process
            // Debugger commands.
            while (resumeAction == null)
            {
                // Write debug prompt.
                Console.Write("[DBG] PS >> ");
                string command = Console.ReadLine();
                Console.WriteLine();

                // Stream output from command processing to console.
                var output = new PSDataCollection<PSObject>();
                output.DataAdded += (dSender, dArgs) =>
                {
                    foreach (var item in output.ReadAll())
                    {
                        Console.WriteLine(item);
                    }
                };

                // Process command.
                // The Debugger.ProcesCommand method will parse and handle debugger specific
                // commands such as 'h' (help), 'list', 'stepover', etc.  If the command is 
                // not specific to the debugger then it will be evaluated as a PowerShell 
                // command or script.  The returned DebuggerCommandResults object will indicate
                // whether the command was evaluated by the debugger and if the debugger should
                // be released with a specific resume action.
                PSCommand psCommand = new PSCommand();
                psCommand.AddScript(command).AddCommand("Out-String").AddParameter("Stream", true);
                DebuggerCommandResults results = debugger.ProcessCommand(psCommand, output);
                if (results.ResumeAction != null)
                {
                    resumeAction = results.ResumeAction;
                }
            }

            // Return from event handler with user resume action.
            args.ResumeAction = resumeAction.Value;
        }

        // Method to handle the Debugger BreakpointUpdated event.
        // This method will display the current breakpoint change and maintain a 
        // collection of all current breakpoints.
        private void HandlerBreakpointUpdatedEvent(object sender, BreakpointUpdatedEventArgs args)
        {
            // Write message to console.
            ConsoleColor saveFGColor = Console.ForegroundColor;
            Console.ForegroundColor = ConsoleColor.Yellow;
            Console.WriteLine();

            switch (args.UpdateType)
            {
                case BreakpointUpdateType.Set:
                    if (!_breakPoints.ContainsKey(args.Breakpoint.Id))
                    {
                        _breakPoints.Add(args.Breakpoint.Id, args.Breakpoint);
                    }
                    Console.WriteLine("Breakpoint created:");
                    break;

                case BreakpointUpdateType.Removed:
                    _breakPoints.Remove(args.Breakpoint.Id);
                    Console.WriteLine("Breakpoint removed:");
                    break;

                case BreakpointUpdateType.Enabled:
                    Console.WriteLine("Breakpoint enabled:");
                    break;

                case BreakpointUpdateType.Disabled:
                    Console.WriteLine("Breakpoint disabled:");
                    break;
            }

            Console.WriteLine(args.Breakpoint.ToString());
            Console.WriteLine();
            Console.ForegroundColor = saveFGColor;
        }

        /// <summary>
        /// Helper method to write debugger stop messages.
        /// </summary>
        /// <param name="args">DebuggerStopEventArgs for current debugger stop</param>
        private void WriteDebuggerStopMessages(DebuggerStopEventArgs args)
        {
            // Write debugger stop information in yellow.
            ConsoleColor saveFGColor = Console.ForegroundColor;
            Console.ForegroundColor = ConsoleColor.Yellow;

            // Show help message only once.
            if (!_showHelpMessage)
            {
                Console.WriteLine("Entering debug mode. Type 'h' to get help.");
                Console.WriteLine();
                _showHelpMessage = true;
            }

            // Break point summary message.
            string breakPointMsg = String.Format(System.Globalization.CultureInfo.InvariantCulture,
                "Breakpoints: Enabled {0}, Disabled {1}",
                (_breakPoints.Values.Where<Breakpoint>((bp) => { return bp.Enabled; })).Count(),
                (_breakPoints.Values.Where<Breakpoint>((bp) => { return !bp.Enabled; })).Count());
            Console.WriteLine(breakPointMsg);
            Console.WriteLine();

            // Breakpoint stop information.  Writes all breakpoints that 
            // pertain to this debugger execution stop point.
            if (args.Breakpoints.Count > 0)
            {
                Console.WriteLine("Debugger hit breakpoint on:");
                foreach (var breakPoint in args.Breakpoints)
                {
                    Console.WriteLine(breakPoint.ToString());
                }
                Console.WriteLine();
            }

            // Script position stop information.
            // This writes the InvocationInfo position message if
            // there is one.
            if (args.InvocationInfo != null)
            {
                Console.WriteLine(args.InvocationInfo.PositionMessage);
                Console.WriteLine();
            }

            Console.ForegroundColor = saveFGColor;
        }

        #endregion
    }
}

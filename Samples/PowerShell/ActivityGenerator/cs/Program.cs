// <copyright file="Program.cs" company="Microsoft Corporation">
// Copyright (c) 2012 Microsoft Corporation. All rights reserved.
// </copyright>
// DISCLAIMER OF WARRANTY: The software is licensed “as-is.” You 
// bear the risk of using it. Microsoft gives no express warranties, 
// guarantees or conditions. You may have additional consumer rights 
// under your local laws which this agreement cannot change. To the extent 
// permitted under your local laws, Microsoft excludes the implied warranties 
// of merchantability, fitness for a particular purpose and non-infringement.

using System;
using System.Management.Automation;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Text;

namespace ActivityGeneratorSample
{
    /// <summary>
    /// Provides an example of generating Workflow activities for cmdlets not shipped
    /// with PowerShell by default.
    /// </summary>
    public static class ActivityGenerator
    {
        static void Main(string[] args)
        {
            string targetModule = "PSDiagnostics";

            if(args.Length == 1)
            {
                targetModule = args[0];
            }

            using (PowerShell targetPowerShell = PowerShell.Create())
            {
                // Set the execution policy for this session.
                targetPowerShell.AddCommand("Set-ExecutionPolicy").
                    AddParameter("ExecutionPolicy", "RemoteSigned").
                    AddParameter("Scope", "Process").Invoke();
                targetPowerShell.Commands.Clear();

                // Import the given module, using "PSDiagnostics" as an example if the user hasn't
                // specified one.
                targetPowerShell.AddCommand("Import-Module").AddParameter("Name", targetModule).Invoke();
                targetPowerShell.Commands.Clear();

                // Get the first command from the module. If we were to generate activities for
                // the entire module, we would loop over these results.
                Collection<CommandInfo> results = 
                    targetPowerShell.AddCommand("Get-Command").AddParameter("Module", targetModule).Invoke<CommandInfo>();
                CommandInfo targetCommand = results.FirstOrDefault<CommandInfo>();

                if (targetCommand != null)
                {
                    // Generate the source code.
                    string targetNamespace = String.Format(
                        System.Globalization.CultureInfo.InvariantCulture,
                        "Microsoft.PowerShell.Samples.{0}.Activities", targetModule);
                    string source = Microsoft.PowerShell.Activities.ActivityGenerator.GenerateFromCommandInfo(targetCommand, targetNamespace);

                    // Write it to the console.
                    Console.WriteLine(source);
                }
            }
        }
    }
}

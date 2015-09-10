// <copyright file="PowerShell02.cs" company="Microsoft Corporation">
// Copyright (c) 2009 Microsoft Corporation. All rights reserved.
// </copyright>
// DISCLAIMER OF WARRANTY: The software is licensed “as-is.” You 
// bear the risk of using it. Microsoft gives no express warranties, 
// guarantees or conditions. You may have additional consumer rights 
// under your local laws which this agreement cannot change. To the extent 
// permitted under your local laws, Microsoft excludes the implied warranties 
// of merchantability, fitness for a particular purpose and non-infringement.

namespace Sample
{
  using System;
  using System.Collections;
  using System.Collections.Generic;
  using System.Management.Automation;
  using System.Management.Automation.Runspaces;
  
  /// <summary>
  /// This class contains the Main entry point for the application.
  /// </summary>
  internal class PowerShell02
  {
    /// <summary>
    /// Runs many commands with the help of a RunspacePool.
    /// </summary>
    /// <param name="args">This parameter is unused.</param>
    private static void Main(string[] args)
    {
      // Creating and opening runspace pool. Use a minimum of 1 runspace and a maximum of 
      // 5 runspaces can be opened at the same time.
      RunspacePool runspacePool = RunspaceFactory.CreateRunspacePool(1, 5);
      runspacePool.Open();
      
      using (runspacePool)
      {
        // Define the commands to be run.
        List<PowerShell> powerShellCommands = new List<PowerShell>();
        
        // The command results.
        List<IAsyncResult> powerShellCommandResults = new List<IAsyncResult>();
        
        // The maximum number of runspaces that can be opened at one time is 
        // 5, but we can queue up many more commands that will use the 
        // runspace pool.
        for (int i = 0; i < 100; i++)
        {
          // Using a PowerShell object, run the commands.
          PowerShell powershell = PowerShell.Create();
                   
          // Instead of setting the Runspace property of powershell,
          // the RunspacePool property is used. That is the only difference
          // between running commands with a runspace and running commands 
          // with a runspace pool.
          powershell.RunspacePool = runspacePool;
          
          // The script to be run outputs a sequence number and the number of available runspaces 
          // in the pool.
          string script = String.Format(
                        "write-output ' Command: {0}, Available Runspaces: {1}'",
                        i,
                        runspacePool.GetAvailableRunspaces());

          // The three lines below look the same running with a runspace or 
          // with a runspace pool.
          powershell.AddScript(script);
          powerShellCommands.Add(powershell);
          powerShellCommandResults.Add(powershell.BeginInvoke());
        }
        
        // Collect the results.
        for (int i = 0; i < 100; i++)
        {
          // EndInvoke will wait for each command to finish, so we will be getting the commands
          // in the same 0 to 99 order that they have been invoked withy BeginInvoke.
          PSDataCollection<PSObject> results = powerShellCommands[i].EndInvoke(powerShellCommandResults[i]);
          
          // Print all the results. One PSObject with a plain string is the expected result.
          PowerShell02.PrintCollection(results);
        }
      }
    }
    
    /// <summary>
    /// Iterates through a collection printing all items.
    /// </summary>
    /// <param name="collection">collection to be printed</param>
    private static void PrintCollection(IList collection)
    {
      foreach (object obj in collection)
      {
        Console.WriteLine("PowerShell Result: {0}", obj);
      }
    }
  }
}

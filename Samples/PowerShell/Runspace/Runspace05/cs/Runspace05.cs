// <copyright file="Runspace05.cs" company="Microsoft Corporation">
// Copyright (c) 2009 Microsoft Corporation. All rights reserved.
// </copyright>
// DISCLAIMER OF WARRANTY: The software is licensed “as-is.” You 
// bear the risk of using it. Microsoft gives no express warranties, 
// guarantees or conditions. You may have additional consumer rights 
// under your local laws which this agreement cannot change. To the extent 
// permitted under your local laws, Microsoft excludes the implied warranties 
// of merchantability, fitness for a particular purpose and non-infringement.

namespace Microsoft.Samples.PowerShell.Runspaces
{
  using System;
  using System.Collections.ObjectModel;
  using System.Management.Automation;
  using System.Management.Automation.Runspaces;
  using PowerShell = System.Management.Automation.PowerShell;
    
  /// <summary>
  /// This class contains the Main entry point for this host application.
  /// </summary>
  internal class Runspace05
  {
    /// <summary>
    /// This sample uses an initial session state to create a runspace. The sample
    /// invokes a command from a PowerShell snap-in present in the console file.
    /// </summary>
    /// <param name="args">Parameter not used.</param>
    /// <remarks>
    /// This sample assumes that user has the GetProcessSample01.dll that is produced 
    /// by the GetProcessSample01 sample copied to the current directory. 
    /// This sample demonstrates the following:
    /// 1. Creating a default initial session state.
    /// 2. Creating a runspace using the default initial session state.
    /// 3. Creating a PowerShell object that uses the runspace.
    /// 4. Adding the get-proc cmdlet to the PowerShell object from a 
    ///    snap-in.
    /// 5. Using PSObject objects to extract and display properties from 
    ///    the objects returned by the cmdlet.
    /// </remarks>
    private static void Main(string[] args)
    {
      // Create the default initial session state. The default initial 
      // session state contains all the elements provided by Windows PowerShell.
      InitialSessionState iss = InitialSessionState.CreateDefault();
      PSSnapInException warning;
      iss.ImportPSSnapIn("GetProcPSSnapIn01", out warning);
           
      // Create a runspace. Notice that no PSHost object is supplied to the 
      // CreateRunspace method so the default host is used. See the Host 
      // samples for more information on creating your own custom host.
      using (Runspace myRunSpace = RunspaceFactory.CreateRunspace(iss))
      {
        myRunSpace.Open();
         
        // Create a PowerShell object. 
        using (PowerShell powershell = PowerShell.Create())
        {
          // Add the Cmdlet and specify the runspace.
          powershell.AddCommand("GetProcPSSnapIn01\\get-proc");
          powershell.Runspace = myRunSpace;
         
          // Run the cmdlet synchronously.
          Collection<PSObject> results = powershell.Invoke();
           
          Console.WriteLine("Process              HandleCount");
          Console.WriteLine("--------------------------------");
          
          // Display the results.
          foreach (PSObject result in results)
          {
            Console.WriteLine(
                              "{0,-20} {1}",
                              result.Members["ProcessName"].Value,
                              result.Members["HandleCount"].Value);
          }
        }
        
        // Close the runspace to release any resources.
        myRunSpace.Close();
      }
      System.Console.WriteLine("Hit any key to exit...");
      System.Console.ReadKey();
    }
  }
}


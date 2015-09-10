// <copyright file="Runspace11.cs" company="Microsoft Corporation">
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
  using System.Collections.Generic;
  using System.Diagnostics;
  using System.Management.Automation;
  using System.Management.Automation.Runspaces;
  using PowerShell = System.Management.Automation.PowerShell;
  
  #region GetProcCommand
  
  /// <summary>
  /// This class implements the get-proc cmdlet. It has been copied 
  /// verbatim from the GetProcessSample02.cs sample.
  /// </summary>
  [Cmdlet(VerbsCommon.Get, "Proc")]
  public class GetProcCommand : Cmdlet
  {
    #region Parameters
    
    /// <summary>
    /// The names of the processes to act on.
    /// </summary>
    private string[] processNames;
    
    /// <summary>
    /// Gets or sets the list of process names on which 
    /// the Get-Proc cmdlet will work.
    /// </summary>
    [Parameter(Position = 0)]
    [ValidateNotNullOrEmpty]
    public string[] Name
    {
      get { return this.processNames; }
      set { this.processNames = value; }
    }
    
    #endregion Parameters
    
    #region Cmdlet Overrides
    
    /// <summary>
    /// The ProcessRecord method calls the Process.GetProcesses 
    /// method to retrieve the processes specified by the Name 
    /// parameter. Then, the WriteObject method writes the 
    /// associated processes to the pipeline.
    /// </summary>
    protected override void ProcessRecord()
    {
      // If no process names are passed to the cmdlet, get all 
      // processes.
      if (this.processNames == null)
      {
        WriteObject(Process.GetProcesses(), true);
      }
      else
      {
        // If process names are passed to cmdlet, get and write 
        // the associated processes.
        foreach (string name in this.processNames)
        {
          WriteObject(Process.GetProcessesByName(name), true);
        }
      } // if (processNames...
    } // ProcessRecord
    
    #endregion Cmdlet Overrides
  } // GetProcCommand
  
  #endregion GetProcCommand
  
  /// <summary>
  /// This class contains the Main entry point for this host application.
  /// </summary>
  internal class Runspace11
  {
    /// <summary>
    /// This sample uses the ProxyCommand class to create a proxy command that 
    /// calls an existing cmdlet, but restricts the set of available parameters.  
    /// The proxy command is then added to an intial session state that is used to 
    /// create a contrained runspace. This means that the user can access the cmdlet 
    /// through the proxy command.
    /// </summary>
    /// <remarks>
    /// This sample demonstrates the following:
    /// 1. Creating a CommandMetadata object that describes the metadata of an 
    ///    existing cmdlet.
    /// 2. Modifying the cmdlet metadata to remove a parameter of the cmdlet.
    /// 3. Adding the cmdlet to an initial session state and making it private.
    /// 4. Creating a proxy function that calls the existing cmdlet, but exposes 
    ///    only a restricted set of parameters.   
    /// 6. Adding the proxy function to the initial session state. 
    /// 7. Calling the private cmdlet and the proxy function to demonstrate the 
    ///    constrained runspace.
    /// </remarks>
    private static void Main()
    {
      // Create a default intial session state. The default inital session state
      // includes all the elements that are provided by Windows PowerShell. 
      InitialSessionState iss = InitialSessionState.CreateDefault();
      
      // Add the get-proc cmdlet to the initial session state.
      SessionStateCmdletEntry cmdletEntry = new SessionStateCmdletEntry("get-proc", typeof(GetProcCommand), null);
      iss.Commands.Add(cmdletEntry);
      
      // Make the cmdlet private so that it is not accessable.
      cmdletEntry.Visibility = SessionStateEntryVisibility.Private;
      
      // Set the language mode of the intial session state to NoLanguge to 
      //prevent users from using language features. Only the invocation of 
      // public commands is allowed.
      iss.LanguageMode = PSLanguageMode.NoLanguage;
      
      // Create the proxy command using cmdlet metadata to expose the 
      // get-proc cmdlet.
      CommandMetadata cmdletMetadata = new CommandMetadata(typeof(GetProcCommand));
      
      // Remove one of the parameters from the command metadata.
      cmdletMetadata.Parameters.Remove("Name");
      
      // Generate the body of a proxy function that calls the original cmdlet, 
      // but does not have the removed parameter.
      string bodyOfProxyFunction = ProxyCommand.Create(cmdletMetadata);
      
      // Add the proxy function to the initial session state. The name of the proxy 
      // function can be the same as the name of the cmdlet, but to clearly 
      // demonstrate that the original cmdlet is not available a different name is 
      // used for the proxy function.
      iss.Commands.Add(new SessionStateFunctionEntry("get-procProxy", bodyOfProxyFunction));
      
      // Create the constrained runspace using the intial session state. 
      using (Runspace myRunspace = RunspaceFactory.CreateRunspace(iss))
      {
        myRunspace.Open();
        
        // Call the private cmdlet to demonstrate that it is not available.
        try
        {
          using (PowerShell powershell = PowerShell.Create())
          {
            powershell.Runspace = myRunspace;
            powershell.AddCommand("get-proc").AddParameter("Name", "*explore*");
            powershell.Invoke();
          }
        }
        catch (CommandNotFoundException e)
        {
          System.Console.WriteLine(
                        "Invoking 'get-proc' failed as expected: {0}: {1}", 
                        e.GetType().FullName, 
                        e.Message);
        }
        
        // Call the proxy function to demonstrate that the -Name parameter is 
        // not available.
        try
        {
          using (PowerShell powershell = PowerShell.Create())
          {
            powershell.Runspace = myRunspace;
            powershell.AddCommand("get-procProxy").AddParameter("Name", "idle");
            powershell.Invoke();
          }
        }
        catch (ParameterBindingException e)
        {
          System.Console.WriteLine(
                        "\nInvoking 'get-procProxy -Name idle' failed as expected: {0}: {1}", 
                        e.GetType().FullName, 
                        e.Message);
        }
        
        // Call the proxy function to demonstrate that it calls into the 
        // private cmdlet to retrieve the processes.
        using (PowerShell powershell = PowerShell.Create())
        {
          powershell.Runspace = myRunspace;
          powershell.AddCommand("get-procProxy");
          List<Process> processes = new List<Process>(powershell.Invoke<Process>());
          System.Console.WriteLine(
                        "\nInvoking the get-procProxy function called into the get-proc cmdlet and returned {0} processes", 
                        processes.Count);
        }
        
        // Close the runspace to release resources.
        myRunspace.Close();
      }
      
      System.Console.WriteLine("Hit any key to exit...");
      System.Console.ReadKey();
    }
  }
}


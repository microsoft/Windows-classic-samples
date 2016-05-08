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
using Win32Exception = System.ComponentModel.Win32Exception;
using System.Diagnostics;
using System.Security.Permissions;
using System.Management.Automation;    // Windows PowerShell namespace


// This sample is a complete implementation of the get-proc cmdlet.
//
// To test this cmdlet, create a module folder that has the same name 
// as this assembly, and then run the following command:
// import-module getprocesssample05

namespace Microsoft.Samples.PowerShell.Commands
{
    #region GetProcCommand

    /// <summary>
   /// This class implements the get-proc cmdlet.
   /// </summary>
   [Cmdlet(VerbsCommon.Get, "Proc", 
      DefaultParameterSetName="ProcessName")]
   public class GetProcCommand: PSCmdlet
   {
      #region Parameters

      /// <summary>
      /// This parameter provides the list of process names on 
      /// which the Get-Proc cmdlet will work.
      /// </summary>
      [Parameter(
         Position = 0,
         ParameterSetName = "ProcessName",
         ValueFromPipeline = true,
         ValueFromPipelineByPropertyName = true)]
      [ValidateNotNullOrEmpty]
      public string[] Name
      {
         get { return processNames; }
         set { processNames = value; }
      }

      /// <summary>
      /// This parameter provides the list of process identifiers on 
      /// which the Get-Proc cmdlet will work.
      /// </summary>
      [Parameter(
         ParameterSetName = "Id",
         Mandatory = true,
         ValueFromPipeline = true,
         ValueFromPipelineByPropertyName = true,
         HelpMessage = "The unique id of the process to get.")]
      public int[] Id
      {
         get { return processIds; }
         set { processIds = value; }
      }

      /// <summary>
      /// This parameter takes Process objects directly. If the input is a 
      /// stream of [collection of] Process objects, the cmdlet bypasses the 
      /// ProcessName and Id parameters and reads the Process objects 
      /// directly.  This allows the cmdlet to deal with processes that have 
      /// wildcard characters in their name.
      /// <value>Process objects</value>
      /// </summary>
      [Parameter(
         ParameterSetName = "InputObject",
         Mandatory = true,
         ValueFromPipeline = true)]
      public Process[] Input
      {
         get { return inputObjects; }
         set { inputObjects = value; }
      }

      #endregion Parameters

      #region Cmdlet Overrides

      /// <summary>
      /// The ProcessRecord method calls the Process.GetProcesses 
      /// method to retrieve the processes. Then, the WriteObject 
      /// method writes the associated processes to the pipeline.
      /// </summary>
      protected override void ProcessRecord()
      {
         List<Process> matchingProcesses;
         
         WriteDebug("Obtaining the list of matching process objects.");

         switch (ParameterSetName)
         {
            case "Id":
               matchingProcesses = GetMatchingProcessesById();
               break;
            case "ProcessName":
               matchingProcesses = GetMatchingProcessesByName();
               break;
            case "InputObject":
               matchingProcesses = GetProcessesByInput();
               break;
            default:
               ThrowTerminatingError(
                   new ErrorRecord(
                       new ArgumentException("Bad ParameterSetName"),
                       "UnableToAccessProcessList",
                       ErrorCategory.InvalidOperation,
                       null));
               return;
         } // switch (ParameterSetName)

         WriteDebug("Outputting the matching process objects.");

         matchingProcesses.Sort(ProcessComparison);

         foreach (Process process in matchingProcesses)
         {
            WriteObject(process);
         }
      } // ProcessRecord

      #endregion Overrides

      #region protected Methods and Data

      /// <summary>
      /// Retrieves the list of all processes matching the ProcessName
      /// parameter and generates a nonterminating error for each 
      /// specified process name which is not found even though the name
      /// contains no wildcards.
      /// </summary>
      /// <returns></returns>
      [EnvironmentPermissionAttribute(
         SecurityAction.LinkDemand, 
         Unrestricted = true)]
      private List<Process> GetMatchingProcessesByName()
      {
         new EnvironmentPermission(
            PermissionState.Unrestricted).Assert();
         
         List<Process> allProcesses = 
            new List<Process>(Process.GetProcesses());

         // The keys dictionary is used for rapid lookup of 
         // processes that are already in the matchingProcesses list.
         Dictionary<int, byte> keys = new Dictionary<int, byte>();

         List<Process> matchingProcesses = new List<Process>();

         if (null == processNames)
            matchingProcesses.AddRange(allProcesses);
         else
         {
            foreach (string pattern in processNames)
            {
               WriteVerbose("Finding matches for process name \""
                  + pattern + "\".");

               // WildCard serach on the available processes
               WildcardPattern wildcard =
                  new WildcardPattern(pattern, 
                     WildcardOptions.IgnoreCase);

               bool found = false;

               foreach (Process process in allProcesses)
               {
                  if (!keys.ContainsKey(process.Id))
                  {
                     string processName = SafeGetProcessName(process);

                     // Remove the process from the allProcesses list 
                     // so that it is not tested again.
                     if(processName.Length == 0)
                        allProcesses.Remove(process);

                     // Perform a wildcard search on this particular 
                     // process name and check whether it matches the 
                     // pattern specified.
                     if (!wildcard.IsMatch(processName))
                        continue;

                     WriteDebug("Found matching process id " 
                        + process.Id + ".");

                     // A match is found.
                     found = true;
                     // Store the process identifier so that the same process
                     // is not added twice.
                     keys.Add(process.Id, 0);
                     // Add the process to the processes list.
                     matchingProcesses.Add(process);
                  }
               } // foreach (Process...

               if (!found &&
                 !WildcardPattern.ContainsWildcardCharacters(pattern))
               {
                  WriteError(new ErrorRecord(
                     new ArgumentException("Cannot find process name "
                        + "\"" + pattern + "\"."),
                     "ProcessNameNotFound",
                     ErrorCategory.ObjectNotFound,
                     pattern));
               }
            } // foreach (string...
         } // if (null...
         
         return matchingProcesses;
      } // GetMatchingProcessesByName

      /// <summary>
      /// Returns the name of a process.  If an error occurs, a blank
      /// string is returned.
      /// </summary>
      /// <param name="process">The process whose name is 
      /// returned.</param>
      /// <returns>The name of the process.</returns>
      [EnvironmentPermissionAttribute(
         SecurityAction.LinkDemand, Unrestricted = true)]
      protected static string SafeGetProcessName(Process process)
      {
         new EnvironmentPermission(PermissionState.Unrestricted).Assert();
         string name = "";

         if (process != null)
         {
            try 
            { 
                return process.ProcessName; 
            }
            catch (Win32Exception) { }
            catch (InvalidOperationException) { }
         }

         return name;
     } // SafeGetProcessName

      #endregion Cmdlet Overrides

      #region Private Methods

      /// <summary>
      /// Function to sort by process name first, and then by 
      /// the process identifier.
      /// </summary>
      /// <param name="x">First process object.</param>
      /// <param name="y">Second process object.</param>
      /// <returns>
      /// Returns less than zero if x is less than y,
      /// greater than 0 if x is greater than y, and 0 if x == y.
      /// </returns>
      private static int ProcessComparison(Process x, Process y)
      {
         int diff = String.Compare(
            SafeGetProcessName(x),
            SafeGetProcessName(y),
            StringComparison.CurrentCultureIgnoreCase);

         if (0 != diff)
            return diff;

         return x.Id - y.Id;
      }

      /// <summary>
      /// Retrieves the list of all processes matching the Id
      /// parameter and generates a nonterminating error for 
      /// each specified process identofier which is not found.
      /// </summary>
      /// <returns>An array of processes that match the given identifier.
      /// </returns>
      [EnvironmentPermissionAttribute(
         SecurityAction.LinkDemand,
         Unrestricted = true)]
      private List<Process> GetMatchingProcessesById()
      {
         new EnvironmentPermission(
            PermissionState.Unrestricted).Assert();

         List<Process> matchingProcesses = new List<Process>();

         if (null != processIds)
         {
            // The keys dictionary is used for rapid lookup of the 
            // processes already in the matchingProcesses list.
            Dictionary<int, byte> keys = new Dictionary<int, byte>();

            foreach (int processId in processIds)
            {
               WriteVerbose("Finding match for process id "
                  + processId + ".");

               if (!keys.ContainsKey(processId))
               {
                  Process process;
                  try { process = Process.GetProcessById(processId); }
                  catch (ArgumentException ex)
                  {
                     WriteError(new ErrorRecord(
                        ex,
                        "ProcessIdNotFound",
                        ErrorCategory.ObjectNotFound,
                        processId));
                     continue;
                  }
                  
                  WriteDebug("Found matching process.");
                  
                  matchingProcesses.Add(process);
                  keys.Add(processId, 0);
               }
            }
         }
         
         return matchingProcesses;
      } // GetMatchingProcessesById

      /// <summary>
      /// Retrieves the list of all processes matching the InputObject
      /// parameter.
      /// </summary>
      [EnvironmentPermissionAttribute(
         SecurityAction.LinkDemand,
         Unrestricted = true)]
      private List<Process> GetProcessesByInput()
      {
         new EnvironmentPermission(
            PermissionState.Unrestricted).Assert();

         List<Process> matchingProcesses = new List<Process>();

         if (null != Input)
         {
            // The keys dictionary is used for rapid lookup of the
            // processes already in the matchingProcesses list.
            Dictionary<int, byte> keys = new Dictionary<int, byte>();

            foreach (Process process in Input)
            {
               WriteVerbose("Refreshing process object.");

               if (!keys.ContainsKey(process.Id))
               {
                  try { process.Refresh(); }
                  catch (Win32Exception) { }
                  catch (InvalidOperationException) { }

                  matchingProcesses.Add(process);
               }
            }
         }
         return matchingProcesses;
      } // GetProcessesByInput

      #endregion Private Methods

      #region Private Data

      private string[] processNames;
      private int[] processIds;
      private Process[] inputObjects;

      #endregion Private Data

    } //GetProcCommand

    #endregion GetProcCommand
}



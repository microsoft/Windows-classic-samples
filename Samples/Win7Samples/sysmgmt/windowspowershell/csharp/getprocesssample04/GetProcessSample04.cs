//
// Copyright (c) 2006 Microsoft Corporation. All rights reserved.
// 
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
// EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.
//
using System;
using System.Diagnostics;
using System.Management.Automation;            // Windows PowerShell namespace


// This sample shows how to implement error handling for nonterminating 
// errors in a cmdlet.
//
// To test this cmdlet, create a module folder that has the same name 
// as this assembly, and then run the following command:
// import-module getprocesssample04


namespace Microsoft.Samples.PowerShell.Commands
{
   #region GetProcCommand

   /// <summary>
   /// This class implements the get-proc cmdlet.
   /// </summary>
   [Cmdlet(VerbsCommon.Get, "Proc")]
   public class GetProcCommand: Cmdlet
   {
      #region Parameters

      /// <summary>
      /// This parameter provides the list of process names on 
      /// which the Get-Proc cmdlet will work.
      /// </summary>
      [Parameter(
         Position = 0,
         ValueFromPipeline = true,
         ValueFromPipelineByPropertyName = true
      )]
      [ValidateNotNullOrEmpty]
      public string[] Name
      {
         get { return processNames; }
         set { processNames = value; }
      }
      private string[] processNames;

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
          // If no process names are passed to cmdlet, get all 
          // processes.
          if (processNames == null)
          {
              WriteObject(Process.GetProcesses(), true);
          }
          else
          {
              // If process names are passed to the cmdlet, get and write 
              // the associated processes.
              // If a nonterminating error occurs while retrieving processes, 
              // call the WriteError method to send an error record to the 
              // error stream.
              foreach (string name in processNames)
              {
                  Process[] processes;

                  try
                  {
                      processes = Process.GetProcessesByName(name);
                  }
                  catch (InvalidOperationException ex)
                  {
                      WriteError(new ErrorRecord(
                         ex,
                         "UnableToAccessProcessByName",
                         ErrorCategory.InvalidOperation,
                         name));
                      continue;
                  }

                  WriteObject(processes, true);
              } // foreach (string name...
          } // else
      } // ProcessRecord

      #endregion Overrides

    } //GetProcCommand

   #endregion GetProcCommand
}


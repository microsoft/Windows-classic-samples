//
// Copyright (c) 2006 Microsoft Corporation. All rights reserved.
// 
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
// EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.
//
using System;
using System.Diagnostics;
using System.Management.Automation;           // Windows PowerShell namespace


// This sample shows how to declare a parameter that accepts input
// from the pipeline. In this sample the Name parameter can accept 
// an object from the pipeline or accept a value from a property 
// of an object that has the same name as the parameter.
//
// To test this cmdlet, create a module folder that has the same name 
// as this assembly, and then run the following command:
// import-module getprocesssample03


namespace Microsoft.Samples.PowerShell.Commands
{
    #region GetProcCommand

   /// <summary>
   /// This class implements the get-proc cmdlet.
   /// </summary>
   [Cmdlet(VerbsCommon.Get, "Proc")]
   public class GetProcCommand : Cmdlet
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
          // If no process names are passed to the cmdlet, get all 
          // processes.
          if (processNames == null)
          {
              WriteObject(Process.GetProcesses(), true);
          }
          else
          {
              // If process names are passed to the cmdlet, get and write 
              // the associated processes.
              foreach (string name in processNames)
              {
                  WriteObject(Process.GetProcessesByName(name), true);
              }
          } // if (processNames ...
      } // ProcessRecord

      #endregion Overrides

   } //GetProcCommand

    #endregion GetProcCommand
}


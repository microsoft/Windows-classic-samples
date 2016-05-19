//
// Copyright (c) 2006 Microsoft Corporation. All rights reserved.
// 
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
//

using namespace System;
using namespace System::Collections;
using namespace System::ComponentModel;
using namespace System::Diagnostics;
using namespace System::Globalization;

using namespace System::Management::Automation;

/// This sample demonstrates the following:
/// 1. Writing a Cmdlet to stop a process
/// 2. Using ShouldProcess and ShouldContinue to 
///    take user input before stopping a process
namespace Microsoft
{
    namespace Samples
    {
        namespace PowerShell
        {
            namespace Commands
            {
                /// <remarks>
                /// Class that implements the stop-proc cmdlet.
                /// </remarks>
                [Cmdlet(VerbsLifecycle::Stop, "Proc",
                    DefaultParameterSetName = "ProcessId",
                    SupportsShouldProcess = true)]
                public ref class StopProcCmdlet : public PSCmdlet 
                {
                private:
                    #pragma region Private Data
                    array<String^>^ processNames;		
                    bool force;
                    bool passThru;
                    array<int>^ processIds;
                    array<Process^>^ inputObject;

                    bool yesToAll, noToAll;

                    /// <summary>
                    /// Partial list of critical processes that should not be 
                    /// stopped.  Lower case is used for case insensitive matching.
                    /// </summary>
                    static ArrayList^ criticalProcessNames = gcnew ArrayList(
                        gcnew array<String^>(4) { "system", "winlogon", "spoolsv", "calc" }
                    );
                    #pragma endregion

                public:
                    #pragma region Parameters
                    /// <summary>
                    /// The list of process names on which this cmdlet will work.
                    /// </summary>
                    [Parameter(
                        Position = 0,
                        ParameterSetName = "ProcessName",
                        Mandatory = true,
                        ValueFromPipeline = true,
                        ValueFromPipelineByPropertyName = true,
                        HelpMessage = "The name of one or more processes to stop. Wildcards are permitted."
                        )]
                    [Alias("ProcessName")]
                    property array<String^>^ Name
                    {
                        array<String^>^ get() 
                        { 
                            return processNames; 
                        }

                        void set(array<String^>^ value) 
                        { 
                            processNames = value; 
                        }
                    }

                    /// <summary>
                    /// Overrides the ShouldContinue check to force stop operation.
                    /// This option should always be used with caution.
                    /// </summary>
                    [Parameter]
                    property SwitchParameter Force
                    {
                        SwitchParameter get()
                        { 
                            return force; 
                        }

                        void set(SwitchParameter value)
                        { 
                            force = value; 
                        }
                    }

                    /// <summary>
                    /// Common parameter to determine if the process should pass the
                    /// object down the pipeline after the process has been stopped.
                    /// </summary>
                    [Parameter(
                        HelpMessage = "If set the process(es) will be passed to the pipeline after stopped."
                        )]
                    property SwitchParameter PassThru
                    {
                        SwitchParameter get()
                        { 
                            return passThru; 
                        }

                        void set(SwitchParameter value)
                        { 
                            passThru = value; 
                        }
                    }

                    /// <summary>
                    /// The list of process IDs on which this cmdlet will work.
                    /// </summary>
                    [Parameter(
                        ParameterSetName = "ProcessId",
                        Mandatory = true,
                        ValueFromPipelineByPropertyName = true,
                        ValueFromPipeline = true
                        )]
                    [Alias("ProcessId")]
                    property array<int>^ Id
                    {
                        array<int>^ get()
                        { 
                            return processIds; 
                        }

                        void set(array<int>^ value)
                        { 
                            processIds = value; 
                        }
                    }

                    /// <summary>
                    /// An array of Process objects from the stream to stop.
                    /// </summary>
                    /// <value>Process objects</value>
                    [Parameter(
                        ParameterSetName = "InputObject",
                        Mandatory = true,
                        ValueFromPipeline = true)]
                    property array<Process^>^ InputObject
                    {
                        array<Process^>^ get()
                        { 
                            return inputObject; 
                        }

                        void set(array<Process^>^ value)
                        { 
                            inputObject = value; 
                        }
                    }
                    #pragma endregion 

                protected:
                    #pragma region CmdletOverrides
                    /// <summary>
                    /// For each of the requested processnames:
                    /// 1) check it's not a special process
                    /// 2) attempt to stop that process.
                    /// If no process requested, then nothing occurs.
                    /// </summary>
                    virtual void ProcessRecord() override
                    {
                        if (ParameterSetName->Equals("ProcessName"))
                        {
                            ProcessByName();
                        }
                        else if (ParameterSetName->Equals("ProcessId"))
                        {
                            ProcessById();
                        }
                        else if (ParameterSetName->Equals("InputObject"))
                        {
                            for each (Process^ process in inputObject)
                            {
                                SafeStopProcess(process);
                            }
                        }
                        else
                        {
                            throw gcnew ArgumentException("Bad ParameterSet Name");
                        }		  
                    } // ProcessRecord

                    #pragma endregion

                private:
                    #pragma region Helper Methods

                    /// <summary>
                    /// Stop processes based on their names (using the
                    /// ParameterSetName as ProcessName)
                    /// </summary>
                    void ProcessByName()
                    {
                        ArrayList^ allProcesses = nullptr;

                        // Get a list of all processes.
                        try
                        {
                            allProcesses = gcnew ArrayList(Process::GetProcesses());
                        }
                        catch (InvalidOperationException^ ioe)
                        {
                            ThrowTerminatingError(gcnew ErrorRecord(
                                ioe, "UnableToAccessProcessList",
                                ErrorCategory::InvalidOperation, nullptr));
                        }

                        // If a name parameter is passed to cmdlet, get 
                        // the associated process(es). 
                        // Write a non-terminating error for failure to
                        // retrieve a process
                        for each (String^ name in processNames)
                        {
                            // The allProcesses array list is passed as a reference because 
                            // any process whose name cannot be obtained will be removed
                            // from the list so that its not compared the next time.
                            ArrayList^ processes =
                                SafeGetProcessesByName(name, allProcesses);

                            // If no processes were found write a non-
                            // terminating error.
                            if (processes->Count == 0)
                            {
                                WriteError(gcnew ErrorRecord(
                                    gcnew Exception("Process not found."),
                                    "ProcessNotFound",
                                    ErrorCategory::ObjectNotFound,
                                    name));
                            } // if (processes...
                            // Otherwise terminate all processes in the list.
                            else
                            {
                                for each (Process^ process in processes)
                                {
                                    SafeStopProcess(process);
                                } // foreach (Process...
                            } // else
                        } // foreach (string...
                    }

                    /// <summary>
                    /// Returns all processes with matching names.
                    /// </summary>
                    /// <param name="processName">
                    /// The name of the process(es) to return
                    /// </param>
                    /// <param name="allProcesses">An array of all 
                    /// machine processes.</param>
                    /// <returns>An array of matching processes.</returns>
                    ArrayList^ SafeGetProcessesByName(String^ processName,
                        ArrayList^% allProcesses)
                    {
                        // Create and array to store the matching processes.
                        ArrayList^ matchingProcesses = gcnew ArrayList();

                        // Create the wildcard for pattern matching.
                        WildcardOptions options = WildcardOptions::IgnoreCase |
                            WildcardOptions::Compiled;
                        WildcardPattern^ wildcard = gcnew WildcardPattern(processName, options);           

                        // Walk all of the machine processes.
                        for each(Process^ process in allProcesses)
                        {
                            String^ processNameToMatch = nullptr;
                            try
                            {
                                processNameToMatch = process->ProcessName;
                            }
                            catch (Win32Exception^ e)
                            {
                                // Remove the process from the list so that it is not 
                                // checked again.
                                allProcesses->Remove(process);

                                String^ message = String::Format(CultureInfo::CurrentCulture, 
                                    "The process \"{0}\" could not be found",
                                    processName);
                                WriteVerbose(message);
                                WriteError(gcnew ErrorRecord(e, "ProcessNotFound",
                                    ErrorCategory::ObjectNotFound, processName));

                                continue;
                            }

                            if (!wildcard->IsMatch(processNameToMatch))
                            {
                                continue;
                            }

                            matchingProcesses->Add(process); 
                        } // foreach(Process...

                        return matchingProcesses;
                    } // SafeGetProcessesByName

                    /// <summary>
                    /// Stop processes based on their ids (using the
                    /// ParameterSetName as ProcessIds)
                    /// </summary>
                    void ProcessById()
                    {
                        for each (int processId in processIds)
                        {
                            Process^ process = nullptr;
                            try
                            {
                                process = Process::GetProcessById(processId);

                                // Write a debug message to the host which will be helpful
                                // in troubleshooting a problem. All debug messages
                                // will appear with the -Debug option
                                String^ message = String::Format(CultureInfo::CurrentCulture, 
                                    "Acquired process for pid : {0}",
                                    process->Id);
                                WriteDebug(message);
                            }
                            catch (ArgumentException^ ae)
                            {
                                String^ message = String::Format(CultureInfo::CurrentCulture, 
                                    "The process id {0} could not be found",
                                    processId);
                                WriteVerbose(message);
                                WriteError(gcnew ErrorRecord(ae, "ProcessIdNotFound",
                                    ErrorCategory::ObjectNotFound, processId));				   
                                continue;
                            }

                            SafeStopProcess(process);
                        } // foreach (int...
                    }

                    /// <summary>
                    /// Safely stops a named process.  Used as standalone function
                    /// to declutter ProcessRecord method.
                    /// </summary>
                    /// <param name="process">The process to stop.</param>
                    void SafeStopProcess(Process^ process)
                    {
                        String^ processName = nullptr;

                        try
                        {
                            processName = process->ProcessName;
                        }
                        catch (Win32Exception^ e)
                        {
                            WriteError(gcnew ErrorRecord(e, "ProcessNotFound",
                                ErrorCategory::OpenError, processName));

                            return;
                        }          

                        // Confirm the operation first.
                        // This is always false if WhatIf is set.
                        if (!ShouldProcess(String::Format(CultureInfo::CurrentCulture, 
                            "{0} ({1})", processName, process->Id)))
                        {
                            return;
                        }

                        // Make sure the user really wants to stop a critical
                        // process and possibly stop the machine.
                        bool criticalProcess = criticalProcessNames->Contains(
                            processName->ToLower(CultureInfo::CurrentCulture));

                        String^ message = nullptr;
                        if (criticalProcess && !force)
                        {
                            message = String::Format(CultureInfo::CurrentCulture, 
                                "The process \"{0}\" is a critical process and should not be stopped. Are you sure you wish to stop the process?",
                                processName);

                            // It is possible that ProcessRecord is called multiple 
                            // times when objects are recieved as inputs from a pipeline.
                            // So, to retain YesToAll and NoToAll input that the 
                            // user may enter across mutilple calls to this 
                            // function, they are stored as private members of the 
                            // Cmdlet.
                            if (!ShouldContinue(message, "Warning!", yesToAll, noToAll))
                            {
                                return;
                            }
                        } // if (criticalProcess...

                        // Display a warning information if stopping a critical 
                        // process
                        if (criticalProcess)
                        {
                            message = String::Format(CultureInfo::CurrentCulture,
                                "Stopping the critical process \"{0}\".",
                                processName);
                            WriteWarning(message);
                        } // if (criticalProcess...

                        try
                        {
                            // Stop the process.
                            process->Kill();
                        }
                        catch (Exception^ e)
                        {
                            if ((e->GetType() == Win32Exception::typeid) || (e->GetType() == SystemException::typeid) ||
                                (e->GetType() == InvalidOperationException::typeid))
                            {
                                // This process could not be stopped so write
                                // a non-terminating error.
                                WriteError(gcnew ErrorRecord(e, "CouldNotStopProcess",
                                    ErrorCategory::CloseError,
                                    process));

                                return;
                            } // if ((e is...
                            else throw;
                        } // catch 

                        // Write a user-level message to the pipeline. These are 
                        // intended to give the user detailed information on the 
                        // operations performed by the Cmdlet. These messages will
                        // appear with the -Verbose option.
                        message = String::Format(CultureInfo::CurrentCulture, 
                            "Stopped process \"{0}\", pid {1}.",
                            processName, process->Id);

                        WriteVerbose(message);

                        // If the -PassThru command line argument is
                        // specified, pass the terminated process on.
                        if (passThru)
                        {
                            // Write a debug message to the host which will be helpful
                            // in troubleshooting a problem. All debug messages
                            // will appear with the -Debug option
                            message = String::Format(CultureInfo::CurrentCulture, 
                                "Writing process \"{0}\" to pipeline",
                                processName);
                            WriteDebug(message);
                            WriteObject(process);
                        } // if (passThru..
                    }

                    #pragma endregion
                };
            }
        }
    }
}
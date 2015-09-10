// <copyright file="AssemblyInfo.cs" company="Microsoft Corporation">
// Copyright (c) 2012 Microsoft Corporation. All rights reserved.
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
using Microsoft.PowerShell.Activities;
using Microsoft.PowerShell.Workflow;
using System.Activities;
using System.Management.Automation;
using System.Collections.Concurrent;
using System.Threading;
using System.Management.Automation.Runspaces;

namespace ActivityControllerExtensibilitySample
{
    // This activity controller sample does not handle multiple concurrent requests from the same job, but it could be extended to do so.
    class SampleActivityController : PSResumableActivityHostController
    {
        // Pass the  PSWorkflowRuntime as the parameter.
        // This resumes execution of the job.
        public SampleActivityController(PSWorkflowRuntime runtime)
            : base(runtime)
        {
            _runtime = runtime;
        }
        private PSWorkflowRuntime _runtime;

        // This parameter notifies the workflow runtime about the usage of the streams.
        // If there is no need to maintain the live streams from the job then, this method should return true.
        // If the disconnected streams are supported then the activit can be performed on a different computer.
        // Once the job is completed the data is sent to the live streams.
        public override bool SupportDisconnectedPSStreams
        {
            get
            {
                return true;
            }
        }

        // This function executes the activity.
        // It is highly recommended to not to block the execution of this function while executing the activity.
        // All the information should be logged into the queue and the function should be returned.
        // In a separate thread or on a separate machine, read the data from the queue and execute the activity.
        // Once the activity action is completed the bookmark should be resumed.
        // Design the activity controller to hanlde multiple activities from one workflowflow.
        public override void StartResumablePSCommand(Guid jobInstanceId, Bookmark bookmark, PowerShell command, PowerShellStreams<PSObject, PSObject> streams, PSActivityEnvironment environment, PSActivity activityInstance)
        {
            ActivityActionData data = new ActivityActionData();
            data.jobInstanceId = jobInstanceId;
            data.bookmark = bookmark;
            data.command = command;
            data.streams = streams;
            data.environment = environment;

            // Add the request to the queue.
            ActivityActionsQueue.TryAdd(jobInstanceId, data);


            // Return the fucntion and allow the workfow do other work in parallel.
            // There should be a servicing thead which gets the data from the queue and perform the action.
            // To keep this sample simple, the worker thread calls the servicing function.
            ThreadPool.QueueUserWorkItem(ServiceRequests, jobInstanceId);
            
        }

        // This function is called to stop the execution of the job.
        public override void StopAllResumablePSCommands(Guid jobInstanceId)
        {
            this.StopExecution(jobInstanceId);
        }
        
        #region Private Variables and Function

        // Dictionary inside dictionary can be used for handling multiple parallel jobs.
        private ConcurrentDictionary<Guid, ActivityActionData> ActivityActionsQueue = new ConcurrentDictionary<Guid, ActivityActionData>();

        // this class will hold all the information about the activity action
        // there is no need to save the PSActivity instance, just get the relevant info and let go the PSActivity object.
        private class ActivityActionData
        {
            internal Guid jobInstanceId;
            internal Bookmark bookmark;
            internal PowerShell command;
            internal PowerShellStreams<PSObject, PSObject> streams;
            internal PSActivityEnvironment environment;
        }

        // the servicing thread will execute this function to perform the activity action.
        private void ServiceRequests(object state)
        {
            Guid jobInstanceId = (Guid)state;
            ActivityActionData actiondata = null;

            ActivityActionsQueue.TryRemove(jobInstanceId, out actiondata);

            if (actiondata != null)
            {
                // the work is getting perform here
                PerformWork(actiondata);
            }

        }

        // In this function the activity action is performed
        private void PerformWork(ActivityActionData data)
        {
            bool failed = false;
            Exception exception = null;

            try
            {
                // setting up the streams
                data.command.Streams.Debug = data.streams.DebugStream;
                data.command.Streams.Error = data.streams.ErrorStream;
                data.command.Streams.Progress = data.streams.ProgressStream;
                data.command.Streams.Verbose = data.streams.VerboseStream;
                data.command.Streams.Warning = data.streams.WarningStream;


                // Custom WinRM Workflow Endpoint details
                // run CustomWorkflowEndpointSetup.ps1 in Powershell console as administrator, if you have not done.
                //
                WSManConnectionInfo connectionInfo = new WSManConnectionInfo();
                connectionInfo.ShellUri = "http://schemas.microsoft.com/powershell/CustomeWorkflowEndpoint";                

                // Create runspace pool on custom workflow endpoint where command will be invoked
                using (RunspacePool r = RunspaceFactory.CreateRunspacePool(1, 1, connectionInfo))
                {
                    try
                    {
                        r.Open();
                        data.command.RunspacePool = r;

                        // now executing the powershell command.
                        data.command.Invoke(data.streams.InputStream, data.streams.OutputStream, new PSInvocationSettings());
                    }
                    finally
                    {
                        r.Close();
                        r.Dispose();
                    }                    
                }
            }
            catch (Exception e)
            {
                // since work is getting performed on background thread so there should not be any exception.
                failed = true;
                exception = e;
            }

            // Now since activity action has already been performed so now we need to resume the execution of the 
            // workflow. This will be done by 
            PSWorkflowJob job = _runtime.JobManager.GetJob(data.jobInstanceId);

            // Now resuming the job
            if (failed)
            {
                job.ResumeBookmark(data.bookmark, this.SupportDisconnectedPSStreams, data.streams, exception);
            }
            else
            {
                job.ResumeBookmark(data.bookmark, this.SupportDisconnectedPSStreams, data.streams);
            }
        }

        // stopping the execution of commands
        private void StopExecution(Guid jobInstanceId)
        {
            // can be impletement by holding the reference to
            // actiondata and the calling stop() on command object
        }

        #endregion Private Variables and Function

    }
}

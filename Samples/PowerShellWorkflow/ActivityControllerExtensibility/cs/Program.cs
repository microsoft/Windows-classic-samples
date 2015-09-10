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
using System.Linq;
using System.Activities;
using System.Activities.Statements;
using System.Management.Automation;
using Microsoft.PowerShell.Activities;
using Microsoft.PowerShell.Workflow;
using System.IO;
using System.Collections.Generic;
using System.Threading;

namespace ActivityControllerExtensibilitySample
{
    class Program
    {
        static void Main(string[] args)
        {
            // Set the path to the XAML workflow.
            string workflowFileName = "SampleWorkflow.xaml";

            // Read the XAML from the workflow into the variable
            string xaml = File.ReadAllText(workflowFileName);

            // Create a runtime to host the workflow, passing the custom configuration provider to the constructor.           
            PSWorkflowRuntime runtime = new PSWorkflowRuntime(new SampleConfigurationProvider());

            // Parameters for the workflow can be provided in this dictionary
            Dictionary<string, object>parameters = new Dictionary<string, object>();

            // Create the job. Because we are using the default file-based store, we need to provide the XAML worklfow definition.
            PSWorkflowJob job = runtime.JobManager.CreateJob(Guid.NewGuid(), xaml, "Get-CurrentProcess", "SampleWorkflow", parameters);

            // Subscribe to the state change event before starting the job.
            AutoResetEvent wfEvent = new AutoResetEvent(false);
            job.StateChanged += delegate(object sender, JobStateEventArgs e)
            {
                switch (e.JobStateInfo.State)
                {
                    case JobState.Failed:
                    case JobState.Completed:
                        {
                            wfEvent.Set();
                        }
                        break;
                }
            };

            // Start the job
            job.StartJob();

            // Wait for the state change event.
            wfEvent.WaitOne();

            if (job.JobStateInfo.State == JobState.Completed)
            {
                Console.WriteLine("The job has completed successfully.");
                Console.WriteLine("Total Commands found: " + job.PSWorkflowInstance.Streams.OutputStream.Count);
            }
            else
                Console.WriteLine("The job has Failed.");

            Console.WriteLine("Press <Enter> to continue...");
            Console.ReadLine();

        }
    }
}

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
using System.Activities;
using Microsoft.PowerShell.Activities;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Text;

namespace ContainerActivitySample
{
    /// <summary>
    /// The ConfirmStep activity takes the specified action, invokes it,
    /// logs communication to the LogPath file, and then waits for the
    /// file to be deleted. Once the file has been deleted, the activity
    /// is considered complete and proceeds.
    /// </summary>
    public class ConfirmStep : NativeActivity
    {
        /// <summary>
        /// The action to be invoked. If you provide a script block to this parameter in a PowerShell
        /// workflow, PowerShell automatically converts the contents of the script block into the
        /// corresponding activities in the same way that it converts scripts to workflows.
        /// </summary>
        public Activity Action { get; set; }

        // A comment to be added to the log file associated with this action
        public InArgument<String> Comment { get; set; }

        // The path that this activity should use for communication
        public InArgument<String> LogPath { get; set; }

        // A helper activity to let us suspend the workflow after activity execution
        // is complete.
        private Suspend suspendActivity = new Suspend();

        // Executes the logic of the activity
        protected override void Execute(NativeActivityContext context)
        {
            // Prepare the log message, and log it to a file
            string logPath = LogPath.Get(context);
            string comment = Comment.Get(context);

            string[] logMessage = new string[] {
                "[" + DateTime.Now + "]",
                "Please validate that activity '" + comment + "' has completed successfully.",
                "If it has, delete this file and resume the workflow.",
                ""
            };
            System.IO.File.AppendAllLines(logPath,  logMessage);

            // Schedule the user's desired activity, and then schedule the suspend activity.
            // When the suspend activity is complete, tell workflow to call our OnWorkflowResumed
            // method.
            context.ScheduleActivity(Action);
            context.ScheduleActivity(suspendActivity, OnWorkflowResumed);
        }

        // Called when the workflow is resumed after the activity is invoked, and the
        // workflow is resumed.
        public void OnWorkflowResumed(NativeActivityContext context, ActivityInstance instance)
        {
            // Check if the log file is still there. If it is, re-run this activity.
            // Otherwise, we're done.
            string logPath = LogPath.Get(context);
            if (System.IO.File.Exists(logPath))
            {
                Execute(context);
            }
        }

        // Tells workflow that this activity can suspend / idle the runtime.
        protected override bool CanInduceIdle
        {
            get { return true; }
        }

        /// <summary>
        /// Tells the workflow runtime that this activity also schedules the
        /// suspend activity (which it cannot automatically detect because it is not
        /// provided as a parameter).
        /// </summary>
        /// <param name="metadata">The metadata provided by the hosting application.</param>
        protected override void CacheMetadata(NativeActivityMetadata metadata)
        {
            base.CacheMetadata(metadata);
            metadata.AddImplementationChild(this.suspendActivity);
        }
    }
}
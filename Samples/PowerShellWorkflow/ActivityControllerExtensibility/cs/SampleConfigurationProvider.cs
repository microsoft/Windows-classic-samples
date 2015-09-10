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
using Microsoft.PowerShell.Workflow;

namespace ActivityControllerExtensibilitySample
{
    // This class defines the configuration of a complete PSWorkflowRuntime.
    // To extend an activity controller, you must override the CreatePSActivityHostController function
    class SampleConfigurationProvider : PSWorkflowConfigurationProvider
    {
        // This causes the runtime to get the custom activity controller instead of the default controller.
        public override Microsoft.PowerShell.Activities.PSActivityHostController CreatePSActivityHostController()
        {
            return new SampleActivityController(Runtime);
        }

        // Put the 'GetProcess' activity in the out-of-process activity list.
        // This causes the activity controller to be called. for in-process activity there is no need for activity controller.
        public override IEnumerable<string> OutOfProcessActivity
        {
            get
            {
                yield return "GetProcess";
            }
        }

    }
}

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
using System.Management.Automation;

namespace SimpleExtensibilitySample
{
    // This class defines the configuration of a complete PSWorkflowRuntime object.
    // If you need to override configuration parameters, override them in this class.
    class SampleConfigurationProvider : PSWorkflowConfigurationProvider
    {
        // Put the 'InlineScript' Activity in the out-of-process activity list to help ensure the reliability of the host.
        public override IEnumerable<string> OutOfProcessActivity
        {
            get
            {
                yield return "InlineScript";
            }
        }

        // Override AllowedActivity property to change the list of allowed activities.
        public override IEnumerable<string> AllowedActivity
        {
            get
            {
                yield return "InlineScript";
                yield return "GetProcess";
            }
        }

        // PowerShell supports multiple lanaguage modes for hosting the PowerShell workflow engine.
        // Override the LanguageMode property to ConstrainedLanguage for the Inlinescript and PowerShellValue<T> activities.
        public override PSLanguageMode? LanguageMode
        {
            get
            {
                return PSLanguageMode.ConstrainedLanguage;
            }
        }

    }
}

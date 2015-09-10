#
#  Copyright (c) 2012 Microsoft Corporation.  All rights reserved.
#  
# DISCLAIMER OF WARRANTY: The software is licensed “as-is.” You 
# bear the risk of using it. Microsoft gives no express warranties, 
# guarantees or conditions. You may have additional consumer rights 
# under your local laws which this agreement cannot change. To the extent 
# permitted under your local laws, Microsoft excludes the implied warranties 
# of merchantability, fitness for a particular purpose and non-infringement.
#

winrm quickconfig -quiet

restart-service winrm

# Create custom workflow session option.
$wfExecuteOption = New-PSWorkflowExecutionOption -MaxRunningWorkflows 35 

# Register CustomeWorkflowEndpoint
Register-PSSessionConfiguration -Name CustomeWorkflowEndpoint -SessionType Workflow -SessionTypeOption $wfExecuteOption -Force

# Test the CustomeWorkflowEndpoint by running a workflow.
$wfEP = New-PSSession -ConfigurationName CustomeWorkflowEndpoint
icm $wfEP {workflow TestWorkflow {get-process}}
icm $wfEP {TestWorkflow}
Remove-PSSession $wfEP



#
#  Copyright (c) 2013 Microsoft Corporation.  All rights reserved.
#  
# DISCLAIMER OF WARRANTY: The software is licensed “as-is.” You 
# bear the risk of using it. Microsoft gives no express warranties, 
# guarantees or conditions. You may have additional consumer rights 
# under your local laws which this agreement cannot change. To the extent 
# permitted under your local laws, Microsoft excludes the implied warranties 
# of merchantability, fitness for a particular purpose and non-infringement.
#

## In this workflow, we include "ContainerActivity.dll", which we've defined
## in our sample project. This DLL includes a "ConfirmStep" activity, which
## accepts a parameter called 'Action' of type Activity. When PowerShell
## Workflow sees that you are trying to provide a script block to this type
## of parameter, it converts the script block to the corresponding activities
## (much the same way that it does when converting PowerShell scripts to
## workflows.
workflow Invoke-ContainerActivity
{
    #requires -Assembly "bin\debug\ContainerActivity.dll"

    "Beginning Workflow Execution"

    ConfirmStep -Action {
        "Here is some PowerShell Workflow code that will be invoked,"
        "but you want it manually verified before the workflow resumes."
    } -Comment "Manually check that the action was successful" `
      -LogPath "c:\temp\Invoke-ContainerActivity-output.txt"

    "Completed Workflow Execution"
}

## Remove the log file if one already exists
if(Test-Path c:\temp\Invoke-ContainerActivity-output.txt)
{
    Remove-Item c:\temp\Invoke-ContainerActivity-output.txt
}

## Run the workflow. We see that it outputs
## "Beginning workflow execution", and we review the
## output of the log.
$j = Invoke-ContainerActivity -AsJob
Receive-Job $j -Wait

Get-Content c:\temp\Invoke-ContainerActivity-output.txt

## Imagine that the activity execution wasn't successful, so we
## want to execute it again. Resume the job and this will happen
## automatically. Once we resume it, we can receive its output
## and review the log file again.
$null = Resume-Job $j
Receive-Job $j -Wait

Get-Content c:\temp\Invoke-ContainerActivity-output.txt

## Now, imagine that we were satisfied with the execution.
## Remove the log file, resume the job, and the activity
## will complete. The last Receive-Job call now gets:
## "Completed Workflow Execution".
Remove-Item c:\temp\Invoke-ContainerActivity-output.txt
$null = Resume-Job $j
Receive-Job $j -Wait
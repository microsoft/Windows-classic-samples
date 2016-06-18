# THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
# ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
# THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
# PARTICULAR PURPOSE.
#
# Copyright (c) Microsoft Corporation. All rights reserved

Write-DiagProgress -activity "Changing power plan..."

[string]$PowercfgCmd = Join-Path $Env:windir "\System32\Powercfg.exe"

# Balanced power plan GUID (for setting power plan with Powercfg)
[string]$PlanGuid = get-diaginput -id "IT_ChoosePowerScheme"

& $PowercfgCmd -SETACTIVE $PlanGuid
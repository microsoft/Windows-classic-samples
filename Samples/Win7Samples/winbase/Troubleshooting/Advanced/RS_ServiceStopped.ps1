# THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
# ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
# THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
# PARTICULAR PURPOSE.
#
# Copyright (c) Microsoft Corporation. All rights reserved

Write-DiagProgress -activity "Starting themes service..."

Start-Service Themes

#Wait for the service to enter running state
[ServiceProcess.ServiceController]$Service = New-Object "ServiceProcess.ServiceController" "Themes"
[TimeSpan]$TimeOut = New-Object TimeSpan(0,0,0,5,0)
$Service.WaitForStatus([ServiceProcess.ServiceControllerStatus]::Running, $TimeOut)

# THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
# ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
# THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
# PARTICULAR PURPOSE.
#
# Copyright (c) Microsoft Corporation. All rights reserved

Write-DiagProgress -activity "Checking service status..."

[bool]$Detected = (Get-Service Themes).Status -ne "Running"

Update-DiagRootCause -id "RC_ServiceStopped" -Detected $Detected

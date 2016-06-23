# THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
# ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
# THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
# PARTICULAR PURPOSE.
#
# Copyright (c) Microsoft Corporation. All rights reserved

# Load required .NET Framework assembly
[void][Reflection.Assembly]::LoadWithPartialName("System.Windows.Forms")

Write-DiagProgress -activity "Checking power plan and power source..."

# Get power-related events from the past week for troubleshooting report
$PowerEvents = Get-WinEvent -FilterHashTable @{ProviderName="Microsoft-Windows-Kernel-Power"; `
                                            StartTime=[DateTime]::Today.AddDays(-7)}

$PowerEvents | ConvertTo-Xml | Update-DiagReport -id "PowerEvents" `
                                                 -name "Power Events" `
                                                 -description "Recent power-related events from the Event Log"
                                            

# Form absolute path to Powercfg tool using %windir% environment variable
[string]$PowercfgCmd = Join-Path $Env:windir "\System32\Powercfg.exe"

# Get AC power status 
#     "Online" means plugged into AC outlet
#     "Offline" means running on battery power
[string]$ACPowerStatus = [System.Windows.Forms.SystemInformation]::PowerStatus.PowerLineStatus

[bool]$Detected = ((& $PowercfgCmd "-GETACTIVESCHEME").contains("a1841308-3541-4fab-bc81-f71556f20b4a")) -and ($ACPowerStatus -eq "Offline")

Update-DiagRootCause -id "RC_PowerSaverBattery" -detected $Detected


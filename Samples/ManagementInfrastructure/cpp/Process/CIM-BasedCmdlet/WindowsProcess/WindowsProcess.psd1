<#

THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Copyright (c) Microsoft Corporation. All rights reserved

#>

@{
    ModuleVersion = '1.0.0.0'
    NestedModules = @('WindowsProcess.cdxml')
    TypesToProcess = @('WindowsProcess.types.ps1xml')
    FormatsToProcess = @('WindowsProcess.Format.ps1xml')
    FunctionsToExport = @(
        'Get-WindowsProcess',
        'Set-WindowsProcess',
        'Start-WindowsProcess',
        'Stop-WindowsProcess')
}
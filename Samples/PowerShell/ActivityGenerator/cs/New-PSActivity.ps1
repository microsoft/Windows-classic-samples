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

param
(
    [Parameter(Mandatory=$true)]
    [String]$Namespace,
    [Parameter(Mandatory=$true)]
    [String[]]$ModulePath,
    [Parameter(Mandatory=$true)]
    [String]$OutputPath,
    [String]$AssemblyName
)

Import-Module PSWorkflow

# Provides an example of generating Workflow activities for cmdlets not shipped
# with PowerShell by default.

# Validate provided module path.
if (!(Test-Path $ModulePath))
{
    throw (New-Object System.IO.DirectoryNotFoundException $ModulePath)
}

# If the Outputpath doesn't exist, create one.
if (!(Test-Path $OutputPath))
{
    $null = mkdir $OutputPath -Verbose
}

# Collect commands from modules in provided modulePath.
$moduleInfoList = @(Import-Module $ModulePath -Force -PassThru)

# Foreach moduleInfo, call generator API and create cs files
foreach($moduleInfo in $ModuleInfoList)
{
    $cmdName = @()
    $cmdIndex = 0
    $codeCollection = @()

    $codeCollection = [Microsoft.PowerShell.Activities.ActivityGenerator]::GenerateFromModuleInfo($moduleInfo, $namespace)
    $cmdName = [string[]] ($moduleInfo.ExportedCommands.Keys)

    # If API returns bigger collection than command imported by module, there must be GeneratedTypes.
    if($codeCollection.count -gt $cmdName.count)
    {
        1..($codeCollection.count - $cmdName.count) | % {$cmdName += "GeneratedTypes" + $_}
    }

    foreach($code in $codeCollection)
    {
        write-verbose "Generating activity code for command $($cmdName[$cmdIndex])" -verbose
        $fileName = $cmdName[$cmdIndex] + ".cs"
        $filePath = (join-path $OutputPath $fileName)
        Set-Content -Path $filePath -Value $codeCollection[$cmdIndex]
        $cmdIndex++;
    }
}

# Compile all cs files into a single dll
if($AssemblyName)
{
    if ($AssemblyName -notlike "*.dll") 
    {
        $AssemblyName += ".dll"
    }
    Write-Verbose "Compiling .cs file in $outputpath folder into $assemblyname" -verbose
    Set-Alias -Name csc -Value "$($env:SystemRoot)\Microsoft.NET\Framework\v4.0.30319\csc.exe"
    pushd $OutputPath

    csc /nologo /out:$AssemblyName /target:library /recurse:*.cs `
    /reference:$([System.Activities.Activity].Assembly.Location) `
    /reference:$([PSObject].Assembly.Location) `
    /reference:$([Microsoft.Management.Infrastructure.CimSession].Assembly.Location) `
    /reference:$([Reflection.Assembly]::LoadWithPartialName("Microsoft.PowerShell.Activities").Location) `
    /reference:$([Reflection.Assembly]::LoadWithPartialName("Microsoft.PowerShell.Workflow.ServiceCore").Location)

    dir 
    popd
}

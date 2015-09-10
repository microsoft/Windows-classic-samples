$scriptPath = $(Split-Path $MyInvocation.MyCommand.Path)
$iisAppName = "WinHttpWebSocketSample"
$iisAppPath = "$env:systemdrive\inetpub\wwwroot\WinHttpWebSocketSample"
$websitePath = "$scriptPath\website"
$firewallRuleName = "WinHttpWebSocketSample - HTTP 80"
$requiredFeatures = "IIS-WebServer", "IIS-WebServerRole", "NetFx4Extended-ASPNET45", "IIS-ApplicationDevelopment", "IIS-ASPNET45", "IIS-ISAPIExtensions", "IIS-ISAPIFilter", "IIS-NetFxExtensibility45", "IIS-WebSockets"
$settingsFile = "$scriptPath\WinHttpWebSocketSampleScriptSettings"

# Check if running as Administrator.
$windowsIdentity = [System.Security.Principal.WindowsIdentity]::GetCurrent()
$principal = New-Object System.Security.Principal.WindowsPrincipal($windowsIdentity)
$administratorRole = [System.Security.Principal.WindowsBuiltInRole]::Administrator
if ($principal.IsInRole($administratorRole) -eq $false)
{
    "Please run the script from elevated PowerShell."
    return
}

# Check if the config file was created. If yes, user should first run RemoveServer.ps1.
if (Test-Path $settingsFile)
{
    "The script has already been run. Please run RemoveServer.ps1 before running it again."
    return;
}

# Get features that should be enabled.
$featuresToEnable = @()
Get-WindowsOptionalFeature -Online | ?{ $_.State -eq [Microsoft.Dism.Commands.FeatureState]::Disabled -and $requiredFeatures -contains $_.FeatureName } | %{ $featuresToEnable += $_.FeatureName }

# Save enabled features to the config file.
$featuresToEnable -join "," > $settingsFile

# Enable features.
if ($featuresToEnable.Count -gt 0)
{
    "Enabling following features: $($featuresToEnable -join ", ")."
    Enable-WindowsOptionalFeature -Online -FeatureName $featuresToEnable > $null
}

# Copy the webpage files
if (-not (Test-Path $iisAppPath))
{
    mkdir $iisAppPath > $null
    Copy-Item $websitePath\* $iisAppPath -r
}

# Add web application.
if ($(Get-WebApplication $iisAppName) -eq $null)
{
    "Adding a `'$iisAppName`' web application at $iisAppPath."
    New-WebApplication -Site "Default Web Site" -Name $iisAppName -PhysicalPath $iisAppPath > $null
    Add-WebConfigurationProperty -pspath "MACHINE/WEBROOT/APPHOST/Default Web Site/$iisAppName" -filter "appSettings" -name "." -value @{key='aspnet:UseTaskFriendlySynchronizationContext';value='true'}
}
else
{
    "Web application `'$iisAppName`' already exists."
}

# Add firewall rule.
"Adding firewall rule `'$firewallRuleName`'."
New-NetFirewallRule -DisplayName $firewallRuleName -Direction Inbound -Protocol TCP -LocalPort 80 -Action Allow > $null

###############################################################################
# Configures Management OData web service endpoint using the Role based plugins
# sample
###############################################################################

# Test for presence of Microsoft.Samples.Management.OData.RoleBasedPlugins.dll
$assemblyName = ".\Microsoft.Samples.Management.OData.RoleBasedPlugins.dll"

$customPluginAssembly = $assemblyName
if (!(Test-Path $customPluginAssembly))
{
    $customPluginAssembly = "..\bin\Debug\$assemblyName"

    if (!(Test-Path $customPluginAssembly))
    {
        $customPluginAssembly = "..\bin\Release\$assemblyName"

        if (!(Test-Path $customPluginAssembly))
        {
            throw "ERROR: Custom plugin assembly $assemblyName not found. Please either put it in the current folder or build the sample (so that it can be picked from bin folder)";
        }
    }
}

# RBAC account provisioning message
Write-Host "RBAC authorizes only provisioned user accounts."
Write-Host "Default RBAC configuration shipped along with samples has been provisioned for two local user accounts: localNonAdmin and localAdmin."
Write-Host "Please create either of the two user accounts on your machine and access web service from either of them."
Write-Host "If you like to use your current account with the web service, please provision it in RBAC. You need to add your user name (and domain, if there is any) in the users section of RbacConfiguration.xml"

# Installing Management OData optional component
Write-Host "Installing Management OData Service..."
.\installModata.ps1

# Setting up web service endpoint
Write-Host "Setting up web service endpoint..."
.\SetupIISConfig.ps1 -site MODataSvc -path $env:HOMEDRIVE\inetpub\wwwroot\Modata -cfgfile .\Web.config -port 7000 -app MODataSvc -svc .\Microsoft.Management.Odata.svc -schema .\Schema.mof -dispatchXml .\Schema.xml -rbac .\RbacConfiguration.xml -customPluginAssembly $customPluginAssembly

Write-Host "Web Service endpoint is setup. The source root URI is http://localhost:7000/MODataSvc/Microsoft.Management.Odata.svc"
###############################################################################
# Installs Management OData Service optional component
###############################################################################

&"$Env:SystemRoot\System32\Dism.exe" /online /enable-feature /FeatureName:ManagementOdata /all /Quiet /NoRestart

&"$Env:SystemRoot\System32\Dism.exe" /online /enable-feature /featurename:WCF-HTTP-Activation45 /all /Quiet /NoRestart

&"$Env:SystemRoot\System32\Dism.exe" /online /enable-feature /featurename:IIS-BasicAuthentication /featurename:IIS-WindowsAuthentication /all /Quiet /NoRestart

&"$Env:SystemRoot\System32\Dism.exe" /online /enable-feature /featurename:IIS-WebServerManagementTools /featurename:IIS-ManagementConsole /featurename:IIS-ManagementScriptingTools /featurename:IIS-ManagementService /featurename:IIS-IIS6ManagementCompatibility /featurename:IIS-Metabase /featurename:IIS-WMICompatibility /featurename:IIS-LegacyScripts /featurename:IIS-LegacySnapIn /all /Quiet /NoRestart

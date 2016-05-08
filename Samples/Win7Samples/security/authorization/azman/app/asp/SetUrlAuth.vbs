'
'  Configuring UrlAuth with a Vdir. 
'  This script uses ADSI to configre the a vdir in the IIS 6.0 metabase. to use
'  IIS 6.0 Url Authorization. 
'
'  This script assumes that the IIS vdir corresponding to the applicaiton has 
'  already been created.
'
'  to configure urlAuth in IIS go the the IIS applicaiton properties for this
'  app, press the "configuration" button which creates an IIS app and set the
'  urlauth.dll ISAPI filter as an "extension for all mappings"

'  After running this script to verify the settings use the adsutil tool like so
'
'  inetpub\adminscripts\adsutil enum w3svc/1/root/MyApp
'  (where MyApp is the app name)

'  Use the Authorization Manager mmc snap-in to do the following:
'     Create an application called "IIS 6.0 URL Authorization"
'     Create an operation called "AccessURL" with an operation ID of 1
'     Create a scope corresponding to the AzScopeName specified to this sciprt.
'       This scope name will be used to manage accesss to urls in this Vdir
'     Make sure that the IIS worker process has read access to the
'       AzMan policy store. You can do this using the AzMan UI (right click
'       store and select options and then the security tab


Set objArgs = WScript.Arguments

If objArgs.count < 4 then
  wscript.echo "Usage: SetUrlAuth VDirPath AzScopeName AzStoreName AzEnable [post rc1: ImpersonationLevel]"
  wscript.echo "Example: SetUrlAuth w3svc/1/root/MyApp MyApp msxml://d:\inetpub\wwwroot\AzStore.xml True"
  wscript.echo "Run with 'cscript' command in cmd.exe to avoid msg boxes"
Else

wscript.echo objargs(0)


'
' Get Interface to the Vdir object
'
DIM iis
set iis = GetObject("IIS://localhost/" & objArgs(0))


' Set scope name - this scope will preside over urls refering to this vdir
' This scope must exist in the corresponding Authorization Manager Store.
'
iis.AzScopeName = objArgs(1)


'
' Specify location of Authorization manager policy store
'
iis.AzStoreName = objArgs(2)


'
' Enable urlauth for this Vdir
'
iis.AzEnable = objArgs(3)


'
' Set Impersonation Level (we'll use worker process)
'
If objArgs.count > 4 then
   iis.AzImpersonationLevel = objArgs(4)
End if


'
' Write settings to Metabase
'
iis.SetInfo

End if

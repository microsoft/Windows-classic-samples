'
'  Enabling or disabling BizRules for an application
'  This script uses Authorization Manager Administrative interfaces to enable or disable
'  BizRules for a specified AzMan application in a specified AzMan policy store

On Error Resume Next

Set objArgs = WScript.Arguments

If objArgs.count <> 3 then
  wscript.echo "Usage: SetBizRule ""AzManStoreURL"" ""AzApplicaitonName"" True/False"
  wscript.echo "Example: SetBizRule ""msxml://d:\inetpub\wwwroot\AzStore.xml"" ""MyApp"" True"
  wscript.echo "Run with 'cscript' command in cmd.exe to avoid msg boxes"
  WScript.Quit
Else


  ' VBScript source code
  Dim AzStoreObj
  Dim AzManStoreURL : AzManStoreURL = objArgs(0)
  Dim AzManAppName : AzManAppName = objArgs(1)
  Dim BizRulesEnabled : BizRulesEnabled = objArgs(2)


  ' create azman object
  Set AzStoreObj = CreateObject("AzRoles.AzAuthorizationStore")

  If Err.Number > 0 Then
    WScript.Echo "Can not create AzRoles.AzAuthorizationStore. Check AzMan installation"
    WScript.Quit
  End If

  ' initialize store for Administration
  ' assumes store exists - if store is being created (e.g. an installing applicaion)
  ' use the value 3 instead of 2 in the call to IAzAuthorizationStore::initialize

  Err.Clear

  AzStoreObj.Initialize 2, AzManStoreURL

  If Err.Number <> 0 Then
    WScript.Echo "AzRoles.AzAuthorizationStore failed to initialize. Check store URL"
    WScript.Quit
  End If

  ' open applicaion
  set AzApp = AzStoreObj.OpenApplication(AzManAppName)
  If Err.Number <> 0 Then
    WScript.Echo "AzRoles.AzAuthorizationStore failed to open application: " + AzManAppName + ". Check application Name."
    WScript.Quit
  End If

  ' set BizRulesEnabled property
  WSCript.Echo "App BizRule Before:" & AzApp.BizRulesEnabled
  AzApp.BizRulesEnabled = BizRulesEnabled
  WSCript.Echo "App BizRule After:" & AzApp.BizRulesEnabled

  If Err.Number = 0 Then
    WScript.Echo "BizRulesEnabled is updated successfully ."
  Else
    WScript.Echo "BizRulesEnabled is NOT updated successfully."
  End If

End if
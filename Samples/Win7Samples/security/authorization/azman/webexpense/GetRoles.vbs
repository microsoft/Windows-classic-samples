' THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
' ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
' THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
' PARTICULAR PURPOSE.
'
'  Copyright (c) Microsoft Corporation. All rights reserved
'
'
' Script to query and dump a users roles in a specified Authorization Manager
' store across all applications in the store
'

Option Explicit
Dim objArgs
Set objArgs = WScript.Arguments
If objArgs.count <> 2 then
  wscript.echo "Usage: GetRoles <AzManStoreURL> <UserName>"
  wscript.echo "Example: SetBizRule msxml://c:\AzStore.xml nwtraders\JohnDoe"
  wscript.echo "Run with 'cscript' command in cmd.exe to avoid msg boxes"
Else
Dim AzManStoreURL : AzManStoreURL = objArgs(0)
Dim UserName: UserName = objArgs(1)
End If

'
'--- Initilaize the Authorization Manager store object
'
Dim pAzManStore
Set pAzManStore = CreateObject("AzRoles.AzAuthorizationStore")
pAzManStore.Initialize 0, AzManStoreURL
pAzManStore.Submit

'
'--- Dump a users roles
'
Dim Apps,App
Dim ClientContext
Dim ClientScopes,Scope
Dim CurrentScopesPage
Dim ClientRoles, Role
Dim MoreScopes


'
'--- For each app create a client context and enumerate roles across scopes
'
Set apps = pAzManStore.Applications

wscript.echo ("Roles for " & UserName)
for each app in apps

   Set ClientContext = app.InitializeClientContextFromName(UserName) 
   wscript.echo (vbnewline & "Application: " & app.name)
   
   Set CurrentScopesPage = nothing
   Set ClientScopes = nothing
   MoreScopes = True

   do while MoreScopes = True        
      ClientScopes = ClientContext.GetAssignedScopesPage(0,9,CurrentScopesPage)

      for each scope in ClientScopes
         If scope = "" then
            wscript.echo ("  Applicaiton Level Roles:")
         Else
            wscript.echo ("    Scope '" & scope & "' Roles:")
         End if

         ClientRoles = ClientContext.GetRoles (scope)
         for each role in ClientRoles
            wscript.echo ("      " & role)
         next
      next

      if UBound(ClientScopes) = -1 then
         MoreScopes = FALSE
      End If
   loop
next

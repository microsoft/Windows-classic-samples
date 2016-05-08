' Windows Installer utility to list registered products/patches and 
' product/patch info
' For use with Windows Scripting Host, CScript.exe or WScript.exe
' Copyright (c) Microsoft Corporation. All rights reserved.
' Demonstrates the use of the PatchesEx method and underlying APIs
'
Option Explicit

' Install Contexts
Const msiInstallContextFirstVisible    = 0
Const msiInstallContextUserManaged     = 1
Const msiInstallContextUser            = 2
Const msiInstallContextMachine         = 4
Const msiInstallContextAllUserManaged  = 8

' Installation Source Types
Const msiInstallSourceTypeNetwork = 1
Const msiInstallSourceTypeURL = 2
Const msiInstallSourceTypeMedia = 4

' Patch States    
Const msiFilterApplied = 1
Const msiFilterSuperseded = 2
Const msiFilterObsoleted = 4
Const msiFilterRegistered = 8

' Connect to Windows Installer object
On Error Resume Next
Dim installer : Set installer = Nothing
Set installer = Wscript.CreateObject("WindowsInstaller.Installer") : CheckError

' Check for ?, and show help message if found
Dim productName:productName = Wscript.Arguments(0)
If InStr(1, productName, "?", vbTextCompare) > 0 Then Usage

Dim argCount:argCount = Wscript.Arguments.Count
Dim productCode, userSID, contextOptions, context
Dim value, property
Dim message
Dim products, product
Dim productSources, productSource
Dim mediaDisks, mediaDisk
Dim mediaPackagePath, diskPrompt, lastUsedSource, lastUsedType, packageName
Dim tabbedewLine

' Check if we got any arguments
If (argCount > 0) Then
   productName = Wscript.Arguments(0)
   ' If Product name supplied, need to search for product code
   If Left(productName, 1) = "{" And Right(productName, 1) = "}" Then
      If installer.ProductState(productName) <> msiInstallStateUnknown Then productCode = UCase(productName)
   Else
      For Each productCode In installer.Products : CheckError
         If LCase(installer.ProductInfo(productCode, "ProductName")) = LCase(productName) Then Exit For
      Next
   End If
   If IsEmpty(productCode) Then Wscript.Echo "Product is not registered: " & productName : Wscript.Quit 2
End If

' See if the caller is interested in any specific user context
If (argCount > 1) Then
   context = 0
   contextOptions = Wscript.Arguments(1)
   If InStr(1, contextOptions, "m", vbTextCompare) > 0 Then context = context + msiInstallContextUserManaged
   If InStr(1, contextOptions, "u", vbTextCompare) > 0 Then context = context + msiInstallContextUser
   If InStr(1, contextOptions, "a", vbTextCompare) > 0 Then context = context + msiInstallContextMachine
Else
   context = msiInstallContextUserManaged + msiInstallContextUser + msiInstallContextMachine
End If

' Check if the user gave us any SID or wanted us to work with an empty SID.
' Per-Machine app enumeration requires an empty SID. If we are looking at 
' per-user apps, an empty SID indicates that we want to work with apps that 
' are visible to the current user only.
If (argCount > 2) Then
   userSID = Wscript.Arguments(2)

   ' Per-Machine product enumeration shouldn't specify a SID. So, display Usage.
   If InStr(1, contextOptions, "a", vbTextCompare) > 0 Then Usage
End If

' Based on the command-line parameters passed to us make an appropriate 
' call to installer.ProductsEx and get the collection of products
If productCode = Empty then
   Set products = installer.ProductsEx(vbNullString, vbNullString, context)
Else If userSID = Empty then
   Set products = installer.ProductsEx(productCode, vbNullString, context)
Else   
   Set products = installer.ProductsEx(productCode, userSID, context)
   End If
End If

If Err <> 0 Then Err.Clear

' Enumerate through the collection and display information for each of the products
For Each product In products : checkError

   'Get the product code
   message = vbNewLine & "---Product: " & product.ProductCode & " ---"
   message = message & vbNewLine & "User SID: " & product.UserSid
   message = message & vbNewLine & "Context: " & MapContext(product.Context)
   
   'Get the product properties
   message = message & vbNewLine & "Product Properties"
   For Each property In Array(_
			"Language",_
			"ProductName",_
			"PackageCode",_
			"Transforms",_
			"AssignmentType",_
			"PackageName",_
			"InstalledProductName",_
			"VersionString",_
			"RegCompany",_
			"RegOwner",_
			"ProductID",_
			"ProductIcon",_
			"InstallLocation",_
			"InstallSource",_
			"InstallDate",_
			"Publisher",_
			"LocalPackage",_
			"HelpLink",_
			"HelpTelephone",_
			"URLInfoAbout",_
			"URLUpdateInfo") : CheckError
		value = product.InstallProperty(property)
		If Err <> 0 Then Err.Clear : value = Empty
      If value <> Empty Then message = message & vbNewLine & vbTab & property & " = " & value
	Next
   
   ' Get all the source list information
   tabbedewLine = vbNewLine
   ListSources product
   
   ' Get information about all the patches
   tabbedewLine = vbNewLine & vbTab
   message = message & vbNewLine & "Patch Information"
   ListPatches product.ProductCode, userSID, context
   
   Wscript.Echo message
   
Next

Sub ListPatches(productCode, userSID, context)
   
   Dim patches, patch
   Dim patchSources, patchSource

   ' Get the collection of patches
   If userSID = Empty then
      Set patches = installer.PatchesEx(productCode, vbNullString, context, msiFilterApplied)
   Else
      Set patches = installer.PatchesEx(productCode, userSID, context, msiFilterApplied)
   End If    

   ' Enumerate through the collection and display information for each of the patches
   For Each patch In patches : CheckError
      ' Get the Patch Code
      message = message & tabbedewLine & "---Patch: " & patch.PatchCode & " ---"

      'Get the patch properties
       message = message & tabbedewLine & "Patch Properties"
       For Each property In Array(_
          "LocalPackage",_
          "Transforms",_
          "InstallDate",_
          "Uninstallable",_
          "State")
          value = patch.PatchProperty(property)
          If Err <> 0 Then Err.Clear : value = Empty
          If value <> Empty Then message = message & tabbedewLine & vbTab & property & " = " & value
       Next
       
       ' Get all the source list information
       ListSources patch
       
   Next
End Sub

Sub ListSources(patchOrProduct)

   Dim patchOrProductSources, patchOrProductSource
   
   message = message & tabbedewLine & "Source List Information"

    ' Get the network sources
    message = message & tabbedewLine & vbTab & "Network Sources"
    Set patchOrProductSources = patchOrProduct.sources(msiInstallSourceTypeNetwork) : CheckError
    For Each patchOrProductSource In patchOrProductSources
       message = message & tabbedewLine & vbTab & vbTab & patchOrProductSource
    Next

   ' Get the URL sources
    message = message & tabbedewLine & vbTab & "URL Sources"
    Set patchOrProductSources = patchOrProduct.sources(msiInstallSourceTypeURL)  : CheckError
    For Each patchOrProductSource In patchOrProductSources
       message = message & tabbedewLine & vbTab & vbTab & patchOrProductSource
    Next

    ' Get the Media disks
    message = message & tabbedewLine & vbTab & "Media Sources"
    Set mediaDisks = patchOrProduct.MediaDisks: CheckError
    For Each mediaDisk In mediaDisks
       message = message & tabbedewLine & vbTab & vbTab & "Disk ID = " & mediaDisk.StringData(1)
       message = message & tabbedewLine & vbTab & vbTab & "Volume = " & mediaDisk.StringData(2)
       message = message & tabbedewLine & vbTab & vbTab & "DiskPrompt = " & mediaDisk.StringData(3)
    Next

    ' Get other source list information
    mediaPackagePath = patchOrProduct.SourceListInfo("MediaPackagePath")
    message = message & tabbedewLine & vbTab & "MediaPackagePath = " & mediaPackagePath
    diskPrompt = patchOrProduct.SourceListInfo("DiskPrompt")
    message = message & tabbedewLine & vbTab & "DiskPrompt = " & diskPrompt
    lastUsedSource = patchOrProduct.SourceListInfo("LastUsedSource")
    message = message & tabbedewLine & vbTab & "LastUsedSource = " & lastUsedSource
    LastUsedType = patchOrProduct.SourceListInfo("LastUsedType")
    message = message & tabbedewLine & vbTab & "LastUsedType = " & LastUsedType
    packageName = patchOrProduct.SourceListInfo("PackageName")
    message = message & tabbedewLine & vbTab & "PackageName = " & packageName
End Sub

Sub CheckError
	Dim message, errRec
	If Err = 0 Then Exit Sub
	message = Err.Source & " " & Hex(Err) & ": " & Err.Description
	If Not installer Is Nothing Then
		Set errRec = installer.LastErrorRecord
		If Not errRec Is Nothing Then message = message & vbNewLine & errRec.FormatText
	End If
	Wscript.Echo message
	Wscript.Quit 2
End Sub

Function MapContext(context)
   Select Case (context)
		Case 1  : MapContext = "MSIINSTALLCONTEXT_USERMANAGED"
		Case 2  : MapContext = "MSIINSTALLCONTEXT_USER"
		Case 3  : MapContext = "MSIINSTALLCONTEXT_USERMANAGED + MSIINSTALLCONTEXT_USER"
      Case 4  : MapContext = "MSIINSTALLCONTEXT_MACHINE"
      Case 5  : MapContext = "MSIINSTALLCONTEXT_USERMANAGED + MSIINSTALLCONTEXT_MACHINE"
      Case 6  : MapContext = "MSIINSTALLCONTEXT_USER + MSIINSTALLCONTEXT_MACHINE"
      Case 7  : MapContext = "MSIINSTALLCONTEXT_USERMANAGED + MSIINSTALLCONTEXT_USER + MSIINSTALLCONTEXT_MACHINE"      
   End Select
End Function

Sub Usage
    Wscript.Echo "Windows Installer utility to list products and product information" &_
        vbNewLine & " Lists product information of all products of the current user for all contexts if no arguments are specified" &_
        vbNewLine & " Else 1st argument is a product name or product code (GUID)" &_
    vbNewLine & " 2nd argument is Install Context. If not specified all contexts are searched" &_
    vbNewLine & "     Use 'm' to indicate managed context" &_
    vbNewLine & "     Use 'u' to indicate unmanaged context" &_
    vbNewLine & "     Use 'a' to indicate machine context" &_
    vbNewLine & " 3rd argument is User SID. If not specified current user is the default. Do not set this argument when viewing machine context" &_
    vbNewLine &_
    vbNewLine & "Copyright (C) Microsoft Corporation.  All rights reserved."
    Wscript.Quit 1
End Sub
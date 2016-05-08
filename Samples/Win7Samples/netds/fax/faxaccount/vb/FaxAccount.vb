'==========================================================================
'
' THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
' ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
' THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
' PARTICULAR PURPOSE.
'
' Copyright (c) Microsoft Corporation. All rights reserved.
'
'--------------------------------------------------------------------------

Imports System
Imports System.Globalization
Imports System.Security.Permissions
Imports Microsoft.VisualBasic
Imports Microsoft.VisualBasic.ApplicationServices

<Assembly: System.Reflection.AssemblyKeyFile("key.snk")> 
<Assembly: CLSCompliant(True)> 
Namespace Microsoft.Samples.Fax.FaxAccounts.VB

    Module FaxAccount
       
        '+---------------------------------------------------------------------------
        '
        '  function:   GiveUsage
        '
        '  Synopsis:   prints the usage of the application
        '
        '  Arguments:  void
        '
        '  Returns:    void
        '
        '  Modifies:
        '
        '----------------------------------------------------------------------------
        Sub GiveUsage()
            System.Console.WriteLine("Usage : " + System.Diagnostics.Process.GetCurrentProcess().ProcessName)
            System.Console.WriteLine(" /s Fax Server Name ")
            System.Console.WriteLine(" /o <Add/Delete/Validate/Enum> Account option  ")
            System.Console.WriteLine(" /a Account Name Only if the option is Add/Delete or Validate ")
            System.Console.WriteLine("Usage : " + System.Diagnostics.Process.GetCurrentProcess().ProcessName + " /? -- help message")
        End Sub
        '+---------------------------------------------------------------------------
        '
        '  function:   IsOSVersionCompatible
        '
        '  Synopsis:   finds whether the target OS supports this functionality.
        '
        '  Arguments:  [iVersion] - Minimum Version of the OS required for the Sample to run.
        '
        '  Returns:    bool - true if the Sample can run on this OS
        '
        '----------------------------------------------------------------------------
        Function IsOSVersionCompatible(ByVal iVersion As Integer) As Boolean
            Dim os As OperatingSystem
            Dim osVersion As Version

            os = Environment.OSVersion
            osVersion = os.Version
            If (osVersion.Major >= iVersion) Then
                Return True
            Else
                Return False
            End If
        End Function

        '+---------------------------------------------------------------------------
        '
        '  function:   DisplayFaxAccount
        '
        '  Synopsis:   prints the display name of a Fax Account
        '
        '  Arguments:  [objFaxAcc] - FaxAccount object whose display name is to be printed
        '
        '  Returns:    bool: true if passed successfully
        '
        '  Modifies:
        '
        '----------------------------------------------------------------------------
        Function DisplayFaxAccount(ByVal objFaxAcc As FAXCOMEXLib.IFaxAccount) As Boolean
            If (TypeOf objFaxAcc Is FAXCOMEXLib.IFaxAccount) Then
                'Print the account name
                System.Console.WriteLine("Fax Account Name: " + objFaxAcc.AccountName)
                Return True
            End If
            System.Console.WriteLine("DisplayFaxAccount: Parameter is NULL")
            Return False


        End Function
        '+---------------------------------------------------------------------------
        '
        '  function:   FaxEnumAccounts
        '
        '  Synopsis:   Enumerates the list of accounts
        '
        '  Arguments:  [objFaxAccSet] - FaxAccountSet object having the list of all Fax Accounts
        '                [bCheck] - if set to true then is verifies if the account with name strAccName is present.
        '                [strAccName] - name of the account that is to be verified.
        '                [pbFound] - used to return the result of whether the account is present or not
        '
        '  Returns:    bool: true if passed successfully
        '
        '  Modifies:    pbFound : if the account with the name strAccName is found then pbFound is set to true.
        '
        '----------------------------------------------------------------------------
        Function FaxEnumAccounts(ByVal objFaxAccSet As FAXCOMEXLib.IFaxAccountSet, ByVal bCheck As Boolean, ByVal strAccName As String, ByRef pbFound As Boolean) As Boolean
            If (TypeOf objFaxAccSet Is FAXCOMEXLib.IFaxAccountSet) Then
                Dim objFaxAccounts As FAXCOMEXLib.IFaxAccounts
                Dim objFaxEnum As System.Collections.IEnumerator
                Dim bLast As Boolean
                Dim objFaxAccount As FAXCOMEXLib.IFaxAccount

                pbFound = False
                'Get the FaxAccounts object 
                objFaxAccounts = objFaxAccSet.GetAccounts()
                'Print the number of FaxAccounts
                System.Console.WriteLine("Number of accounts: " + objFaxAccounts.Count.ToString(CultureInfo.CurrentCulture.NumberFormat))

                'start enumerating each account.
                objFaxEnum = objFaxAccounts.GetEnumerator()

                While (True)
                    bLast = objFaxEnum.MoveNext()
                    If (bLast = False) Then
                        'enumeration is done
                        System.Console.WriteLine("Enumeration of accounts done.")
                        Exit While
                    End If

                    objFaxAccount = objFaxEnum.Current
                    'if check that a account is present.
                    If (bCheck) Then
                        If (String.IsNullOrEmpty(strAccName) = False) Then
                            If (String.Compare(objFaxAccount.AccountName.ToLower(CultureInfo.CurrentCulture), strAccName.ToLower(CultureInfo.CurrentCulture), True, CultureInfo.CurrentCulture) = 0) Then
                                pbFound = True
                            End If
                        Else
                            System.Console.WriteLine("FaxEnumAccounts: strAccName parameter is NULL")
                            Return False
                        End If
                    End If
                    'Display the current account info.
                    DisplayFaxAccount(objFaxAccount)
                End While
                Return True
            End If
            System.Console.WriteLine("FaxEnumAccounts: Parameter is NULL")
            Return False

        End Function
        '+---------------------------------------------------------------------------
        '
        '  function:   AddAccount
        '
        '  Synopsis:   Adds a Fax Account
        '
        '  Arguments:  [objFaxAccSet] - FaxAccountSet object having the list of all Fax Accounts
        '                [strAccName] - name of the account to be added. Must be a valid NT/Domain user.
        '
        '  Returns:    bool: true if passed successfully
        '
        '  Modifies:
        '
        '----------------------------------------------------------------------------
        Function AddAccount(ByVal objFaxAccSet As FAXCOMEXLib.IFaxAccountSet, ByVal strAccName As String) As Boolean
            If (TypeOf objFaxAccSet Is FAXCOMEXLib.IFaxAccountSet) And String.IsNullOrEmpty(strAccName) = False Then
                Dim bFound As Boolean
                Dim objFaxAccount As FAXCOMEXLib.IFaxAccount
                bFound = True
                'first enum the existing accounts
                If (FaxEnumAccounts(objFaxAccSet, False, Nothing, bFound) = False) Then
                    'enum failed 
                    System.Console.WriteLine("FaxEnumAccounts failed")
                End If
                'now add the account
                objFaxAccount = objFaxAccSet.AddAccount(strAccName)
                'Display Info on added account.
                DisplayFaxAccount(objFaxAccount)
                Return True
            End If
            System.Console.WriteLine("AddAccount: Parameter is NULL")
            Return False
        End Function
        '+---------------------------------------------------------------------------
        '
        '  function:   DeleteAccount
        '
        '  Synopsis:   Deletes a Fax Account
        '
        '  Arguments:  [objFaxAccSet] - FaxAccountSet object having the list of all Fax Accounts
        '                [strAccountName] - name of the account to be deleted. 
        '
        '  Returns:    bool: true if passed successfully
        '
        '  Modifies:
        '
        '----------------------------------------------------------------------------
        Function DeleteAccount(ByVal objFaxAccSet As FAXCOMEXLib.IFaxAccountSet, ByVal strAccountName As String) As Boolean
            If (TypeOf objFaxAccSet Is FAXCOMEXLib.IFaxAccountSet) And String.IsNullOrEmpty(strAccountName) = False Then
                Dim bFound As Boolean
                'Dim objFaxAccount As FAXCOMEXLib.IFaxAccount
                bFound = True
                'first enum the existing accounts
                If (FaxEnumAccounts(objFaxAccSet, False, Nothing, bFound) = False) Then
                    'FaxEnumAccounts failed
                    System.Console.WriteLine("FaxEnumAccounts failed before deleting")
                End If
                objFaxAccSet.RemoveAccount(strAccountName)
                'now enumerate to see if account exists            
                If (FaxEnumAccounts(objFaxAccSet, True, strAccountName, bFound) = False) Then
                    'we can properly validate if this call fails, hence log an error
                    System.Console.WriteLine("FaxEnumAccounts failed during validation")
                    Return False
                End If
                If (bFound) Then
                    'we just deleted the account but still the enumeration shows the account. hecen log error
                    System.Console.WriteLine("Account exists after deleteion")
                    Return False
                End If
                Return True
            End If
            System.Console.WriteLine("DeleteAccount: Parameter is NULL")
            Return False
        End Function
        '+---------------------------------------------------------------------------
        '
        '  function:   GetAccountInfo
        '
        '  Synopsis:   Get the account object.
        '
        '  Arguments:  [objFaxAccSet] - FaxAccountSet object having the list of all Fax Accounts
        '                [strAccountName] - Account whose info is to be printed.
        '
        '  Returns:    bool: true if passed successfully
        '
        '  Modifies:
        '
        '----------------------------------------------------------------------------
        Function GetAccountInfo(ByVal objFaxAccSet As FAXCOMEXLib.IFaxAccountSet, ByVal strAccountName As String) As Boolean
            If (TypeOf objFaxAccSet Is FAXCOMEXLib.IFaxAccountSet) And String.IsNullOrEmpty(strAccountName) = False Then
                Dim objFaxAccount As FAXCOMEXLib.IFaxAccount
                'Get the account with the name lptstrAccountName
                objFaxAccount = objFaxAccSet.GetAccount(strAccountName)
                DisplayFaxAccount(objFaxAccount)
                Return True
            End If
            System.Console.WriteLine("GetAccountInfo: Parameter is NULL")
            Return False
        End Function

        Sub Main()
            Dim objFaxServer As FAXCOMEXLib.FaxServer
            Dim objFaxAccountSet As FAXCOMEXLib.FaxAccountSet
            Dim strServerName As String
            Dim strAccountName As String
            Dim strOption As String
            Dim bConnected As Boolean
            Dim bFound As Boolean
            Dim count As Integer
            Dim args As String
            Dim iVista As Integer
            Dim bVersion As Boolean
            Dim bRetVal As Boolean

            iVista = 6
            bVersion = IsOSVersionCompatible(iVista)

            If (bVersion = False) Then
                System.Console.WriteLine("OS Version does not support this feature")
                bRetVal = False
                Return
            End If

            strServerName = ""
            strAccountName = Nothing
            strOption = Nothing
            objFaxServer = Nothing
            bRetVal = True
            bFound = True
            bConnected = False
            count = 0
            args = Nothing

            Try
                If ((My.Application.CommandLineArgs.Count = 0)) Then
                    System.Console.WriteLine("Missing args.")
                    GiveUsage()
                    bRetVal = False
                    GoTo ExitFun
                End If


                ' check for commandline switches
                Do Until count = My.Application.CommandLineArgs.Count -1
                    If count >= My.Application.CommandLineArgs.Count - 1 Then
                        Exit Do
                    End If
                    args = My.Application.CommandLineArgs.Item(count)
                    If ((String.Compare(args.Substring(0, 1), "/", True, CultureInfo.CurrentCulture) = 0) Or (String.Compare(args.Substring(0, 1), "-", True, CultureInfo.CurrentCulture) = 0)) Then
                        Select Case (((args.ToLower(CultureInfo.CurrentCulture).Substring(1, 1))))
                            Case "s"
                                If (String.IsNullOrEmpty(strServerName) = False) Then
                                    GiveUsage()
                                    bRetVal = False
                                    GoTo ExitFun
                                End If
                                strServerName = My.Application.CommandLineArgs.Item(count + 1)
                            Case "o"
                                If (String.IsNullOrEmpty(strOption) = False) Then
                                    GiveUsage()
                                    bRetVal = False
                                    GoTo ExitFun
                                End If
                                strOption = My.Application.CommandLineArgs.Item(count + 1)
                            Case "a"
                                If (String.IsNullOrEmpty(strAccountName) = False) Then
                                    GiveUsage()
                                    bRetVal = False
                                    GoTo ExitFun
                                End If
                                strAccountName = My.Application.CommandLineArgs.Item(count + 1)
                            Case "?"
                                GiveUsage()
                                bRetVal = False
                                GoTo ExitFun
                            Case Else
                        End Select
                    End If
                    count = count + 1
                Loop

                If (strOption = Nothing) Then
                    System.Console.WriteLine("Missing args.")
                    GiveUsage()
                    bRetVal = False
                    GoTo ExitFun
                End If


                If ((String.Compare("enum", strOption.ToLower(CultureInfo.CurrentCulture), True, CultureInfo.CurrentCulture) <> 0) And (strAccountName = Nothing)) Then
                    System.Console.WriteLine("Missing args.")
                    GiveUsage()
                    bRetVal = False
                    GoTo ExitFun
                End If

                objFaxServer = New FAXCOMEXLib.FaxServer()
                'Connect to Fax Server
                objFaxServer.Connect(strServerName)
                bConnected = True

                If (objFaxServer.APIVersion < FAXCOMEXLib.FAX_SERVER_APIVERSION_ENUM.fsAPI_VERSION_3) Then
                    bRetVal = False
                    System.Console.WriteLine("Feature not available on this version of the Fax API")
                    GoTo ExitFun
                End If

                'lets also get the account set since that is the basis for all account relates operations
                objFaxAccountSet = objFaxServer.FaxAccountSet

                'if Enum Account option is selected
                If (String.Compare("enum", strOption.ToLower(CultureInfo.CurrentCulture), True, CultureInfo.CurrentCulture) = 0) Then
                    If (FaxEnumAccounts(objFaxAccountSet, False, Nothing, bFound) = False) Then

                        'we dont want to log any error here as the error will be logged in the function itself
                        bRetVal = False
                    End If
                End If
                'if Add Account option is selected
                If (String.Compare("add", strOption.ToLower(CultureInfo.CurrentCulture), True, CultureInfo.CurrentCulture) = 0) Then
                    If (AddAccount(objFaxAccountSet, strAccountName) = False) Then
                        bRetVal = False
                    End If
                End If

                'if Delete Account option is selected
                If (String.Compare("delete", strOption.ToLower(CultureInfo.CurrentCulture), True, CultureInfo.CurrentCulture) = 0) Then
                    If (DeleteAccount(objFaxAccountSet, strAccountName) = False) Then
                        bRetVal = False
                    End If
                End If


                'if validate account option is selected
                If (String.Compare("validate", strOption.ToLower(CultureInfo.CurrentCulture), True, CultureInfo.CurrentCulture) = 0) Then
                    If (GetAccountInfo(objFaxAccountSet, strAccountName) = False) Then
                        bRetVal = False
                    End If
                End If

            Catch excep As Exception
                System.Console.WriteLine("Exception:" + excep.Message)
            End Try
ExitFun:
            If (bConnected) Then
                objFaxServer.Disconnect()
            End If
            If (bRetVal = False) Then
                System.Console.WriteLine("Function Failed")
            End If
        End Sub
    End Module
End Namespace
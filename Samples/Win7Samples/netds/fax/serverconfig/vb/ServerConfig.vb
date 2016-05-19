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
Imports FAXCOMEXLib

<Assembly: System.Reflection.AssemblyKeyFile("key.snk")> 
<Assembly: CLSCompliant(True)> 
Namespace Microsoft.Samples.Fax.ServerConfig.VB

    Module ServerConfig
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
            System.Console.WriteLine(" /o <PersonalCoverPage/Branding/IncomingFaxesPublic/AutoCreateAccount> option ")
            System.Console.WriteLine(" /v value to be set '0' or '1' ")
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
        '  function:   PrintGeneralConfig
        '
        '  Synopsis:   prints the Server Configuration (PersonalCoverPages, Branding, IncomingFaxPublic and AutoCreateOnConnect)
        '
        '  Arguments:  [pFaxConfiguration] - FaxConfiguration object pointing to the configuration of the current server
        '
        '  Returns:    bool: true if passed successfully
        '
        '----------------------------------------------------------------------------
        Function PrintGeneralConfig(ByVal objFaxConfiguration As IFaxConfiguration) As Boolean
            Dim bRetVal As Boolean
            Dim bValue As Boolean

            bRetVal = True
            bValue = False

            If (TypeOf objFaxConfiguration Is FAXCOMEXLib.IFaxConfiguration) Then
               
                System.Console.WriteLine()
                System.Console.WriteLine("Logging Gerneral Config details....")
                System.Console.WriteLine()

                objFaxConfiguration.Refresh()
                bValue = objFaxConfiguration.AllowPersonalCoverPages

                If (bValue) Then
                    System.Console.WriteLine("AllowPersonalCoverPages = true")
                Else
                    System.Console.WriteLine("AllowPersonalCoverPages = false")
                End If

                bValue = objFaxConfiguration.AutoCreateAccountOnConnect
                If (bValue) Then
                    System.Console.WriteLine("AutoCreateOnConnect = true")
                Else
                    System.Console.WriteLine("AutoCreateOnConnect = false")
                End If

                bValue = objFaxConfiguration.Branding

                If (bValue) Then
                    System.Console.WriteLine("Branding = true")
                Else
                    System.Console.WriteLine("Branding = false")
                End If

                bValue = objFaxConfiguration.IncomingFaxesArePublic

                If (bValue) Then
                    System.Console.WriteLine("IncomingFaxesArePublic = true")
                Else
                    System.Console.WriteLine("IncomingFaxesArePublic = false")
                End If

                Return bRetVal
            Else
                System.Console.WriteLine("PrintGeneralConfig: Parameter passed is NULL")
                Return False
            End If
        End Function

        '+---------------------------------------------------------------------------
        '
        '  function:   setIncomingFaxesArePublic
        '
        '  Synopsis:   sets the valus of IncomingFaxArePublic according to bState value
        '
        '  Arguments:  [pFaxConfiguration] - FaxConfiguration object pointing to the configuration of the current server
        '                [bState] -    bool value set to true or false
        '
        '  Returns:    bool: true if passed successfully
        '
        '----------------------------------------------------------------------------
        Function setIncomingFaxesArePublic(ByVal objFaxConfiguration As IFaxConfiguration, ByVal bState As Boolean) As Boolean
            If (TypeOf objFaxConfiguration Is FAXCOMEXLib.IFaxConfiguration) Then
                
                objFaxConfiguration.Refresh()
                'Set the configuration object
                objFaxConfiguration.IncomingFaxesArePublic = bState
                'Save it
                objFaxConfiguration.Save()
                Return True
            Else
                System.Console.WriteLine("setIncomingFaxesArePublic: Parameter passed is NULL")
                Return False
            End If

        End Function
        '+---------------------------------------------------------------------------
        '
        '  function:   setAllowPersonalCoverPages
        '
        '  Synopsis:   sets the valus of AllowPersonalCoverPages according to bState value
        '
        '  Arguments:  [pFaxConfiguration] - FaxConfiguration object pointing to the configuration of the current server
        '                [bState] -    bool value set to true or false
        '
        '  Returns:    bool: true if passed successfully
        '
        '----------------------------------------------------------------------------
        Function setAllowPersonalCoverPages(ByVal objFaxConfiguration As IFaxConfiguration, ByVal bState As Boolean) As Boolean
            If (TypeOf objFaxConfiguration Is FAXCOMEXLib.IFaxConfiguration) Then
              
                objFaxConfiguration.Refresh()
                'Set the configuration object
                objFaxConfiguration.AllowPersonalCoverPages = bState
                'Save it
                objFaxConfiguration.Save()
                Return True
            Else
                System.Console.WriteLine("setAllowPersonalCoverPages: Parameter passed is NULL")
                Return False
            End If
        End Function
        '+---------------------------------------------------------------------------
        '
        '  function:   setBranding
        '
        '  Synopsis:   sets the valus of Branding according to bState value
        '
        '  Arguments:  [pFaxConfiguration] - FaxConfiguration object pointing to the configuration of the current server
        '                [bState] -    bool value set to true or false
        '
        '  Returns:    bool: true if passed successfully
        '
        '----------------------------------------------------------------------------
        Function setBranding(ByVal objFaxConfiguration As IFaxConfiguration, ByVal bState As Boolean) As Boolean
            If (TypeOf objFaxConfiguration Is FAXCOMEXLib.IFaxConfiguration) Then
                objFaxConfiguration.Refresh()
                'Set the configuration object
                objFaxConfiguration.Branding = bState
                'Save it
                objFaxConfiguration.Save()
                Return True
            Else
                System.Console.WriteLine("setBranding: Parameter passed is NULL")
                Return False
            End If

        End Function


        '+---------------------------------------------------------------------------
        '
        '  function:   setAutoCreateAccountOnConnect
        '
        '  Synopsis:   sets the valus of AutoCreateAccountonConnect according to bState value
        '
        '  Arguments:  [pFaxConfiguration] - FaxConfiguration object pointing to the configuration of the current server
        '                [bState] -    bool value set to true or false
        '
        '  Returns:    bool: true if passed successfully
        '
        '----------------------------------------------------------------------------
        Function setAutoCreateAccountOnConnect(ByVal objFaxConfiguration As IFaxConfiguration, ByVal bState As Boolean) As Boolean
            If (TypeOf objFaxConfiguration Is FAXCOMEXLib.IFaxConfiguration) Then
                System.Console.WriteLine(bState)
                objFaxConfiguration.Refresh()
                'Set the configuration object
                objFaxConfiguration.AutoCreateAccountOnConnect = bState
                'Save it
                objFaxConfiguration.Save()
                Return True
            Else
                System.Console.WriteLine("setAutoCreateAccountOnConnect: Parameter passed is NULL")
                Return False
            End If
        End Function

        Sub Main()
            Dim objFaxServer As FAXCOMEXLib.FaxServer
            Dim objFaxConfiguration As FAXCOMEXLib.IFaxConfiguration
            Dim strServerName As String
            Dim strValue As String
            Dim strOption As String
            Dim bConnected As Boolean
            Dim bRetVal As Boolean
            Dim count As Integer
            Dim args As String
            Dim bState As Boolean

            Dim iVista As Integer
            Dim bVersion As Boolean

            iVista = 6
            bVersion = IsOSVersionCompatible(iVista)

            If (bVersion = False) Then
                System.Console.WriteLine("OS Version does not support this feature")
                bRetVal = False
                Return
            End If

            strServerName = Nothing 'String.Empty
            strValue = String.Empty
            strOption = Nothing 'String.Empty
            objFaxServer = Nothing
            bRetVal = True
            bState = True
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
                Do Until count = My.Application.CommandLineArgs.Count
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
                            Case "v"
                                If (String.IsNullOrEmpty(strValue) = False) Then
                                    GiveUsage()
                                    bRetVal = False
                                    GoTo ExitFun
                                End If
                                strValue = My.Application.CommandLineArgs.Item(count + 1)
                            Case "?"
                                GiveUsage()
                                bRetVal = False
                                GoTo ExitFun
                            Case Else
                        End Select
                    End If
                    count = count + 1
                Loop

                If ((String.IsNullOrEmpty(strOption) = True) Or (String.IsNullOrEmpty(strValue) = True) Or ((String.Compare("0", strValue, True, CultureInfo.CurrentCulture) <> 0) And (String.Compare("1", strValue, True, CultureInfo.CurrentCulture) <> 0))) Then
                    System.Console.WriteLine("Missing args.")
                    GiveUsage()
                    bRetVal = False
                    GoTo ExitFun
                End If

                objFaxServer = New FAXCOMEXLib.FaxServer()
                'Connect to Fax Server
                objFaxServer.Connect(strServerName)
                bConnected = True

                'check API version
                If (objFaxServer.APIVersion < FAXCOMEXLib.FAX_SERVER_APIVERSION_ENUM.fsAPI_VERSION_3) Then
                    bRetVal = False
                    System.Console.WriteLine("OS Version does not support this feature")
                    GoTo ExitFun
                End If

                objFaxConfiguration = objFaxServer.Configuration
                If (String.Compare("0", strValue, True, CultureInfo.CurrentCulture) = 0) Then
                    bState = False
                End If
                If (String.Compare("1", strValue, True, CultureInfo.CurrentCulture) = 0) Then
                    bState = True
                End If

                System.Console.WriteLine()
                System.Console.WriteLine("Current Configuration.")
                System.Console.WriteLine()
                If (PrintGeneralConfig(objFaxConfiguration) = False) Then

                    'we dont want to log any error here as the error will be logged in the function itself
                    bRetVal = False
                End If


                'if PersonalCoverPages option is selected
                If (String.Compare("personalcoverpage", strOption.ToLower(CultureInfo.CurrentCulture), True, CultureInfo.CurrentCulture) = 0) Then
                    If (setAllowPersonalCoverPages(objFaxConfiguration, bState) = False) Then
                        'we dont want to log any error here as the error will be logged in the function itself
                        bRetVal = False
                    End If
                End If

                'if Branding option is selected
                If (String.Compare("branding", strOption.ToLower(CultureInfo.CurrentCulture), True, CultureInfo.CurrentCulture) = 0) Then
                    If (setBranding(objFaxConfiguration, bState) = False) Then
                        'we dont want to log any error here as the error will be logged in the function itself
                        bRetVal = False
                    End If
                End If

                'if IncomingFaxArePublic option is selected
                If (String.Compare("incomingfaxespublic", strOption.ToLower(CultureInfo.CurrentCulture), True, CultureInfo.CurrentCulture) = 0) Then
                    If (setIncomingFaxesArePublic(objFaxConfiguration, bState) = False) Then
                        'we dont want to log any error here as the error will be logged in the function itself
                        bRetVal = False
                    End If
                End If
                'if AutoCreateAccount option is selected

                If (String.Compare("autocreateaccount", strOption.ToLower(CultureInfo.CurrentCulture), True, CultureInfo.CurrentCulture) = 0) Then
                    If (setAutoCreateAccountOnConnect(objFaxConfiguration, bState) = False) Then
                        'we dont want to log any error here as the error will be logged in the function itself
                        bRetVal = False
                    End If
                End If

                System.Console.WriteLine("Current Server settings after the changes...")
                If (PrintGeneralConfig(objFaxConfiguration) = False) Then

                    'we dont want to log any error here as the error will be logged in the function itself
                    bRetVal = False
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


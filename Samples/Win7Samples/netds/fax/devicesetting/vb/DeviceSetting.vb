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
Imports System.Collections
Imports System.Globalization
Imports System.Security.Permissions
Imports Microsoft.VisualBasic
Imports Microsoft.VisualBasic.ApplicationServices

<Assembly: System.Reflection.AssemblyKeyFile("key.snk")> 
<Assembly: CLSCompliant(True)> 
Namespace Microsoft.Samples.Fax.DeviceSetting.VB
    Module DeviceSetting
        
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
            System.Console.WriteLine(" /l <list/set> Devices on the server ")
            System.Console.WriteLine(" /i device id of the device whose property (TSID or CSID) has to be set ")
            System.Console.WriteLine(" /c new CSID value for the device ")
            System.Console.WriteLine(" /t new TSID value for the device ")
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
        '  function:   setTSID
        '
        '  Synopsis:   sets the value of TSID for a FaxDevice
        '
        '  Arguments:  [objFaxDevices] - FaxDevices object pointing to the list of devices on the server
        '               [lDeviceId] - Device Id of the device to be set
        '               [strTSID] -    value of the TSID
        '
        '  Returns:    bool: true if passed successfully
        '       
        '----------------------------------------------------------------------------
        Function setTSID(ByVal objFaxDevices As FAXCOMEXLib.IFaxDevices, ByVal iDeviceId As Integer, ByVal strTSID As String) As Boolean
            If ((TypeOf objFaxDevices Is FAXCOMEXLib.IFaxDevices) And (String.IsNullOrEmpty(strTSID) = False)) Then
                Dim objFaxDevice As FAXCOMEXLib.FaxDevice
                objFaxDevice = objFaxDevices.ItemById(iDeviceId)
                'set TSID
                objFaxDevice.TSID = strTSID
                'Save it
                objFaxDevice.Save()
                System.Console.WriteLine("New TSID is set")
                Return True
            End If
            System.Console.WriteLine("setTSID: Parameter is NULL")
            Return False
        End Function
        '+---------------------------------------------------------------------------
        '
        '  function:   setCSID
        '
        '  Synopsis:   sets the value of TSID for a FaxDevice
        '
        '  Arguments:  [objFaxDevices] - FaxDevices object pointing to the list of devices on the server
        '               [iDeviceId] - Device Id of the device to be set
        '               [strCSID] -    value of the CSID
        '
        '  Returns:    bool: true if passed successfully
        '
        '  Modifies:
        '
        '----------------------------------------------------------------------------
        Function setCSID(ByVal objFaxDevices As FAXCOMEXLib.IFaxDevices, ByVal iDeviceId As Integer, ByVal strCSID As String) As Boolean
            If ((TypeOf objFaxDevices Is FAXCOMEXLib.IFaxDevices) And (String.IsNullOrEmpty(strCSID) = False)) Then
                Dim objFaxDevice As FAXCOMEXLib.FaxDevice
                objFaxDevice = objFaxDevices.ItemById(iDeviceId)
                'set CSID
                objFaxDevice.CSID = strCSID
                'Save it
                objFaxDevice.Save()
                System.Console.WriteLine("New CSID is set")
                Return True
            End If
            System.Console.WriteLine("setCSID: Parameter is NULL")
            Return False
        End Function
        '+---------------------------------------------------------------------------
        '
        '  function:   setTSID
        '
        '  Synopsis:   sets the value of TSID for a FaxDevice
        '
        '  Arguments:  [pFaxDevices] - FaxDevices object pointing to the list of devices on the server
        '
        '  Returns:    bool: true if passed successfully
        '
        '----------------------------------------------------------------------------
        Function listDevices(ByVal objFaxDevices As FAXCOMEXLib.IFaxDevices) As Boolean

            Dim objFaxDevice As FAXCOMEXLib.FaxDevice
            Dim count As Integer
            Dim i As Integer
            i = 0
            If (TypeOf objFaxDevices Is FAXCOMEXLib.IFaxDevices) Then
                count = objFaxDevices.Count
                While i < count
                    If (i >= count) Then
                        Exit While
                    End If
                    objFaxDevice = objFaxDevices.Item(i + 1)
                    System.Console.Write("Device No: ")
                    System.Console.Write(i)
                    System.Console.Write("  Device Id = ")
                    System.Console.Write(objFaxDevice.Id)
                    System.Console.Write("  Device Name = ")
                    System.Console.Write(objFaxDevice.DeviceName)
                    System.Console.WriteLine()
                    i = i + 1
                End While
                Return True
            End If
            System.Console.WriteLine("listDevices: Parameter is NULL")
            Return False
        End Function


        Sub Main()
            Dim objFaxServer As FAXCOMEXLib.FaxServer
            Dim objFaxDevices As FAXCOMEXLib.FaxDevices
            Dim strServerName As String
            Dim strOption As String
            Dim strTSID As String
            Dim strCSID As String
            Dim strDeviceId As String
            Dim bConnected As Boolean
            Dim bRetVal As Boolean
            Dim args As String
            Dim count As Integer
            Dim iDeviceId As Integer

            Dim iVista As Integer
            Dim bVersion As Boolean

            iVista = 6
            bVersion = IsOSVersionCompatible(iVista)

            If (bVersion = False) Then
                System.Console.WriteLine("This sample is compatible with Windows Vista")
                bRetVal = False
                Return
            End If

            objFaxServer = Nothing
            strServerName = ""
            strDeviceId = Nothing
            strTSID = Nothing
            strCSID = Nothing
            strOption = Nothing
            bRetVal = True
            bConnected = False
            count = 0

            Try
                If ((My.Application.CommandLineArgs.Count = 0)) Then
                    System.Console.WriteLine("Missing args.")
                    GiveUsage()
                    bRetVal = False
                    GoTo ExitFun
                End If

                ' check for commandline switches
                Do Until count = My.Application.CommandLineArgs.Count - 1
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
                            Case "l"
                                If (String.IsNullOrEmpty(strOption) = False) Then
                                    GiveUsage()
                                    bRetVal = False
                                    GoTo ExitFun
                                End If
                                strOption = My.Application.CommandLineArgs.Item(count + 1)
                            Case "i"
                                If (String.IsNullOrEmpty(strDeviceId) = False) Then
                                    GiveUsage()
                                    bRetVal = False
                                    GoTo ExitFun
                                End If
                                strDeviceId = My.Application.CommandLineArgs.Item(count + 1)
                            Case "c"
                                If (String.IsNullOrEmpty(strCSID) = False) Then
                                    GiveUsage()
                                    bRetVal = False
                                    GoTo ExitFun
                                End If
                                strCSID = My.Application.CommandLineArgs.Item(count + 1)
                            Case "t"
                                If (String.IsNullOrEmpty(strTSID) = False) Then
                                    GiveUsage()
                                    bRetVal = False
                                    GoTo ExitFun
                                End If
                                strTSID = My.Application.CommandLineArgs.Item(count + 1)
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

                If ((String.Compare("set", strOption.ToLower(CultureInfo.CurrentCulture), True, CultureInfo.CurrentCulture) = 0) And ((strDeviceId = Nothing) Or (strCSID = Nothing And strTSID = Nothing))) Then
                    System.Console.WriteLine("Missing args.")
                    GiveUsage()
                    bRetVal = False
                    GoTo ExitFun
                End If

                objFaxServer = New FAXCOMEXLib.FaxServer
                'Connect to Fax Server
                objFaxServer.Connect(strServerName)
                bConnected = True

                'check API version
                If (objFaxServer.APIVersion < FAXCOMEXLib.FAX_SERVER_APIVERSION_ENUM.fsAPI_VERSION_3) Then
                    bRetVal = False
                    System.Console.WriteLine("This sample is compatible with Windows Vista")
                    GoTo ExitFun
                End If

                objFaxDevices = objFaxServer.GetDevices()
                'if "list" devices is selected
                If (String.Compare("list", strOption.ToLower(CultureInfo.CurrentCulture), True, CultureInfo.CurrentCulture) = 0) Then
                    If (listDevices(objFaxDevices) = False) Then
                        bRetVal = False
                    End If
                End If

                'if set device option is selected
                If (String.Compare("set", strOption.ToLower(CultureInfo.CurrentCulture), True, CultureInfo.CurrentCulture) = 0) Then
                    iDeviceId = Int32.Parse(strDeviceId, CultureInfo.CurrentCulture.NumberFormat)
                    'if set TSID is selected
                    If (strTSID <> Nothing) Then
                        If (setTSID(objFaxDevices, iDeviceId, strTSID) = False) Then
                            bRetVal = False
                        End If
                    End If
                    'if set CSID is selected
                    If (strCSID <> Nothing) Then
                        If (setCSID(objFaxDevices, iDeviceId, strCSID) = False) Then
                            bRetVal = False
                        End If
                    End If
                End If

            Catch excep As Exception
                System.Console.WriteLine("Exception Occured")
                System.Console.WriteLine(excep.Message)
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

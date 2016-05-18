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
Namespace Microsoft.Samples.Fax.SendToFax.VB

    Module SendToFax

        'Function exported by fxsutility.dll
        Public Enum SendToMode
            SendToFaxRecipientAttachment
        End Enum
        Public Class SendToFax
            Private Sub New()
            End Sub
            Declare Unicode Function CanSendToFaxRecipient Lib "fxsutility.dll" () As Boolean
            Declare Unicode Function SendToFaxRecipient Lib "fxsutility.dll" (ByVal enumType As SendToMode, ByVal strDoc As Char()) As UInt32
        End Class

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
        '----------------------------------------------------------------------------
        Sub GiveUsage()
            System.Console.WriteLine("Usage : " + System.Diagnostics.Process.GetCurrentProcess().ProcessName)
            System.Console.WriteLine(" /o <cansendtofax> or <sendtofax> ")
            System.Console.WriteLine(" /d Document that is to be sent as Fax ")
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


        Sub Main()
            Dim strDoc As String
            Dim strOption As String
            Dim bRetVal As Boolean
            Dim iVista As Integer
            Dim iRet As UInt32
            Dim bVersion As Boolean
            Dim count As Integer
            Dim args As String

            iVista = 6
            bVersion = IsOSVersionCompatible(iVista)

            If (bVersion = False) Then
                System.Console.WriteLine("This sample is compatible with Windows Vista")        
                bRetVal = False
                Return
            End If

            strDoc = Nothing
            strOption = Nothing
            bRetVal = True
            iRet = 0
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
                            Case "o"
                                If (String.IsNullOrEmpty(strOption) = False) Then
                                    GiveUsage()
                                    bRetVal = False
                                    GoTo ExitFun
                                End If
                                strOption = My.Application.CommandLineArgs.Item(count + 1)
                            Case "d"
                                If (String.IsNullOrEmpty(strDoc) = False) Then
                                    GiveUsage()
                                    bRetVal = False
                                    GoTo ExitFun
                                End If
                                strDoc = My.Application.CommandLineArgs.Item(count + 1)
                            Case "?"
                                GiveUsage()
                                bRetVal = False
                                GoTo ExitFun
                            Case Else
                        End Select
                    End If
                    count = count + 1
                Loop

                If ((String.IsNullOrEmpty(strOption) = True)) Then
                    System.Console.WriteLine("Missing args.")
                    GiveUsage()
                    bRetVal = False
                    GoTo ExitFun
                End If

                If ((String.Compare("sendtofax", strOption.ToLower(CultureInfo.CurrentCulture), True, CultureInfo.CurrentCulture) =0) and (String.IsNullOrEmpty(strDoc) = True)) Then
                    System.Console.WriteLine("Missing args.")
                    GiveUsage()
                    bRetVal = False
                    GoTo ExitFun
                End If

                'if sendtofax option is selected
                If (String.Compare("sendtofax", strOption.ToLower(CultureInfo.CurrentCulture), True, CultureInfo.CurrentCulture) = 0) Then
                    strDoc = strDoc.Replace(";", " ")
                    iRet = SendToFax.SendToFaxRecipient(SendToMode.SendToFaxRecipientAttachment, strDoc.ToCharArray())
                    If (iRet <> 0) Then
                        System.Console.Write("SendToFaxRecipient: failed. Error ")
                        System.Console.Write(iRet)
                        System.Console.WriteLine()
                        bRetVal = False
                        GoTo ExitFun
                    End If
                    System.Console.WriteLine("SendToFaxRecipient was successful")
                End If

                'if cansendtofax option is selected
                If (String.Compare("cansendtofax", strOption.ToLower(CultureInfo.CurrentCulture), True, CultureInfo.CurrentCulture) = 0) Then

                    If (SendToFax.CanSendToFaxRecipient() = False) Then
                        System.Console.WriteLine("CanSendToFaxRecipient: failed. ")
                        bRetVal = False
                    End If
                    System.Console.WriteLine("CanSendToFaxRecipient was successful")
                End If

            Catch excep As Exception
                System.Console.WriteLine("Exception:" + excep.Message)
            End Try
ExitFun:
            If (bRetVal = False) Then
                System.Console.WriteLine("Function Failed")
            End If
        End Sub
    End Module
End Namespace

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
Namespace Microsoft.Samples.Fax.FaxReassign.VB

    Module FaxReassign

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
            System.Console.WriteLine(" /s Fax Server Name ")
            System.Console.WriteLine(" /o <List/Reassign> Message option ")
            System.Console.WriteLine(" /i Message Id. Used if Reassign option ")
            System.Console.WriteLine(" /r Recipients in the form 'domain1\\user1;domain1\\user2' ")
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
        '  function:   IFaxIncomingMessageIterator
        '
        '  Synopsis:   Get the incoming archive folder of the current account
        '
        '  Arguments:  [objFaxFolders] - List of folders for the current account
        '
        '  Returns:    IFaxIncomingMessageIterator: Iterator to the messages in inbox folder
        '
        '----------------------------------------------------------------------------
        Function FaxAccountIncomingArchive(ByVal objFaxFolders As FAXCOMEXLib.IFaxAccountFolders) As FAXCOMEXLib.IFaxIncomingMessageIterator
            If (TypeOf objFaxFolders Is FAXCOMEXLib.IFaxAccountFolders) Then
                Dim NUM_MSGS As Integer
                Dim objFaxInbox As FAXCOMEXLib.IFaxAccountIncomingArchive
                Dim objIncomingMsgIterator As FAXCOMEXLib.IFaxIncomingMessageIterator
                NUM_MSGS = 100

                'Initialize MsgArchive Object
                objFaxInbox = objFaxFolders.IncomingArchive

                'Initialize Msg Iterator
                objIncomingMsgIterator = objFaxInbox.GetMessages(NUM_MSGS)
                Return objIncomingMsgIterator
            End If
            System.Console.WriteLine("FaxAccountIncomingArchive: Parameter is NULL")
            Return Nothing
        End Function

        '+---------------------------------------------------------------------------
        '
        '  function:   hasReassignPermission
        '
        '  Synopsis:   Check if the current user has ReAssign Permission
        '
        '  Arguments:  [objFaxServer] - Fax Server object
        '
        '  Returns:    bool: true if it has reassign permissions
        '
        '  Modifies:
        '
        '----------------------------------------------------------------------------
        Function hasReassignPermission(ByVal objFaxServer As FAXCOMEXLib.FaxServerClass) As Boolean
            If (TypeOf objFaxServer Is FAXCOMEXLib.FaxServerClass) Then
                Dim objFaxSecurity2 As FAXCOMEXLib.IFaxSecurity2
                Dim enumFaxRights As FAXCOMEXLib.FAX_ACCESS_RIGHTS_ENUM_2
                'Get the Security Object
                objFaxSecurity2 = objFaxServer.Security2
                'Get the Access Rights of the user
                enumFaxRights = objFaxSecurity2.GrantedRights
                If ((enumFaxRights And FAXCOMEXLib.FAX_ACCESS_RIGHTS_ENUM_2.far2MANAGE_RECEIVE_FOLDER) = FAXCOMEXLib.FAX_ACCESS_RIGHTS_ENUM_2.far2MANAGE_RECEIVE_FOLDER) Then
                    Return True
                Else
                    Return False
                End If
            End If
            System.Console.WriteLine("hasReassignPermission: Parameter is NULL")
            Return False
        End Function


        '+---------------------------------------------------------------------------
        '
        '  function:   getUnassignedMsg
        '
        '  Synopsis:   Get unassigned msgs
        '
        '  Arguments:  [objIncomingMsgIterator] - Iterator to the messages in inbox folder
        '                [pCount] - Referenced variable containing the number of reassignable faxes.
        '
        '  Returns:    ArrayList: Array of strings containing the mesg ids of reassignable faxes
        '
        '----------------------------------------------------------------------------
        Function getUnassignedMsg(ByVal objIncomingMsgIterator As FAXCOMEXLib.IFaxIncomingMessageIterator, ByRef pCount As Integer) As ArrayList
            If (TypeOf objIncomingMsgIterator Is FAXCOMEXLib.IFaxIncomingMessageIterator) Then
                Dim objIncomingMessage As FAXCOMEXLib.IFaxIncomingMessage
                Dim objIncomingMessage2 As FAXCOMEXLib.IFaxIncomingMessage2
                Dim arrMsgIds As New ArrayList()
                Dim i As Integer

                pCount = -1
                'Get the number of reassignable messages    
                'Goto first Msg
                objIncomingMsgIterator.MoveFirst()
                'Loop thru all msgs
                i = 0
                While (True)
                    If (objIncomingMsgIterator.AtEOF = True) Then
                        Exit While
                    End If
                    objIncomingMessage = objIncomingMsgIterator.Message
                    objIncomingMessage2 = objIncomingMessage
                    'if not reassigned
                    If (objIncomingMessage2.WasReAssigned <> True) Then
                        arrMsgIds.Add(objIncomingMessage2.Id)
                        i = i + 1
                    End If
                    objIncomingMsgIterator.MoveNext()
                End While
                pCount = i
                Return arrMsgIds
            End If
            System.Console.WriteLine("getUnassignedMsg: Parameter is NULL")
            Return Nothing
        End Function

        '+---------------------------------------------------------------------------
        '
        '  function:   Reassign
        '
        '  Synopsis:   Reassign the Msg
        '
        '  Arguments:  [objIncomingMsgIterator] - Iterator to the messages in inbox folder
        '                [strMsgId] - Id of the message to be reassigned
        '                [strRecipients] - Recipients to whom the message is to be assigned.            
        '
        '  Returns:    bool : true if reassign was successful
        '
        '----------------------------------------------------------------------------
        Function Reassign(ByVal objIncomingMsgIterator As FAXCOMEXLib.IFaxIncomingMessageIterator, ByVal strMsgId As String, ByVal strRecipients As String) As Boolean
            If (TypeOf objIncomingMsgIterator Is FAXCOMEXLib.IFaxIncomingMessageIterator) Then

                Dim SENDER_NAME As String
                Dim SENDER_FAXNUMBER As String
                Dim SUBJECT As String
                SENDER_NAME = "ReassignAdmin"
                SENDER_FAXNUMBER = "1234"
                SUBJECT = "Reassigned Fax"

                Dim objIncomingMessage As FAXCOMEXLib.IFaxIncomingMessage
                Dim objIncomingMessage2 As FAXCOMEXLib.IFaxIncomingMessage2
                Dim bRetVal As Boolean
                bRetVal = False
                'Goto first Msg
                objIncomingMsgIterator.MoveFirst()
                While (True)
                    If (objIncomingMsgIterator.AtEOF = True) Then
                        System.Console.WriteLine("Reassign Message Id not found ")
                        Exit While
                    End If
                    'Get current Msg
                    objIncomingMessage = objIncomingMsgIterator.Message
                    objIncomingMessage2 = objIncomingMessage
                    If (String.Compare(objIncomingMessage2.Id, strMsgId, True, CultureInfo.CurrentCulture) = 0) Then
                        'Set the Msg Parameters
                        objIncomingMessage2.Subject = SUBJECT
                        objIncomingMessage2.SenderName = SENDER_NAME
                        objIncomingMessage2.Recipients = strRecipients
                        objIncomingMessage2.SenderFaxNumber = SENDER_FAXNUMBER
                        'Reassign
                        objIncomingMessage2.ReAssign()
                        System.Console.WriteLine("Reassign was successful")
                        bRetVal = True
                        Exit While
                    End If
                    'Next Msg
                    objIncomingMsgIterator.MoveNext()
                End While
                Return bRetVal
            End If
            System.Console.WriteLine("Reassign: Parameter is NULL")
            Return False
        End Function

        Sub Main()
            Dim objFaxServer As FAXCOMEXLib.FaxServer
            Dim objFaxAccount As FAXCOMEXLib.FaxAccount
            Dim objFaxFolders As FAXCOMEXLib.FaxAccountFolders
            Dim objIncomingMessageIterator As FAXCOMEXLib.FaxIncomingMessageIterator
            Dim strServerName As String
            Dim strOption As String
            Dim strRecipient As String
            Dim strMsgId As String
            Dim bConnected As Boolean
            Dim arrFaxMsgIds As System.Collections.ArrayList
            Dim bRetVal As Boolean
            Dim count As Integer
            Dim args As String
            Dim iVista As Integer
            Dim bVersion As Boolean

            iVista = 6
            bVersion = IsOSVersionCompatible(iVista)
            If (bVersion = False) Then
                System.Console.WriteLine("OS Version does not support this feature")
                bRetVal = False
                Return
            End If

           
            strServerName = ""
            objFaxServer = Nothing
            strRecipient = Nothing
            strMsgId = Nothing
            strOption = Nothing
            bRetVal = True
            arrFaxMsgIds = Nothing
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
                            Case "o"
                                If (String.IsNullOrEmpty(strOption) = False) Then
                                    GiveUsage()
                                    bRetVal = False
                                    GoTo ExitFun
                                End If
                                strOption = My.Application.CommandLineArgs.Item(count + 1)
                            Case "r"
                                If (String.IsNullOrEmpty(strRecipient) = False) Then
                                    GiveUsage()
                                    bRetVal = False
                                    GoTo ExitFun
                                End If
                                strRecipient = My.Application.CommandLineArgs.Item(count + 1)
                            Case "i"
                                If (String.IsNullOrEmpty(strMsgId) = False) Then
                                    GiveUsage()
                                    bRetVal = False
                                    GoTo ExitFun
                                End If
                                strMsgId = My.Application.CommandLineArgs.Item(count + 1)
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

                If (((strMsgId = Nothing) Or (strRecipient = Nothing)) And (String.Compare("list", strOption.ToLower(CultureInfo.CurrentCulture), True, CultureInfo.CurrentCulture) <> 0)) Then
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
                objFaxAccount = objFaxServer.CurrentAccount
                'Now that we have got the account object lets get the folders object
                objFaxFolders = objFaxAccount.Folders

                'if reassign message option is selected
                If (String.Compare("reassign", strOption.ToLower(CultureInfo.CurrentCulture), True, CultureInfo.CurrentCulture) = 0) Then
                    If (hasReassignPermission(objFaxServer) = True) Then
                        objIncomingMessageIterator = FaxAccountIncomingArchive(objFaxFolders)
                        If (TypeOf objIncomingMessageIterator Is FAXCOMEXLib.IFaxIncomingMessageIterator) Then
                            If (Reassign(objIncomingMessageIterator, strMsgId, strRecipient) = False) Then
                                'we dont want to log any error here as the error will be logged in the function itself
                                bRetVal = False
                            Else
                                'we dont want to log any error here as the error will be logged in the function itself
                                bRetVal = True
                            End If
                        End If
                    Else
                        System.Console.WriteLine("User doesn't have reassign permission")
                        bRetVal = False
                    End If
                End If

                'if list message ids option is selected
                If (String.Compare("list", strOption.ToLower(CultureInfo.CurrentCulture), True, CultureInfo.CurrentCulture) = 0) Then
                    If (hasReassignPermission(objFaxServer) = True) Then
                        objIncomingMessageIterator = FaxAccountIncomingArchive(objFaxFolders)
                        If (TypeOf objIncomingMessageIterator Is FAXCOMEXLib.IFaxIncomingMessageIterator) Then
                            arrFaxMsgIds = getUnassignedMsg(objIncomingMessageIterator, count)
                            If (TypeOf arrFaxMsgIds Is System.Collections.ArrayList) Then
                                System.Console.WriteLine("Printing Msg Ids of reassignable faxes")
                                For i As Integer = 0 To (count - 1)
                                    System.Console.Write("Msg Id of Message Number ")
                                    System.Console.Write(i)
                                    System.Console.Write(" is ")
                                    System.Console.Write(arrFaxMsgIds(i))
                                    System.Console.WriteLine()
                                Next i
                            Else
                                System.Console.WriteLine("No reassignable faxes present")
                            End If
                        Else
                            'we dont want to log any error here as the error will be logged in the function itself
                            bRetVal = False
                        End If
                    Else
                        System.Console.WriteLine("User doesn't have reassign permission")
                        bRetVal = False
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
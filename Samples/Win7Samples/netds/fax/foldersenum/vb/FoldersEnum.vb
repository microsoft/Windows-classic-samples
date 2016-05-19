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
Imports FAXCOMEXLib


<Assembly: System.Reflection.AssemblyKeyFile("key.snk")> 
<Assembly: CLSCompliant(True)> 
Namespace Microsoft.Samples.Fax.FoldersEnum.VB
    Module FoldersEnum

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
            System.Console.WriteLine(" /o <EnumInbox>/<EnumOutbox>/<EnumSentItems>/<EnumIncoming> ")
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
        '  function:   EnumerateFaxOutgoingJobs
        '
        '  Synopsis:   Enumerates the Fax Jobs in the Outbox folder
        '
        '  Arguments:  [objFaxOutgoingJobs] - Fax Outgoing Jobs Object
        '
        '  Returns:    bool - true if the function was successful
        '
        '----------------------------------------------------------------------------
        Function EnumerateFaxOutgoingJobs(ByVal objFaxOutgoingJobs As IFaxOutgoingJobs) As Boolean
            'check for NULL
            If (TypeOf objFaxOutgoingJobs Is FAXCOMEXLib.IFaxOutgoingJobs) Then
               
                Dim objEnumerator As IEnumerator
                Dim objFaxOutgoingJob As IFaxOutgoingJob2

                objEnumerator = objFaxOutgoingJobs.GetEnumerator()
                objEnumerator.Reset()
                System.Console.WriteLine("Enumerating Outgoing Jobs ...")

                While (objEnumerator.MoveNext())

                    objFaxOutgoingJob = objEnumerator.Current
                    System.Console.WriteLine("Outgoing Job Id: " + objFaxOutgoingJob.Id)
                End While
                Return True
            Else
                System.Console.WriteLine("EnumerateFaxOutgoingJobs: Parameter passed is NULL")
                Return False
            End If
        End Function
        '+---------------------------------------------------------------------------
        '
        '  function:   EnumerateFaxIncomingJobs
        '
        '  Synopsis:   Enumerates the Fax Jobs in the Incoming folder
        '
        '  Arguments:  [objFaxIncomingJobs] - Fax Incoming Jobs Object
        '
        '  Returns:    bool - true if the function was successful
        '
        '----------------------------------------------------------------------------

        Function EnumerateFaxIncomingJobs(ByVal objFaxIncomingJobs As IFaxIncomingJobs) As Boolean
            'check for NULL
            If (TypeOf objFaxIncomingJobs Is FAXCOMEXLib.IFaxIncomingJobs) Then
                Dim objEnumerator As IEnumerator
                Dim objFaxIncomingJob As IFaxIncomingJob
                objEnumerator = objFaxIncomingJobs.GetEnumerator()
                objEnumerator.Reset()
                System.Console.WriteLine("Enumerating Incoming Jobs ...")
                While (objEnumerator.MoveNext())

                    objFaxIncomingJob = objEnumerator.Current
                    System.Console.WriteLine("Incoming Job Id: " + objFaxIncomingJob.Id)
                End While
                Return True
            Else
                System.Console.WriteLine("EnumerateFaxIncomingJobs: Parameter passed is NULL")
                Return False
            End If
        End Function
        '+---------------------------------------------------------------------------
        '
        '  function:   EnumerateFaxInboxMessages
        '
        '  Synopsis:   Enumerates the Fax Messages in the Inbox folder
        '
        '  Arguments:  [objIncomingMsgIterator] - Fax Incoming Message Iterator Object
        '
        '  Returns:    bool - true if the function was successful
        '
        '----------------------------------------------------------------------------

        Function EnumerateFaxInboxMessages(ByVal objIncomingMsgIterator As IFaxIncomingMessageIterator) As Boolean
            If (TypeOf objIncomingMsgIterator Is FAXCOMEXLib.IFaxIncomingMessageIterator) Then
                System.Console.WriteLine("Enumerating Inbox Messages ...")
                objIncomingMsgIterator.MoveFirst()
                While (objIncomingMsgIterator.AtEOF = False)
                    Dim objFaxIncomingMsg As IFaxIncomingMessage2
                    objFaxIncomingMsg = objIncomingMsgIterator.Message
                    System.Console.WriteLine("Inbox Msg Id: " + objFaxIncomingMsg.Id)
                    objIncomingMsgIterator.MoveNext()
                End While
                Return True
            Else
                System.Console.WriteLine("EnumerateFaxInboxMessages: Parameter passed is NULL")
                Return False
            End If
        End Function
        '+---------------------------------------------------------------------------
        '
        '  function:   EnumerateFaxSentItemMessages
        '
        '  Synopsis:   Enumerates the Fax Messages in the Sent Items folder
        '
        '  Arguments:  [objOutgoingMsgIterator] - Fax SentItems Message Iterator Object
        '
        '  Returns:    bool - true if the function was successful
        '
        '----------------------------------------------------------------------------
        Function EnumerateFaxSentItemMessages(ByVal objOutgoingMsgIterator As IFaxOutgoingMessageIterator) As Boolean
            If (TypeOf objOutgoingMsgIterator Is FAXCOMEXLib.IFaxOutgoingMessageIterator) Then
                System.Console.WriteLine("Enumerating SentItems Messages ...")
                objOutgoingMsgIterator.MoveFirst()
                While (objOutgoingMsgIterator.AtEOF = False)
                    Dim objFaxOutgoingMsg As IFaxOutgoingMessage2
                    objFaxOutgoingMsg = objOutgoingMsgIterator.Message
                    System.Console.WriteLine("SentItems Msg Id: " + objFaxOutgoingMsg.Id)
                    objOutgoingMsgIterator.MoveNext()
                End While
                Return True
            Else
                System.Console.WriteLine("EnumerateFaxSentItemMessages: Parameter passed is NULL")
                Return False
            End If
        End Function
        '+---------------------------------------------------------------------------
        '
        '  function:   EnumInbox
        '
        '  Synopsis:   Displays the messages present in the inbox
        '
        '  Arguments:  [objFaxFolders] - Fax Folders object
        '
        '  Returns:    bool - true if the function was successful
        '
        '----------------------------------------------------------------------------
        Function EnumInbox(ByVal objFaxFolders As IFaxAccountFolders) As Boolean
            If (TypeOf objFaxFolders Is FAXCOMEXLib.IFaxAccountFolders) Then
                Dim objFaxInbox As IFaxAccountIncomingArchive
                objFaxInbox = objFaxFolders.IncomingArchive

                Dim objIncomingMsgIterator As IFaxIncomingMessageIterator
                objIncomingMsgIterator = objFaxInbox.GetMessages(100)
                If (EnumerateFaxInboxMessages(objIncomingMsgIterator) = False) Then
                    System.Console.WriteLine("Failed to enumerate ")
                    Return False
                End If
                Return True
            Else
                System.Console.WriteLine("EnumInbox: Parameter passed is NULL")
                Return False
            End If
        End Function


        '+---------------------------------------------------------------------------
        '
        '  function:   EnumSentItems
        '
        '  Synopsis:   Displays the messages present in the Sent Items
        '
        '  Arguments:  [objFaxFolders] - Fax Folders object
        '
        '  Returns:    bool - true if the function was successful
        '
        '----------------------------------------------------------------------------
        Function EnumSentItems(ByVal objFaxFolders As IFaxAccountFolders) As Boolean
            If (TypeOf objFaxFolders Is FAXCOMEXLib.IFaxAccountFolders) Then
            
                Dim objFaxOutbox As IFaxAccountOutgoingArchive
                objFaxOutbox = objFaxFolders.OutgoingArchive

                Dim objOutgoingMsgIterator As IFaxOutgoingMessageIterator
                objOutgoingMsgIterator = objFaxOutbox.GetMessages(100)

                If (EnumerateFaxSentItemMessages(objOutgoingMsgIterator) = False) Then
                    System.Console.WriteLine("Failed to enumerate ")
                    Return False
                End If
                Return True
            Else
                System.Console.WriteLine("EnumSentItems: Parameter passed is NULL")
                Return False
            End If
        End Function
        '+---------------------------------------------------------------------------
        '
        '  function:   EnumIncoming
        '
        '  Synopsis:   Displays the jobs present in the Incoming Folder
        '
        '  Arguments:  [objFaxFolders] - Fax Folders object
        '
        '  Returns:    bool - true if the function was successful
        '
        '----------------------------------------------------------------------------
        Function EnumIncoming(ByVal objFaxFolders As IFaxAccountFolders) As Boolean
            If (TypeOf objFaxFolders Is FAXCOMEXLib.IFaxAccountFolders) Then
                
                Dim objFaxIncomingQueue As IFaxAccountIncomingQueue
                objFaxIncomingQueue = objFaxFolders.IncomingQueue

                Dim objFaxIncomingJobs As IFaxIncomingJobs
                objFaxIncomingJobs = objFaxIncomingQueue.GetJobs()

                If (EnumerateFaxIncomingJobs(objFaxIncomingJobs) = False) Then
                    System.Console.WriteLine("Failed to enumerate ")
                    Return False
                End If
                Return True
            Else
                System.Console.WriteLine("EnumIncoming: Parameter passed is NULL")
                Return False
            End If
        End Function

        '+---------------------------------------------------------------------------
        '
        '  function:   EnumOutbox
        '
        '  Synopsis:   Displays the jobs present in the Outbox Folder
        '
        '  Arguments:  [objFaxFolders] - Fax Folders object
        '
        '  Returns:    bool - true if the function was successful
        '
        '----------------------------------------------------------------------------

        Function EnumOutbox(ByVal objFaxFolders As IFaxAccountFolders) As Boolean
            If (TypeOf objFaxFolders Is FAXCOMEXLib.IFaxAccountFolders) Then
               
                Dim objFaxOutgoingQueue As IFaxAccountOutgoingQueue
                objFaxOutgoingQueue = objFaxFolders.OutgoingQueue
                Dim objFaxOutgoingJobs As IFaxOutgoingJobs
                objFaxOutgoingJobs = objFaxOutgoingQueue.GetJobs()
                If (EnumerateFaxOutgoingJobs(objFaxOutgoingJobs) = False) Then
                    System.Console.WriteLine("Failed to enumerate ")
                    Return False
                End If
                Return True
            Else
                System.Console.WriteLine("EnumOutbox: Parameter passed is NULL")
                Return False
            End If
        End Function



        Sub Main()
            Dim objFaxServer As FAXCOMEXLib.FaxServer
            Dim objFaxAccount As FAXCOMEXLib.IFaxAccount
            Dim objFaxAccFolders As FAXCOMEXLib.IFaxAccountFolders

            Dim strServerName As String
            Dim strOption As String

            Dim bConnected As Boolean
            Dim bRetVal As Boolean
            Dim args As String
            Dim count As Integer

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
                Do Until count = My.Application.CommandLineArgs.Count -1
                    If count >= My.Application.CommandLineArgs.Count -1 Then
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
                            Case "?"
                                GiveUsage()
                                bRetVal = False
                                GoTo ExitFun
                            Case Else
                        End Select
                    End If
                    count = count + 1
                Loop

                If (String.IsNullOrEmpty(strOption) = True) Then
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

                objFaxAccount = objFaxServer.CurrentAccount            
                objFaxAccFolders = objFaxAccount.Folders
                
                If (String.Compare("enuminbox", strOption.ToLower(CultureInfo.CurrentCulture), True, CultureInfo.CurrentCulture) = 0) Then
                    If (EnumInbox(objFaxAccFolders) = False) Then
                        System.Console.WriteLine("EnumInbox Failed")
                        bRetVal = False
                    End If
                End If

                If (String.Compare("enumoutbox", strOption.ToLower(CultureInfo.CurrentCulture), True, CultureInfo.CurrentCulture) = 0) Then
                    If (EnumOutbox(objFaxAccFolders) = False) Then
                        System.Console.WriteLine("EnumOutbox Failed")
                        bRetVal = False
                    End If
                End If

                If (String.Compare("enumincoming", strOption.ToLower(CultureInfo.CurrentCulture), True, CultureInfo.CurrentCulture) = 0) Then
                    If (EnumIncoming(objFaxAccFolders) = False) Then
                        System.Console.WriteLine("EnumIncoming Failed")
                        bRetVal = False
                    End If
                End If

                If (String.Compare("enumsentitems", strOption.ToLower(CultureInfo.CurrentCulture), True, CultureInfo.CurrentCulture) = 0) Then
                    If (EnumSentItems(objFaxAccFolders) = False) Then
                        System.Console.WriteLine("EnumSentItems Failed")
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
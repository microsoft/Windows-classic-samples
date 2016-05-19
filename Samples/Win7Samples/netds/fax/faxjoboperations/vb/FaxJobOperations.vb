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
Namespace Microsoft.Samples.Fax.FaxJobOperations.VB
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
        Function EnumerateFaxOutgoingJobs(ByVal objFaxOutgoingJobs As IFaxOutgoingJobs, ByVal bCancelJob As Boolean, ByVal strJobId As String) As Boolean
            'check for Jobid
            If (bCancelJob = True And String.IsNullOrEmpty(strJobId) = True) Then
                System.Console.WriteLine("EnumerateFaxOutgoingJobs: Parameter passed is NULL")
                Return False
            End If
            'check for NULL
            If (TypeOf objFaxOutgoingJobs Is FAXCOMEXLib.IFaxOutgoingJobs) Then

                Dim objEnumerator As IEnumerator
                Dim objFaxOutgoingJob As IFaxOutgoingJob2
                objEnumerator = objFaxOutgoingJobs.GetEnumerator()
                objEnumerator.Reset()

                While (objEnumerator.MoveNext())
                    objFaxOutgoingJob = objEnumerator.Current
                    If (bCancelJob = True) Then
                        If (String.Compare(objFaxOutgoingJob.Id, strJobId, True, CultureInfo.CurrentCulture) = 0) Then
                            objFaxOutgoingJob.Cancel()
                            System.Console.WriteLine("Job cancelled successfully")
                            Return True
                        End If
                    Else
                        System.Console.Write("Outgoing Job Id: " + objFaxOutgoingJob.Id)
                        System.Console.Write(" Subject: " + objFaxOutgoingJob.Subject)
                        System.Console.Write(" SenderName: " + objFaxOutgoingJob.Sender.Name)
                        System.Console.Write(" Submission Id: " + objFaxOutgoingJob.SubmissionId)
                        System.Console.WriteLine("")
                    End If
                End While
                If (bCancelJob = False) Then
                    Return True
                Else
                    Return False
                End If
            Else
                System.Console.WriteLine("EnumerateFaxOutgoingJobs: Parameter passed is NULL")
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

        Function EnumOutbox(ByVal objFaxOutgoingQueue As IFaxOutgoingQueue) As Boolean
            If (TypeOf objFaxOutgoingQueue Is FAXCOMEXLib.IFaxOutgoingQueue) Then
                Dim objFaxOutgoingJobs As IFaxOutgoingJobs
                objFaxOutgoingJobs = objFaxOutgoingQueue.GetJobs()
                If (EnumerateFaxOutgoingJobs(objFaxOutgoingJobs, False, String.Empty) = False) Then
                    System.Console.WriteLine("Failed to enumerate ")
                    Return False
                End If
                Return True
            Else
                System.Console.WriteLine("EnumOutbox: Parameter passed is NULL")
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

        Function CancelJob(ByVal objFaxOutgoingQueue As IFaxOutgoingQueue, ByVal strJobId As String) As Boolean
            If (TypeOf objFaxOutgoingQueue Is FAXCOMEXLib.IFaxOutgoingQueue) Then
                Dim objFaxOutgoingJobs As IFaxOutgoingJobs
                objFaxOutgoingJobs = objFaxOutgoingQueue.GetJobs()
                If (EnumerateFaxOutgoingJobs(objFaxOutgoingJobs, True, strJobId) = False) Then
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
            Dim objFaxOutgoingQueue As FAXCOMEXLib.IFaxOutgoingQueue
            Dim strServerName As String

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
            bRetVal = True
            bConnected = False
            count = 0

            Try
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
                            Case "?"
                                GiveUsage()
                                bRetVal = False
                                GoTo ExitFun
                            Case Else
                        End Select
                    End If
                    count = count + 1
                Loop

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

                objFaxOutgoingQueue = objFaxServer.Folders.OutgoingQueue

                Dim bQuit As Boolean
                Dim strJobId As String
                Dim cOption As Char
                Dim strChar As String
                cOption = "c"
                strJobId = Nothing
                bQuit = False

                While (bQuit = False)
                    System.Console.WriteLine()
                    objFaxOutgoingQueue.Blocked = True
                    objFaxOutgoingQueue.Paused = True
                    objFaxOutgoingQueue.Save()
                    System.Console.WriteLine("Outgoing Queue is paused. ")
                    System.Console.WriteLine("Outgoing Queue is blocked. ")
                    'Print all outgoing jobs
                    System.Console.WriteLine("Printing list of Outgoing jobs ...")

                    If (EnumOutbox(objFaxOutgoingQueue) = False) Then
                        System.Console.WriteLine("Failed to enumerate")
                        bRetVal = False
                    End If
                    System.Console.WriteLine("Enter 'c' to cancel a job ")
                    System.Console.WriteLine("Enter 'q' to quit ")
                    strChar = System.Console.ReadLine()
                    strChar.Trim()
                    cOption = (strChar.ToLower(CultureInfo.CurrentCulture).Substring(0, 1))
input:
                    Select Case (cOption)
                        Case "c"
                            System.Console.WriteLine("Enter 'i' to enter Job id ")
                            System.Console.WriteLine("Enter 'q' to quit ")
                            strChar = System.Console.ReadLine()
                            strChar.Trim()
                            cOption = (strChar.ToLower(CultureInfo.CurrentCulture).Substring(0, 1))

input2:                     Select Case (cOption)
                                Case "i"
                                    System.Console.WriteLine("Enter Job id ")
                                    strJobId = System.Console.ReadLine()
                                    strJobId.Trim()
                                    System.Console.Write("Job to be cancelled: ")
                                    System.Console.WriteLine(strJobId)
                                    CancelJob(objFaxOutgoingQueue, strJobId)
                                Case "q"
                                    GoTo quit
                                Case Else
                                    System.Console.WriteLine("Invalid Option. Enter cancel option again ")
                                      strChar = System.Console.ReadLine()
                                    strChar.Trim()
                                    cOption = (strChar.ToLower(CultureInfo.CurrentCulture).Substring(0, 1))
                                    GoTo input2
                            End Select
                        Case "q"
quit:                       bQuit = True
                            Exit While
                        Case Else
                            System.Console.WriteLine("Invalid Option. Enter again ")
                            System.Console.WriteLine("Invalid Option. Enter cancel option again ")
                              strChar = System.Console.ReadLine()
                            strChar.Trim()
                            cOption = (strChar.ToLower(CultureInfo.CurrentCulture).Substring(0, 1))
                            GoTo input
                    End Select
                End While
                'unblock queue
                objFaxOutgoingQueue.Paused = False
                objFaxOutgoingQueue.Blocked = False
                objFaxOutgoingQueue.Save()
                System.Console.WriteLine("Outgoing Queue is resumed. ")
                System.Console.WriteLine("Outgoing Queue is unblocked. ")
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
'==========================================================================
'
'  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
'  KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
'  IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
'  PURPOSE.
'
'  Copyright 1998 - 2000 Microsoft Corporation.  All Rights Reserved.
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
Namespace Microsoft.Samples.Fax.SendFax.VB
    Module SendFax
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
            System.Console.WriteLine(" /d DocumentPath (can have multiple documents separated by semicolons. test1.txt;test2.doc ")
            System.Console.WriteLine(" /n Fax Number ")
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
        '  function:   PrintJobStatus
        '
        '  Synopsis:   prints the jobs status
        '
        '  Arguments:  [objFaxOutgoingJob] - FaxOutgoingJob object pointing to the fax that was sent. 
        '
        '  Returns:    bool: true is passed successfully
        '
        '----------------------------------------------------------------------------
        Function PrintJobStatus(ByVal objFaxOutgoingJob As FAXCOMEXLib.IFaxOutgoingJob2) As Boolean

            Dim bRetVal As Boolean
            Dim iDeviceId As Integer
            Dim faxStatus As FAXCOMEXLib.FAX_JOB_STATUS_ENUM
            Dim faxPriority As FAXCOMEXLib.FAX_PRIORITY_TYPE_ENUM

            If (TypeOf objFaxOutgoingJob Is FAXCOMEXLib.IFaxOutgoingJob2) Then
                bRetVal = False
                iDeviceId = -1

                iDeviceId = objFaxOutgoingJob.DeviceId
                System.Console.Write("Device Id : ")
                System.Console.Write(iDeviceId)
                System.Console.WriteLine()

                faxStatus = objFaxOutgoingJob.Status
                If (faxStatus = FAXCOMEXLib.FAX_JOB_STATUS_ENUM.fjsCANCELED) Then
                    System.Console.WriteLine("Status :  Canceled ")
                End If
                If (faxStatus = FAXCOMEXLib.FAX_JOB_STATUS_ENUM.fjsCANCELING) Then
                    System.Console.WriteLine("Status :  Canceling ")
                End If
                If (faxStatus = FAXCOMEXLib.FAX_JOB_STATUS_ENUM.fjsCOMPLETED) Then
                    System.Console.WriteLine("Status :  Completed ")
                End If
                If (faxStatus = FAXCOMEXLib.FAX_JOB_STATUS_ENUM.fjsFAILED) Then
                    System.Console.WriteLine("Status :  Failed ")
                End If
                If (faxStatus = FAXCOMEXLib.FAX_JOB_STATUS_ENUM.fjsINPROGRESS) Then
                    System.Console.WriteLine("Status :  In Progress ")
                End If
                If (faxStatus = FAXCOMEXLib.FAX_JOB_STATUS_ENUM.fjsNOLINE) Then
                    System.Console.WriteLine("Status :  No Line ")
                End If
                If (faxStatus = FAXCOMEXLib.FAX_JOB_STATUS_ENUM.fjsPAUSED) Then
                    System.Console.WriteLine("Status :  Paused ")
                End If
                If (faxStatus = FAXCOMEXLib.FAX_JOB_STATUS_ENUM.fjsPENDING) Then
                    System.Console.WriteLine("Status :  Pending ")
                End If
                If (faxStatus = FAXCOMEXLib.FAX_JOB_STATUS_ENUM.fjsRETRIES_EXCEEDED) Then
                    System.Console.WriteLine("Status :  Retries Exceeded ")
                End If
                If (faxStatus = FAXCOMEXLib.FAX_JOB_STATUS_ENUM.fjsRETRYING) Then
                    System.Console.WriteLine("Status :  Retrying ")
                End If
                If (faxStatus = FAXCOMEXLib.FAX_JOB_STATUS_ENUM.fjsROUTING) Then
                    System.Console.WriteLine("Status :  Routing ")
                End If

                faxPriority = objFaxOutgoingJob.Priority
                If (faxPriority = FAX_PRIORITY_TYPE_ENUM.fptLOW) Then
                    System.Console.WriteLine("Priority :  Low ")
                End If
                If (faxPriority = FAX_PRIORITY_TYPE_ENUM.fptNORMAL) Then
                    System.Console.WriteLine("Priority :  Normal ")
                End If
                If (faxPriority = FAX_PRIORITY_TYPE_ENUM.fptHIGH) Then
                    System.Console.WriteLine("Priority :  High ")
                End If

                bRetVal = True
                Return bRetVal
            End If
            System.Console.WriteLine("PrintJobStatus: Parameter is NULL")
            Return False
        End Function
        '+---------------------------------------------------------------------------
        '
        '  function:   DecodeToDocArray
        '
        '  Synopsis:   Creates a string Array of Docs from the inputDocListString
        '
        '  Arguments:  [inputDocListString] - The list of documents in string format separated by semicolon
        '              [numDocuments] -    The number of documents to be sent
        '                [bRetVal] - true is passed successfully 
        '
        '  Returns:    string[]: Array of strings each containing a single document
        '
        '----------------------------------------------------------------------------
 
        Function DecodeToDocArray(ByVal inputDocListString As String, ByRef numDocuments As Integer, ByRef bRetVal As Boolean) As String()
            bRetVal = False
            If (String.IsNullOrEmpty(inputDocListString)) Then
                Return Nothing
            End If
            Dim docArray As String()
            Dim strDelimiter As String
            Dim delimiter As Char()

            docArray = Nothing
            strDelimiter = ";"
            delimiter = strDelimiter.ToCharArray()
            docArray = inputDocListString.Split(delimiter)
            numDocuments = docArray.GetLength(0)
            bRetVal = True
            Return docArray
        End Function
        Sub Main()
            Dim objFaxServer As FAXCOMEXLib.FaxServer
            Dim objFaxDoc As FaxDocument
            Dim objFaxOutgoingJob2 As IFaxOutgoingJob2

            Dim strServerName As String
            Dim strDocList As String
            Dim strNumber As String
            Dim bConnected As Boolean
            Dim bRetVal As Boolean
            Dim count As Integer
            Dim args As String
            Dim retVal As Boolean
            Dim numDocs As Integer
            Dim docArray As String()

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
            strNumber = Nothing
            strDocList = Nothing
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
                            Case "d"
                                If (String.IsNullOrEmpty(strDocList) = False) Then
                                    GiveUsage()
                                    bRetVal = False
                                    GoTo ExitFun
                                End If
                                strDocList = My.Application.CommandLineArgs.Item(count + 1)
                            Case "n"
                                If (String.IsNullOrEmpty(strNumber) = False) Then
                                    GiveUsage()
                                    bRetVal = False
                                    GoTo ExitFun
                                End If
                                strNumber = My.Application.CommandLineArgs.Item(count + 1)
                            Case "?"
                                GiveUsage()
                                bRetVal = False
                                GoTo ExitFun
                            Case Else
                        End Select
                    End If
                    count = count + 1
                Loop

                If ((String.IsNullOrEmpty(strDocList) = True) Or (String.IsNullOrEmpty(strNumber) = True)) Then
                    System.Console.WriteLine("Missing args.")
                    GiveUsage()
                    bRetVal = False
                    GoTo ExitFun
                End If

                objFaxServer = New FaxServer
                'Connect to Fax Server
                objFaxServer.Connect(strServerName)
                bConnected = True

                'check API version
                If (objFaxServer.APIVersion < FAXCOMEXLib.FAX_SERVER_APIVERSION_ENUM.fsAPI_VERSION_3) Then
                    bRetVal = False
                    System.Console.WriteLine("This sample is compatible with Windows Vista")
                    GoTo ExitFun
                End If

                retVal = False
                numDocs = 0
                docArray = DecodeToDocArray(strDocList, numDocs, retVal)
                If ((docArray.GetLength(0) = 0) Or (retVal = False)) Then
                    System.Console.WriteLine("DecodeToDocArray failed")
                    bRetVal = False
                    GoTo ExitFun
                End If

                objFaxDoc = New FaxDocumentClass()
                objFaxDoc.Bodies = docArray
                objFaxDoc.Sender.LoadDefaultSender()
                objFaxDoc.Recipients.Add(strNumber, "TestUser")
                Dim strJobIds As Object
                strJobIds = Nothing
                Dim iErrorIndex As Integer

                iErrorIndex = objFaxDoc.ConnectedSubmit2(objFaxServer, strJobIds)
                If (iErrorIndex <> -1) Then
                    System.Console.Write("ConnectedSubmit2 failed ErrorIndex = ")
                    System.Console.Write(iErrorIndex)
                    System.Console.WriteLine()
                    bRetVal = False
                    GoTo ExitFun
                End If
                Dim strArrJobIds As String()
                strArrJobIds = strJobIds
                System.Console.Write("Job ID : ")
                System.Console.Write(strArrJobIds(0))
                System.Console.WriteLine()
                objFaxOutgoingJob2 = objFaxServer.Folders.OutgoingQueue.GetJob(strArrJobIds(0))
                If (PrintJobStatus(objFaxOutgoingJob2) = False) Then
                    System.Console.WriteLine("PrintJobStatus failed.")
                    bRetVal = False
                    GoTo ExitFun
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
'
' Copyright (c) 2006 Microsoft Corporation. All rights reserved.
' 
' THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
' ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
' THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
' PARTICULAR PURPOSE.
'

Imports System
Imports System.Collections.Generic
Imports Win32Exception = System.ComponentModel.Win32Exception
Imports System.Diagnostics
Imports System.Security.Permissions

'Windows PowerShell namespace
Imports System.Management.Automation
Imports System.ComponentModel

Namespace Microsoft.Samples.PowerShell.Commands

    ' This sample is a complete implementation of the get-proc Cmdlet.
#Region "GetProcCommand"

    ''' <summary>
    ''' This class implements the get-proc cmdlet
    ''' </summary>
    <Cmdlet(VerbsCommon.Get, "Proc", _
    DefaultParameterSetName:="ProcessName")> _
    Public Class GetProcCommand
        Inherits PSCmdlet
#Region "Parameters"

        ''' <summary>
        ''' The list of process names on which this cmdlet will work
        ''' </summary>
        <Parameter(Position:=0, ParameterSetName:="ProcessName", _
        ValueFromPipeline:=True, _
        ValueFromPipelineByPropertyName:=True), _
        ValidateNotNullOrEmpty()> _
        Public Property Name() As String()
            Get
                Return processNames
            End Get

            Set(ByVal value As String())
                processNames = value
            End Set

        End Property

        ''' <summary>
        ''' gets/sets an array of process IDs
        ''' </summary>
        <Parameter(ParameterSetName:="Id", _
        Mandatory:=True, ValueFromPipeline:=True, _
        ValueFromPipelineByPropertyName:=True, _
        HelpMessage:="The unique id of the process to get.")> _
        Public Property Id() As Integer()
            Get
                Return processIds
            End Get
            Set(ByVal value As Integer())
                processIds = value
            End Set
        End Property

        ''' <summary>
        ''' If the input is a stream of [collections of] Process 
        ''' objects, we bypass the ProcessName and Id parameters and 
        ''' read the Process objects directly.  This allows us to deal 
        ''' with processes which have wildcard characters in their name.
        ''' <value>Process objects</value>
        ''' </summary>
        <Parameter(ParameterSetName:="InputObject", _
        Mandatory:=True, ValueFromPipeline:=True)> _
        Public Property Input() As Process()
            Get
                Return inputObjects
            End Get
            Set(ByVal value As Process())
                inputObjects = value
            End Set
        End Property

#End Region

#Region "Cmdlet Overrides"

        ''' <summary>
        ''' For each of the requested processnames, retrieve and write
        ''' the associated processes.
        ''' </summary>
        Protected Overrides Sub ProcessRecord()
            Dim matchingProcesses As List(Of Process)

            WriteDebug("Obtaining list of matching process objects.")

            Select Case ParameterSetName
                Case "Id"
                    matchingProcesses = GetMatchingProcessesById()
                Case "ProcessName"
                    matchingProcesses = GetMatchingProcessesByName()
                Case "InputObject"
                    matchingProcesses = GetProcessesByInput()
                Case Else
                    ThrowTerminatingError(New ErrorRecord( _
                        New ArgumentException("Bad ParameterSetName"), _
                            "UnableToAccessProcessList", _
                            ErrorCategory.InvalidOperation, Nothing))
                    Return
            End Select

            WriteDebug("Outputting matching process objects.")

            matchingProcesses.Sort(AddressOf ProcessComparison)

            Dim process As Process
            For Each process In matchingProcesses
                WriteObject(process)
            Next process

        End Sub 'ProcessRecord

#End Region

#Region "protected Methods and Data"
        ''' <summary>
        ''' Retrieves the list of all processes matching the ProcessName
        ''' parameter.
        ''' Generates a non-terminating error for each specified
        ''' process name which is not found even though it contains
        ''' no wildcards.
        ''' </summary>
        ''' <returns></returns>

        Private Function GetMatchingProcessesByName() As List(Of Process)

            Dim allProcesses As List(Of Process) = _
                New List(Of Process)(Process.GetProcesses())

            ' The keys dictionary will be used for rapid lookup of 
            ' processes already in the matchingProcesses list.
            Dim keys As Dictionary(Of Integer, Byte) = _
                New Dictionary(Of Integer, Byte)()

            Dim matchingProcesses As List(Of Process) = New List(Of Process)()

            If Nothing Is processNames Then
                matchingProcesses.AddRange(allProcesses)
            Else
                Dim pattern As String
                For Each pattern In processNames
                    WriteVerbose(("Finding matches for process name """ & _
                        pattern & """."))

                    ' WildCard serach on the available processes
                    Dim wildcard As New WildcardPattern(pattern, _
                        WildcardOptions.IgnoreCase)

                    Dim found As Boolean = False

                    Dim process As Process
                    For Each process In allProcesses
                        If Not keys.ContainsKey(process.Id) Then
                            Dim processName As String = _
                                SafeGetProcessName(process)

                            ' Remove the process from the allProcesses list 
                            ' so that it's not tested again.
                            If processName.Length = 0 Then
                                allProcesses.Remove(process)
                            End If

                            ' Perform a wildcard search on this particular 
                            ' process and check whehter this matches the 
                            ' pattern specified.
                            If Not wildcard.IsMatch(processName) Then
                                GoTo ContinueForEach2
                            End If

                            WriteDebug(String.Format( _
                                "Found matching process id ""{0}"".", process.Id))

                            ' We have found a match.
                            found = True

                            ' Store the process ID so that we don't add the 
                            ' same one twice.
                            keys.Add(process.Id, 0)

                            ' Add the process to the processes list.
                            matchingProcesses.Add(process)
                        End If
ContinueForEach2:
                    Next process ' foreach (Process...
                    If Not found AndAlso Not _
                       WildcardPattern.ContainsWildcardCharacters(pattern) _
                    Then
                        WriteError(New ErrorRecord( _
                            New ArgumentException("Cannot find process name " & _
                            " " & pattern & "."), "ProcessNameNotFound", _
                            ErrorCategory.ObjectNotFound, pattern))
                    End If
                Next pattern
            End If
            Return matchingProcesses

        End Function 'GetMatchingProcessesByName

        ''' <summary>
        ''' Returns the name of a process.  If an error occurs, a blank
        ''' string will be returned.
        ''' </summary>
        ''' <param name="process">The process whose name will be 
        ''' returned.</param>
        ''' <returns>The name of the process.</returns>
        Protected Shared Function SafeGetProcessName(ByVal process As Process) _
            As String

            Dim name As String = ""

            If Not (process Is Nothing) Then
                Try
                    Return process.ProcessName
                Catch e1 As Win32Exception
                Catch e2 As InvalidOperationException
                End Try
            End If
            Return name


        End Function 'SafeGetProcessName

#End Region


#Region "Private Methods"

        ''' <summary>
        ''' Function to sort by ProcessName first, then by Id
        ''' </summary>
        ''' <param name="x">first Process object</param>
        ''' <param name="y">second Process object</param>
        ''' <returns>
        ''' returns less than zero if x less than y,
        ''' greater than 0 if x greater than y, 0 if x == y
        ''' </returns>
        Private Shared Function ProcessComparison(ByVal x As Process, _
                ByVal y As Process) As Integer
            Dim diff As Integer = String.Compare(SafeGetProcessName(x), _
            SafeGetProcessName(y), StringComparison.CurrentCultureIgnoreCase)

            If 0 <> diff Then
                Return diff
            End If
            Return x.Id - y.Id

        End Function 'ProcessComparison

        ''' <summary>
        ''' Retrieves the list of all processes matching the Id
        ''' parameter.
        ''' Generates a non-terminating error for each specified
        ''' process ID which is not found.
        ''' </summary>
        ''' <returns>An array of processes that match the given id.
        ''' </returns>
        Protected Function GetMatchingProcessesById() As List(Of Process)

            Dim matchingProcesses As List(Of Process) = New List(Of Process)

            If Not (processIds Is Nothing) Then

                ' The keys dictionary will be used for rapid lookup of 
                ' processes already in the matchingProcesses list.
                Dim keys As Dictionary(Of Integer, Byte) = _
                    New Dictionary(Of Integer, Byte)()

                Dim processId As Integer
                For Each processId In processIds
                    WriteVerbose("Finding match for process id " & _
                        processId & ".")

                    If Not keys.ContainsKey(processId) Then
                        Dim process As Process
                        Try
                            process = _
                               System.Diagnostics.Process.GetProcessById( _
                                   processId)
                        Catch ex As ArgumentException
                            WriteError(New ErrorRecord(ex, _
                                "ProcessIdNotFound", _
                                ErrorCategory.ObjectNotFound, processId))
                            GoTo ContinueForEach1
                        End Try

                        WriteDebug("Found matching process.")

                        matchingProcesses.Add(process)
                        keys.Add(processId, 0)
                    End If
ContinueForEach1:
                Next processId
            End If

            Return matchingProcesses

        End Function 'GetMatchingProcessesById

        ''' <summary>
        ''' Retrieves the list of all processes matching the Input
        ''' parameter.
        ''' </summary>
        Private Function GetProcessesByInput() As List(Of Process)

            Dim matchingProcesses As List(Of Process) = New List(Of Process)()

            If Not (Nothing Is Input) Then
                ' The keys dictionary will be used for rapid lookup of 
                ' processes already in the matchingProcesses list.
                Dim keys As Dictionary(Of Integer, Byte) = _
                    New Dictionary(Of Integer, Byte)()

                Dim process As Process
                For Each process In Input
                    WriteVerbose("Refreshing process object.")

                    If Not keys.ContainsKey(process.Id) Then
                        Try
                            process.Refresh()
                        Catch e1 As Win32Exception
                        Catch e2 As InvalidOperationException
                        End Try
                        matchingProcesses.Add(process)
                    End If
                Next process
            End If
            Return matchingProcesses

        End Function 'GetProcessesByInput

#End Region

#Region "Private Data"

        Private processNames() As String
        Private processIds() As Integer
        Private inputObjects() As Process

#End Region

    End Class 'GetProcCommand 

#End Region

#Region "PowerShell snap-in" '
    ''' <summary>
    ''' Create this sample as an PowerShell snap-in
    ''' </summary>
    <RunInstaller(True)> _
    Public Class GetProcPSSnapIn05
        Inherits PSSnapIn

        ''' <summary>
        ''' Create an instance of the GetProcPSSnapIn05
        ''' </summary>
        Public Sub New()

        End Sub 'New

        ''' <summary>
        ''' Get a name for this PowerShell snap-in. This name will
        ''' be used in registering
        ''' this PowerShell snap-in.
        ''' </summary>

        Public Overrides ReadOnly Property Name() As String
            Get
                Return "GetProcPSSnapIn05"
            End Get
        End Property

        ''' <summary>
        ''' Vendor information for this PowerShell snap-in.
        ''' </summary>

        Public Overrides ReadOnly Property Vendor() As String
            Get
                Return "Microsoft"
            End Get
        End Property

        ''' <summary>
        ''' Gets resource information for vendor. This is a string of format: 
        ''' resourceBaseName,resourceName. 
        ''' </summary>

        Public Overrides ReadOnly Property VendorResource() As String
            Get
                Return "GetProcPSSnapIn05,Microsoft"
            End Get
        End Property

        ''' <summary>
        ''' Description of this PowerShell snap-in.
        ''' </summary>

        Public Overrides ReadOnly Property Description() As String
            Get
                Return "This is a PowerShell snap-in that includes " & _
                    "the get-proc sample."
            End Get
        End Property
    End Class 'GetProcPSSnapIn05

#End Region

End Namespace

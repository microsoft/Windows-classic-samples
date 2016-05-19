'
' Copyright (c) 2006 Microsoft Corporation. All rights reserved.
' 
' THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
' ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
' THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
' PARTICULAR PURPOSE.
' Description: This class will demonstrate creating a sample Select-Obj
' cmdlet. This cmdlet mimics the behavior of the Select-Object cmdlet
' included in the Windows PowerShell distribution.
'
' For a description of Select-Object run "get-help Select-Object" from
' Windows PowerShell command line.
'
' System namespaces needed
Imports System
Imports System.Collections.Generic
Imports System.Diagnostics
Imports System.Globalization
Imports Microsoft.VisualBasic

' Windows PowerShell namespaces needed
Imports System.Management.Automation


Namespace Microsoft.Samples.PowerShell.Commands

    ''' <summary>
    ''' A cmdlet to select properties from objects.
    ''' </summary>
    ''' <remarks>
    ''' Making this an un-derivable class. Depending on your needs
    ''' change this.
    ''' </remarks>
    <Cmdlet("Select", "Obj")> _
    Public NotInheritable Class SelectObjCommand
        Inherits Cmdlet
#Region "Parameters"

        ''' <summary>
        ''' Object that this cmdlet will work on.
        ''' Can be either from pipeline or directly specified 
        ''' as an argument to the cmdlet
        ''' </summary>
        ''' <value></value>

        <Parameter(ValueFromPipeline:=True)> _
        Public Property InputObject() As PSObject
            Get
                Return myInputObject
            End Get
            Set(ByVal value As PSObject)
                myInputObject = value
            End Set
        End Property
        Private myInputObject As PSObject

        ''' <summary>
        ''' Property parameter for the cmdlet.
        ''' Takes an array of strings as input.
        ''' </summary>

        <Parameter(Position:=0)> _
        Public Property [Property]() As String()
            Get
                Return properties
            End Get
            Set(ByVal value As String())
                properties = value
            End Set
        End Property
        Private properties() As String

        ''' <summary>
        ''' Helps to select specific number of objects from the
        ''' last object.
        ''' </summary>

        <Parameter(), ValidateRange(1, Integer.MaxValue)> _
        Public Property Last() As Integer
            Get
                Return myLast
            End Get
            Set(ByVal value As Integer)
                myLast = value
            End Set
        End Property
        Private myLast As Integer = 0

        ''' <summary>
        ''' Helps to select specific number of objects from the
        ''' first object.
        ''' </summary>

        <Parameter(), ValidateRange(1, Integer.MaxValue)> _
        Public Property First() As Integer
            Get
                Return myFirst
            End Get
            Set(ByVal value As Integer)
                myFirst = value
            End Set
        End Property
        Private myFirst As Integer = 0

        ''' <summary>
        ''' Expand specific properties from the selected object
        ''' </summary>
        ''' <value></value>

        <Parameter()> _
        Public Property Expand() As String
            Get
                Return myExpand
            End Get
            Set(ByVal value As String)
                myExpand = value
            End Set
        End Property
        Private myExpand As String = Nothing

        ''' <summary>
        ''' Select unique objects or not
        ''' </summary>
        ''' <value>true or false</value>

        <Parameter()> _
        Public Property Unique() As Boolean
            Get
                Return myUnique
            End Get
            Set(ByVal value As Boolean)
                myUnique = value
            End Set
        End Property
        Private myUnique As Boolean = False

#End Region

#Region "Internal Properties/Data"

        Private inputQueue As SelectObjQueue
        Private uniques As List(Of UniquePSObjectHelper) = Nothing

#End Region

#Region "Cmdlet Overrides"


        ''' <summary>
        ''' Intialize cmdlet data here.
        ''' This method is called only once even when the cmdlet
        ''' is used in the middle a pipe.
        ''' </summary>
        Protected Overrides Sub BeginProcessing()
            WriteVerbose("Initializing Object Queue")

            If unique Then
                uniques = New List(Of UniquePSObjectHelper)()
            End If

            inputQueue = New SelectObjQueue(myFirst, myLast)

        End Sub 'BeginProcessing


        ''' <summary>
        ''' Do per object related work here.
        ''' This method is called for every object that is coming
        ''' from the input pipe.
        ''' </summary>
        Protected Overrides Sub ProcessRecord()
            If myInputObject Is Nothing Then
                Return
            End If

            WriteVerbose("Received an input Object. Processing...")

            inputQueue.Enqueue(myInputObject)

            ' Lets see whether we can process this object right away..
            ' We can process the object
            ' (a) if no first or last is specified..
            ' (b) first is specified and the limit is not reached..
            Dim streamingInputObject As PSObject = inputQueue.StreamingDequeue()

            If Not (streamingInputObject Is Nothing) Then
                ProcessObject(streamingInputObject)

                WriteVerbose("Processed InputObject")
            End If

        End Sub 'ProcessRecord


        ''' <summary>
        ''' This method is called after all the objects are processed.
        ''' This method is called as the last instruction in a pipe.
        ''' 
        ''' For select-obj, we will do all the object manipulation 
        ''' in this method.
        ''' </summary>
        Protected Overrides Sub EndProcessing()
            WriteVerbose("Processing remaining Input objects")
            Dim targetObject As PSObject
            For Each targetObject In inputQueue
                ProcessObject(targetObject)
            Next targetObject

            If Not (uniques Is Nothing) Then
                Dim obj As UniquePSObjectHelper
                For Each obj In uniques
                    If obj.WrittenObject Is Nothing Then
                        GoTo ContinueForEach1
                    End If
                    WriteObject(obj.WrittenObject)
ContinueForEach1:
                Next obj
            End If

        End Sub 'EndProcessing

#End Region

#Region "Internal Methods"


        Private Sub ProcessObject(ByVal inputObject As PSObject)
            If (properties Is Nothing OrElse properties.Length = 0) _
               AndAlso String.IsNullOrEmpty(expand) _
            Then
                SendToOutputPipe(inputObject, New List(Of PSNoteProperty)())
                Return
            End If
            ' Collect property values for the properties specified by
            ' the user.
            Dim matchedProperties As List(Of PSNoteProperty) = _
                New List(Of PSNoteProperty)()

            If Not (properties Is Nothing) AndAlso properties.Length > 0 _
            Then
                WriteVerbose("Collecting property values for the " & _
                    "user specified properties")

                ' make a collection of property values
                Dim propertyName As String
                For Each propertyName In properties
                    ProcessParameter(propertyName, inputObject, _
                        matchedProperties)
                Next propertyName
            End If

            If Not String.IsNullOrEmpty(expand) Then
                WriteVerbose("Expanding a property")
                ProcessExpandParameter(inputObject, matchedProperties)
            Else
                ' Create a new psobject and embed properties that the
                ' user asked for.
                Dim outputObject As New PSObject()

                If matchedProperties.Count <> 0 Then
                    Dim noteProperty As PSNoteProperty
                    For Each noteProperty In matchedProperties
                        Try
                            outputObject.Properties.Add(noteProperty)
                        Catch
                            WriteAlreadyExistingPropertyError( _
                                noteProperty.Name, inputObject)
                        End Try
                    Next noteProperty
                End If

                SendToOutputPipe(outputObject, matchedProperties)
            End If

        End Sub 'ProcessObject


        ''' <summary>
        ''' Gets the value for the property specified by
        ''' <paramref name="propertyName"/> from 
        ''' <paramref name="inputObject"/> and adds the value
        ''' to <paramref name="matchedProperties"/>
        ''' </summary>
        ''' <param name="propertyName">
        ''' Property to get the value for.
        ''' </param>
        ''' <param name="inputObject">
        ''' Input object to get the value from.
        ''' </param>
        ''' <param name="matchedProperties">
        ''' Collection to add the value to.
        ''' </param>
        ''' <remarks>
        ''' <paramref name="matchedProperties"/>
        ''' is assumed to be initialized.
        ''' </remarks>
        Private Sub ProcessParameter(ByVal propertyName As String, _
            ByVal inputObject As PSObject, _
            ByVal matchedProperties As List(Of PSNoteProperty))

            Dim propValue As Object = GetPropValue(inputObject, propertyName)

            ' propValue can be null..
            Dim newProperty As New PSNoteProperty(propertyName, propValue)
            matchedProperties.Add(newProperty)
        End Sub

        Private Sub ProcessExpandParameter(ByRef inputObject As PSObject, _
            ByVal matchedProperties As List(Of PSNoteProperty))

            ' expand parameter is used
            ' expand the property value from the inputobject
            Dim propValue As Object = Nothing

            Try
                propValue = GetPropValue(inputObject, expand)
            Catch e As Exception
                WriteNonTerminatingError(expand, "PropertyAccessException", e, _
                    ErrorCategory.InvalidData)
                Return
            End Try

            If propValue Is Nothing Then
                Dim message As String = String.Format( _
                    CultureInfo.CurrentCulture, _
                    GetErrorMessage(PROPERTYNOTFOUND), expand)

                Dim e As ArgumentException = New ArgumentException(message)
                WriteNonTerminatingError(expand, _
                    "PropertyNotFound", e, ErrorCategory.InvalidData)

                Return
            End If

            ' Expand prop value using a Enumerator
            Dim results As System.Collections.IEnumerable = _
                LanguagePrimitives.GetEnumerable(propValue)

            If results Is Nothing Then
                Dim message As String = String.Format( _
                    CultureInfo.CurrentCulture, _
                    GetErrorMessage(NOTHINGTOEXPAND), expand)

                Dim e As ArgumentException = New ArgumentException(message)
                WriteNonTerminatingError(expand, _
                    "NothingToExpand", e, ErrorCategory.InvalidArgument)

                Return
            End If

            Dim expandedValue As Object
            For Each expandedValue In results
                If expandedValue Is Nothing Then
                    WriteVerbose("One of the expanded values is null")
                    GoTo ContinueForEach1
                End If

                Dim expandedObject As PSObject = _
                    PSObject.AsPSObject(expandedValue)
                Dim noteProperty As PSNoteProperty
                For Each noteProperty In matchedProperties
                    Try
                        If Not (expandedObject.Properties( _
                            noteProperty.Name) Is Nothing) _
                        Then
                            WriteAlreadyExistingPropertyError( _
                                noteProperty.Name, inputObject)
                        Else
                            expandedObject.Properties.Add(noteProperty)
                        End If
                    Catch
                        WriteAlreadyExistingPropertyError( _
                            noteProperty.Name, inputObject)
                    End Try
                Next noteProperty

                SendToOutputPipe(expandedObject, matchedProperties)
ContinueForEach1:
            Next expandedValue
        End Sub

        Private Sub SendToOutputPipe(ByVal objectToWrite As PSObject, _
        ByVal addedNoteProperties As List(Of PSNoteProperty))

            If Not unique Then
                If Not (objectToWrite Is Nothing) Then
                    WriteObject(objectToWrite)
                End If

                Return
            End If

            Dim isObjUnique As Boolean = True

            Dim uniqueObj As UniquePSObjectHelper
            For Each uniqueObj In uniques
                Dim comparer As New ObjectCommandComparer(True, _
                    System.Threading.Thread.CurrentThread.CurrentCulture, True)
                If comparer.Compare(objectToWrite.BaseObject, _
                    uniqueObj.WrittenObject.BaseObject) = 0 AndAlso _
                    uniqueObj.NotePropertyCount = addedNoteProperties.Count _
                Then
                    Dim found As Boolean = True
                    Dim note As PSNoteProperty
                    For Each note In addedNoteProperties
                        Dim prop As PSMemberInfo = _
                            uniqueObj.WrittenObject.Properties(note.Name)
                        If prop Is Nothing OrElse _
                               comparer.Compare(prop.Value, note.Value) <> 0 Then
                            found = False
                            Exit For
                        End If
                    Next note
                    If found Then
                        isObjUnique = False
                        Exit For
                    End If
                Else
                    GoTo ContinueForEach1
                End If
ContinueForEach1:
            Next uniqueObj
            If isObjUnique Then
                uniques.Add(New UniquePSObjectHelper(objectToWrite, _
                    addedNoteProperties.Count))
            End If
        End Sub

        ''' <summary>
        ''' Returns the value of property from the inputObject.
        ''' </summary>
        ''' <param name="inputObject">
        ''' Object for which the property value is needed.
        ''' </param>
        ''' <param name="property">
        ''' Name of the property for which the value is returned.
        ''' </param>
        ''' <returns>
        ''' null if property is not found,
        ''' value of the property otherwise.
        ''' </returns>
        Private Function GetPropValue(ByRef inputObject As PSObject, _
            ByRef propertyName As String) As Object

            ' This is an internal method. Please make sure valid input is passed 
            ' to this method.
            Debug.Assert(Not (inputObject Is Nothing), _
                "Cannot work with a null object.")
            Debug.Assert(Not String.IsNullOrEmpty(propertyName), _
                "Property should not be null or Empty")

            ' we have no globbing: try an exact match, because this is quicker.
            Dim x As PSMemberInfo = inputObject.Members(propertyName)

            If (x Is Nothing) Then
                Return Nothing
            Else
                Return x.Value
            End If
        End Function

#End Region

#Region "Error Handling / Error Messages"

        Public Const PROPERTYALREADYEXISTS As Integer = 0
        Public Const PROPERTYNOTFOUND As Integer = 1
        Public Const NOTHINGTOEXPAND As Integer = 2

        Dim errors() As String = { _
           "Property cannot be processed because property {0} already exists.", _
           "Property ""{0}"" cannot be found", _
           "Cannot expand property ""{0}"" because it has nothing to expand." _
        }

        ''' <summary>
        ''' Gets error message string corresponding to <paramref name="errorID"/>
        ''' </summary>
        ''' <param name="errorID">Error ID of the error message</param>
        ''' <returns>A string representing the error.</returns>
        ''' <remarks>
        ''' You can this error subsystem to return locale specific
        ''' error messages.
        ''' </remarks>
        Private Function GetErrorMessage(ByRef errorID As Integer) As String
            Return errors(errorID)
        End Function

        ''' <summary>
        ''' Writes a non-terminating error onto the pipeline.
        ''' </summary>
        ''' <param name="targetObject">
        ''' Object which caused this exception.
        ''' </param>
        ''' <param name="errorId">ErrorId for this error.</param>
        ''' <param name="innerException">Complete exception object.</param>
        ''' <param name="category">ErrorCategory for this exception.</param>
        Friend Sub WriteNonTerminatingError( _
            ByRef targetObject As Object, _
            ByRef errorId As String, _
            ByRef innerException As Exception, _
            ByRef category As ErrorCategory)

            WriteError(New ErrorRecord(innerException, errorId, category, _
                targetObject))
        End Sub

        Friend Sub WriteAlreadyExistingPropertyError( _
            ByRef propertyName As String, _
            ByRef inputObject As Object)

            Dim message As String = _
                String.Format(CultureInfo.CurrentCulture, _
                GetErrorMessage(PROPERTYALREADYEXISTS), propertyName)

            Dim e As ArgumentException = New ArgumentException(message)
            WriteNonTerminatingError(inputObject, "PropertyAlreadyExists", _
                e, ErrorCategory.InvalidData)
        End Sub

#End Region

#Region "SelectObj Queue"

        Private Class SelectObjQueue
            Inherits Queue(Of PSObject)
#Region "SelectObjQueue Data"

            Dim headLimit As Integer = 0
            Dim tailLimit As Integer = 0
            Dim streamedObjectCount As Integer = 0

#End Region

#Region "SelectObjQueue Construction"

            Public Sub New(ByRef first As Integer, ByRef last As Integer)
                headLimit = first
                tailLimit = last
            End Sub

#End Region

#Region "SelectObjQueue Methods"

            Public Overloads Sub Enqueue(ByRef obj As PSObject)
                If tailLimit > 0 AndAlso Me.Count >= tailLimit Then
                    MyBase.Dequeue()
                End If
                MyBase.Enqueue(obj)
            End Sub

            Public Function StreamingDequeue() As PSObject
                If headLimit = 0 AndAlso tailLimit = 0 OrElse _
                        streamedObjectCount < headLimit Then
                    Debug.Assert(Me.Count > 0, "Streaming an empty queue")
                    streamedObjectCount += 1
                    Return Dequeue()
                End If
                If tailLimit = 0 Then
                    Dequeue()
                End If

                Return Nothing
            End Function

#End Region
        End Class

#End Region

#Region "UniquePSObjectHelper"

        Private Class UniquePSObjectHelper
            Friend Sub New(ByRef o As PSObject, _
                ByRef notePropertyCount As Integer)

                WrittenObject = o
                Me.myNotePropertyCount = notePropertyCount
            End Sub
            Friend ReadOnly WrittenObject As PSObject
            Friend ReadOnly Property NotePropertyCount() As Integer
                Get
                    Return myNotePropertyCount
                End Get
            End Property
            Private myNotePropertyCount As Integer

        End Class

#End Region

    End Class 'SelectObjCommand

End Namespace


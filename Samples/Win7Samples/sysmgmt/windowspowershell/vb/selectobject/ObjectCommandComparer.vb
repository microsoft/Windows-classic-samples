'
' Copyright (c) 2006 Microsoft Corporation. All rights reserved.
' 
' THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
' ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
' THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
' PARTICULAR PURPOSE.
'
' Description: Helper class to support select-obj cmdlet's
'              unique functionality.
'
' System namespaces needed
Imports System
Imports System.Collections.Generic
Imports System.Collections
Imports System.Text
Imports System.Globalization
Imports System.Threading
Imports Microsoft.VisualBasic

' Windows PowerShell namespaces needed
Imports System.Management.Automation

Namespace Microsoft.Samples.PowerShell.Commands

#Region "PSObject Comparer" '

    ''' <summary>
    ''' Keeps the property value of inputObject. Because the value of
    ''' a non-existing property is null, isExistingProperty is needed
    ''' to distinguish whether a property exists and its value is null or
    ''' the property does not exist at all.
    ''' </summary>

    Friend Class ObjectCommandPropertyValue

        Private Sub New()

        End Sub 'New

        Friend Sub New(ByVal propVal As Object)
            myPropertyValue = propVal
            myIsExistingProperty = True

        End Sub 'New

        Friend ReadOnly Property PropertyValue() As Object
            Get
                Return myPropertyValue
            End Get
        End Property

        Friend ReadOnly Property IsExistingProperty() As Boolean
            Get
                Return myIsExistingProperty
            End Get
        End Property

        Private myPropertyValue As Object
        Private myIsExistingProperty As Boolean
        Friend Shared NonExistingProperty As New ObjectCommandPropertyValue()
        Friend Shared ExistingNullProperty As _
            New ObjectCommandPropertyValue(Nothing)

    End Class 'ObjectCommandPropertyValue

    ''' <summary>
    ''' 
    ''' </summary>

    Friend Class ObjectCommandComparer
        Implements IComparer

        ''' <summary>
        ''' Constructor that doesn't set any private field.
        ''' Necessary because compareTo can compare two objects by calling
        ''' ((ICompare)obj1).CompareTo(obj2) without using a key.
        ''' </summary>
        Friend Sub New(ByVal ascending As Boolean, _
                ByVal cultureInfo As CultureInfo, _
                ByVal caseSensitive As Boolean)
            Me.ascendingOrder = ascending
            Me.cultureInfo = cultureInfo
            If Me.cultureInfo Is Nothing Then
                Me.cultureInfo = Thread.CurrentThread.CurrentCulture
            End If
            Me.caseSensitive = caseSensitive

        End Sub 'New


        ''' <summary>
        ''' Check whether <paramref name="obj"/> can be converted to an
        ''' PSObject and then check whehter the PSObject is null.
        ''' </summary>
        ''' <param name="obj"></param>
        ''' <returns></returns>
        Private Shared Function IsValueNull(ByVal obj As Object) As Boolean
            Dim psObj As PSObject

            If (TypeOf obj Is PSObject) Then
                psObj = CType(obj, PSObject)
            Else
                psObj = Nothing
            End If

            ' obj is converted to an psobject and is found to be null..
            If psObj Is Nothing Then
                Return obj Is Nothing
            End If

            If Nothing = psObj.ImmediateBaseObject Then
                Return obj Is Nothing
            End If

            Dim returnValue As Object = Nothing
            Do
                returnValue = psObj.ImmediateBaseObject
                If (TypeOf returnValue Is PSObject) Then
                    psObj = CType(returnValue, PSObject)
                Else
                    psObj = Nothing
                End If

            Loop While Not (psObj Is Nothing) AndAlso _
            (Not (psObj.ImmediateBaseObject Is Nothing))

            Return returnValue Is Nothing

        End Function 'IsValueNull

        ''' <summary>
        ''' Main method that will compare first and second by
        ''' their keys considering case and order
        ''' </summary>
        ''' <param name="x">
        ''' first object to extract value
        ''' </param>
        ''' <param name="y">
        ''' second object to extract value
        ''' </param>
        ''' <returns> 
        ''' 0 if they are the same, less than 0 if first is smaller,
        ''' more than 0 if first is greater
        ''' </returns>
        Friend Function Compare(ByVal first As Object, _
        ByVal second As Object) As Integer Implements IComparer.Compare

            ' This method will never throw exceptions, two null
            ' objects are considered the same
            If IsValueNull(first) AndAlso IsValueNull(second) Then
                Return 0
            End If

            Dim firstPS As PSObject
            If (TypeOf first Is PSObject) Then
                firstPS = CType(first, PSObject)
            Else
                firstPS = Nothing
            End If

            If Not (firstPS Is Nothing) Then
                first = firstPS.BaseObject
            End If

            Dim secondPS As PSObject
            If (TypeOf second Is PSObject) Then
                secondPS = CType(second, PSObject)
            Else
                secondPS = Nothing
            End If

            If Not (secondPS Is Nothing) Then
                second = secondPS.BaseObject
            End If

            Try
                Return LanguagePrimitives.Compare(first, second, _
            Not caseSensitive, cultureInfo) _
                    * IIf(ascendingOrder, 1, -1)
            Catch e1 As InvalidCastException
            Catch e2 As ArgumentException
            End Try

            ' Note that this will occur if the objects do not support
            ' IComparable.  We fall back to comparing as strings.

            ' being here means the first object doesn't support ICompare
            ' or an Exception was raised with Compare
            Dim firstString As String = PSObject.AsPSObject(first).ToString()
            Dim secondString As String = PSObject.AsPSObject(second).ToString()

            Return String.Compare(firstString, secondString, _
                Not caseSensitive, cultureInfo) * _
                IIf(ascendingOrder, 1, -1)

        End Function 'Compare


        Friend Function Compare(ByVal first As ObjectCommandPropertyValue, _
            ByVal second As ObjectCommandPropertyValue) As Integer

            If first.IsExistingProperty AndAlso second.IsExistingProperty Then
                Return [Compare](first.PropertyValue, second.PropertyValue)
            End If

            ' if first.IsExistingProperty, !second.IsExistingProperty;
            ' otherwise the first branch if would return. Regardless
            ' of key orders non existing property 
            ' will be considered greater than others
            If first.IsExistingProperty Then
                Return -1
            End If

            ' viceversa for the first.IsExistingProperty
            If second.IsExistingProperty Then
                Return 1
            End If

            'both are nonexisting
            Return 0

        End Function 'Compare

        Private cultureInfo As CultureInfo = Nothing
        Private ascendingOrder As Boolean = True
        Private caseSensitive As Boolean = False

    End Class 'ObjectCommandComparer 

#End Region

End Namespace


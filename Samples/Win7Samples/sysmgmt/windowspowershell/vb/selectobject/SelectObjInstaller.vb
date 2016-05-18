'
' Copyright (c) 2006 Microsoft Corporation. All rights reserved.
' 
' THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
' ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
' THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
' PARTICULAR PURPOSE.

' Description: PSSnapIn installation information SelectObjSnapIn
' System namespaces needed
Imports System.ComponentModel

' Windows PowerShell namespaces needed
Imports System.Management.Automation


Namespace Microsoft.Samples.PowerShell.Commands

    ''' <summary>
    ''' Create this sample as an PowerShell snap-in
    ''' </summary>
    <RunInstaller(True)> _
    Public Class SelectObjPSSnapIn
        Inherits PSSnapIn

        ''' <summary>
        ''' Create an instance of the SelectObjPSSnapIn
        ''' </summary>
        Public Sub New()

        End Sub 'New

        ''' <summary>
        ''' Get a name for this PowerShell snap-in. This name will be used
        ''' in registering this PowerShell snap-in.
        ''' </summary>
        Public Overrides ReadOnly Property Name() As String
            Get
                Return "SelectObjSample"
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
        ''' Description of this PowerShell snap-in.
        ''' </summary>
        Public Overrides ReadOnly Property Description() As String
            Get
                Return "This is a PowerShell snap-in that includes " & _
                    "the select-obj cmdlet."
            End Get
        End Property
    End Class 'SelectObjPSSnapIn

End Namespace


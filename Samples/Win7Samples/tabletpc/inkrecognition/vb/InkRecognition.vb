'-------------------------------------------------------------------------
'
' THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
' ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
' THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
' PARTICULAR PURPOSE.
'
' Copyright (c) Microsoft Corporation. All rights reserved.
'
'  File: InkRecognition.vb
'  Simple Ink Recognition Sample Application
'
'  This program demonstrates how one can build a basic handwriting
'  recognition application using Microsoft Tablet PC Automation API.
'  It first creates an InkCollector object to enable inking
'  in the window. Upon receiving the "Recognize!" command, fired
'  from the application's button, ToString() is invoked on the
'  collected ink strokes to retrieve the best match using the
'  default recognizer.  The results are presented in a message box.
'
'  The features used are: InkCollector, Ink, Strokes,
'  RecognizerContext, and RecognizerResult
'
'--------------------------------------------------------------------------

Imports System
Imports System.Windows.Forms
Imports System.Drawing

' The Ink namespace, which contains the Tablet PC Platform API
Imports Microsoft.Ink

Namespace Microsoft.Samples.TabletPC.InkRecognition

    Public Class InkRecognition
        Inherits Form

        ' Create a global ink collector object called myInkCollector
        Dim myInkCollector As InkCollector

        ' Recognizers is used to retrieve the number of installed recognizers
        Dim myRecognizers As Recognizers

#Region " Window Form Designer generated code "
        'Form overrides dispose to clean up the component list.
        Protected Overloads Overrides Sub Dispose(ByVal disposing As Boolean)
            If disposing Then
                If Not (components Is Nothing) Then
                    components.Dispose()
                End If
                If Not (myInkCollector Is Nothing) Then
                    myInkCollector.Dispose()
                End If
            End If
            MyBase.Dispose(disposing)
        End Sub

        'Required by the Windows Form Designer
        Private components As System.ComponentModel.IContainer

        'NOTE: The following procedure is required by the Windows Form Designer
        'It can be modified using the Windows Form Designer.  
        'Do not modify it using the code editor.
        Friend WithEvents btnRecognize As System.Windows.Forms.Button
        Friend WithEvents gbInkArea As System.Windows.Forms.GroupBox
        Friend WithEvents txtResults As System.Windows.Forms.TextBox
        <System.Diagnostics.DebuggerStepThrough()> Private Sub InitializeComponent()
            Me.btnRecognize = New System.Windows.Forms.Button()
            Me.gbInkArea = New System.Windows.Forms.GroupBox()
            Me.txtResults = New System.Windows.Forms.TextBox()
            Me.SuspendLayout()
            '
            'btnRecognize
            '
            Me.btnRecognize.Location = New System.Drawing.Point(8, 160)
            Me.btnRecognize.Name = "btnRecognize"
            Me.btnRecognize.Size = New System.Drawing.Size(280, 23)
            Me.btnRecognize.TabIndex = 0
            Me.btnRecognize.Text = "Recognize Ink"
            '
            'gbInkArea
            '
            Me.gbInkArea.Location = New System.Drawing.Point(8, 0)
            Me.gbInkArea.Name = "gbInkArea"
            Me.gbInkArea.Size = New System.Drawing.Size(280, 152)
            Me.gbInkArea.TabIndex = 1
            Me.gbInkArea.TabStop = False
            Me.gbInkArea.Text = "Ink Here"
            '
            'txtResults
            '
            Me.txtResults.BackColor = System.Drawing.SystemColors.Window
            Me.txtResults.Location = New System.Drawing.Point(8, 192)
            Me.txtResults.Name = "txtResults"
            Me.txtResults.ReadOnly = True
            Me.txtResults.Size = New System.Drawing.Size(280, 20)
            Me.txtResults.TabIndex = 2
            Me.txtResults.Text = ""
            '
            'InkRecognition
            '
            Me.AutoScaleBaseSize = New System.Drawing.Size(5, 13)
            Me.ClientSize = New System.Drawing.Size(292, 222)
            Me.Controls.AddRange(New System.Windows.Forms.Control() {Me.txtResults, Me.gbInkArea, Me.btnRecognize})
            Me.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedSingle
            Me.MaximizeBox = False
            Me.Name = "InkRecognition"
            Me.Text = "Ink Recognition Sample"
            Me.ResumeLayout(False)

        End Sub
#End Region

#Region " Standard Template Code "
        Public Shared Sub Main()
            Application.Run(New InkRecognition())
        End Sub
#End Region

        Public Sub New()
            'This call is required by the Windows Form Designer.
            InitializeComponent()
        End Sub

        ''''''''''''''''''''''''''''''''''''''''''''''''''
        ' Event Handler from Form Load Event
        ''''''''''''''''''''''''''''''''''''''''''''''''''
        Private Sub InkRecognition_Load(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles MyBase.Load

            ' Create the recognizers collection
            myRecognizers = New Recognizers()

            'Create a new ink collector that uses the the group box handle
            myInkCollector = New InkCollector(gbInkArea.Handle)

            ' Turn the ink collector on
            myInkCollector.Enabled = True

        End Sub

        ''''''''''''''''''''''''''''''''''''''''''''''''''
        ' Event Handle from Recognize Button Click Event
        ''''''''''''''''''''''''''''''''''''''''''''''''''
        Private Sub btnRecognize_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles btnRecognize.Click

            ' Check to ensure that the user has at least one recognizer installed
            ' Note that this is a preventive check - otherwise, an exception will
            ' occur during recognition
            If 0 = myRecognizers.Count Then
                MessageBox.Show("There are no handwriting recognizers installed.  You need to have at least one in order to run this sample.")
            Else

                ' Note that the Strokes' ToString() method is a
                ' shortcut  for retrieving the best match using the
                ' default recognizer.  The same result can also be
                ' obtained using the RecognizerContext.  For an
                ' example, uncomment the following lines of code:
                '
                ' Dim myRecoContext As New RecognizerContext()
                ' Dim status As RecognitionStatus
                ' Dim recoResult As RecognitionResult
                '
                ' myRecoContext.Strokes = myInkCollector.Ink.Strokes
                ' recoResult = myRecoContext.Recognize(status)
                ' txtResults.SelectedText = recoResult.TopString
                '
                txtResults.SelectedText = myInkCollector.Ink.Strokes.ToString

                ' If the mouse is pressed, do not perform the recognition -
                ' this prevents deleting a stroke that is still being drawn
                If Not myInkCollector.CollectingInk Then

                    ' Delete the ink from the ink collector
                    myInkCollector.Ink.DeleteStrokes(myInkCollector.Ink.Strokes)

                    ' Force the Frame to redraw (so the deleted ink will go away)
                    gbInkArea.Refresh()

                End If
            End If
        End Sub
    End Class

End Namespace

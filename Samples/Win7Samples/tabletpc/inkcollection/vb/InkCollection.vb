' THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
' ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
' THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
' PARTICULAR PURPOSE.
'
' Copyright (c) Microsoft Corporation. All rights reserved.
'
'  File: InkCollection.vb
'  Simple Ink Collection Sample Application
'
'  This sample program is a "Hello World" application, which
'  demonstrates basic functionality of the Tablet PC platform. 
'  It is the simplest program that you can build using the 
'  Tablet PC Platform APIs. This application uses an ink
'  collector to collect and render pen input. 
'
'  The features used are: InkCollector, default tablet, and 
'  modifying default drawing attributes.
'
'--------------------------------------------------------------------------

' The Ink namespace, which contains the Tablet PC Platform API
Imports Microsoft.Ink
Imports System
Imports System.Drawing
Imports System.Windows.Forms

Namespace Microsoft.Samples.TabletPC.InkCollection

    ' The InkCollection Sample Application form class
    Public Class InkCollection
        Inherits Form

        ' Declare the Ink Collector object
        Private myInkCollector As InkCollector

        ' Declare constants for the pen widths used by this application.
        ' Note that these constants are in high metric units (1 unit = .01mm)
        Private Const ThinInkWidth As Integer = 10
        Private Const MediumInkWidth As Integer = 100
        Private Const ThickInkWidth As Integer = 200

#Region " Standard Template Code "
        Friend WithEvents miMain As System.Windows.Forms.MainMenu
#End Region

#Region " Windows Form Designer generated code "

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
        Friend WithEvents miMainFile As System.Windows.Forms.MenuItem
        Friend WithEvents miExit As System.Windows.Forms.MenuItem
        Friend WithEvents miMainInk As System.Windows.Forms.MenuItem
        Friend WithEvents miEnabled As System.Windows.Forms.MenuItem
        Friend WithEvents miColor As System.Windows.Forms.MenuItem
        Friend WithEvents miRed As System.Windows.Forms.MenuItem
        Friend WithEvents miGreen As System.Windows.Forms.MenuItem
        Friend WithEvents miBlue As System.Windows.Forms.MenuItem
        Friend WithEvents miBlack As System.Windows.Forms.MenuItem
        Friend WithEvents miWidth As System.Windows.Forms.MenuItem
        Friend WithEvents miThin As System.Windows.Forms.MenuItem
        Friend WithEvents miMedium As System.Windows.Forms.MenuItem
        Friend WithEvents miThick As System.Windows.Forms.MenuItem
        <System.Diagnostics.DebuggerStepThrough()> Private Sub InitializeComponent()
            Me.miMain = New System.Windows.Forms.MainMenu()
            Me.miMainFile = New System.Windows.Forms.MenuItem()
            Me.miExit = New System.Windows.Forms.MenuItem()
            Me.miMainInk = New System.Windows.Forms.MenuItem()
            Me.miEnabled = New System.Windows.Forms.MenuItem()
            Me.miColor = New System.Windows.Forms.MenuItem()
            Me.miRed = New System.Windows.Forms.MenuItem()
            Me.miGreen = New System.Windows.Forms.MenuItem()
            Me.miBlue = New System.Windows.Forms.MenuItem()
            Me.miBlack = New System.Windows.Forms.MenuItem()
            Me.miWidth = New System.Windows.Forms.MenuItem()
            Me.miThin = New System.Windows.Forms.MenuItem()
            Me.miMedium = New System.Windows.Forms.MenuItem()
            Me.miThick = New System.Windows.Forms.MenuItem()
            '
            'miMain
            '
            Me.miMain.MenuItems.AddRange(New System.Windows.Forms.MenuItem() {Me.miMainFile, Me.miMainInk})
            '
            'miMainFile
            '
            Me.miMainFile.Index = 0
            Me.miMainFile.MenuItems.AddRange(New System.Windows.Forms.MenuItem() {Me.miExit})
            Me.miMainFile.Text = "File"
            '
            'miExit
            '
            Me.miExit.Index = 0
            Me.miExit.Text = "Exit"
            '
            'miMainInk
            '
            Me.miMainInk.Index = 1
            Me.miMainInk.MenuItems.AddRange(New System.Windows.Forms.MenuItem() {Me.miEnabled, Me.miColor, Me.miWidth})
            Me.miMainInk.Text = "Ink"
            '
            'miEnabled
            '
            Me.miEnabled.Checked = True
            Me.miEnabled.Index = 0
            Me.miEnabled.Text = "Enabled"
            '
            'miColor
            '
            Me.miColor.Index = 1
            Me.miColor.MenuItems.AddRange(New System.Windows.Forms.MenuItem() {Me.miRed, Me.miGreen, Me.miBlue, Me.miBlack})
            Me.miColor.Text = "Color"
            '
            'miRed
            '
            Me.miRed.Index = 0
            Me.miRed.Text = "Red"
            '
            'miGreen
            '
            Me.miGreen.Index = 1
            Me.miGreen.Text = "Green"
            '
            'miBlue
            '
            Me.miBlue.Index = 2
            Me.miBlue.Text = "Blue"
            '
            'miBlack
            '
            Me.miBlack.Index = 3
            Me.miBlack.Text = "Black"
            '
            'miWidth
            '
            Me.miWidth.Index = 2
            Me.miWidth.MenuItems.AddRange(New System.Windows.Forms.MenuItem() {Me.miThin, Me.miMedium, Me.miThick})
            Me.miWidth.Text = "Width"
            '
            'miThin
            '
            Me.miThin.Index = 0
            Me.miThin.Text = "Thin"
            '
            'miMedium
            '
            Me.miMedium.Index = 1
            Me.miMedium.Text = "Medium"
            '
            'miThick
            '
            Me.miThick.Index = 2
            Me.miThick.Text = "Thick"
            '
            'InkCollection
            '
            Me.AutoScaleBaseSize = New System.Drawing.Size(5, 13)
            Me.ClientSize = New System.Drawing.Size(292, 267)
            Me.Menu = Me.miMain
            Me.Name = "InkCollection"
            Me.Text = "Ink Collection Sample"

        End Sub

#End Region

#Region " Standard Template Code "
        Public Shared Sub Main()
            Application.Run(New InkCollection())
        End Sub
#End Region

        Public Sub New()
            'This call is required by the Windows Form Designer.
            InitializeComponent()
        End Sub

        ''''''''''''''''''''''''''''''''''''''''''''''''''
        ' Event Handler from Form Load Event
        ''''''''''''''''''''''''''''''''''''''''''''''''''
        Private Sub InkCollection_Load(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles MyBase.Load

            ' Create a new ink collector and assign it to this form's window
            myInkCollector = New InkCollector(Me.Handle)

            ' Set the pen width to be a medium width
            myInkCollector.DefaultDrawingAttributes.Width = MediumInkWidth

            ' If you do not modify the default drawing attributes, the default 
            ' drawing attributes will use the following properties and values:
            ' 
            '      AntiAliased     = true
            '      Color           = black
            '      FitToCurve      = false
            '      Height          = 1
            '      IgnorePressure  = false
            '      PenTip          = ball
            '      RasterOperation = copy pen
            '      Transparency    = 0
            '      Width           = 53 (2 pixels on a 96 dpi screen)
            ' 
            ' For an example of how to modify other drawing attributes, uncomment
            ' the following lines of code:
            ' myInkCollector.DefaultDrawingAttributes.PenTip = PenTip.Rectangle
            ' myInkCollector.DefaultDrawingAttributes.Height = (0.5F) * MediumInkWidth
            ' myInkCollector.DefaultDrawingAttributes.Transparency = 128

            ' Turn the ink collector on
            myInkCollector.Enabled = True
        End Sub

        ''''''''''''''''''''''''''''''''''''''''''''''''''
        ' Event Handler from Ink->Color->Red Menu Item
        ''''''''''''''''''''''''''''''''''''''''''''''''''
        Private Sub miRed_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles miRed.Click
            myInkCollector.DefaultDrawingAttributes.Color = Color.Red
        End Sub

        ''''''''''''''''''''''''''''''''''''''''''''''''''
        ' Event Handler from Ink->Color->Green Menu Item
        ''''''''''''''''''''''''''''''''''''''''''''''''''
        Private Sub miGreen_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles miGreen.Click
            myInkCollector.DefaultDrawingAttributes.Color = Color.Green
        End Sub

        ''''''''''''''''''''''''''''''''''''''''''''''''''
        ' Event Handler from Ink->Color->Blue Menu Item
        ''''''''''''''''''''''''''''''''''''''''''''''''''
        Private Sub miBlue_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles miBlue.Click
            myInkCollector.DefaultDrawingAttributes.Color = Color.Blue
        End Sub

        ''''''''''''''''''''''''''''''''''''''''''''''''''
        ' Event Handler from Ink->Color->Black Menu Item
        ''''''''''''''''''''''''''''''''''''''''''''''''''
        Private Sub miBlack_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles miBlack.Click
            myInkCollector.DefaultDrawingAttributes.Color = Color.Black
        End Sub

        ''''''''''''''''''''''''''''''''''''''''''''''''''
        ' Event Handler from Ink->Width->Thin Menu Item
        ''''''''''''''''''''''''''''''''''''''''''''''''''
        Private Sub miThin_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles miThin.Click
            myInkCollector.DefaultDrawingAttributes.Width = ThinInkWidth
        End Sub

        ''''''''''''''''''''''''''''''''''''''''''''''''''
        ' Event Handler from Ink->Width->Medium Menu Item
        ''''''''''''''''''''''''''''''''''''''''''''''''''
        Private Sub miMedium_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles miMedium.Click
            myInkCollector.DefaultDrawingAttributes.Width = MediumInkWidth
        End Sub

        ''''''''''''''''''''''''''''''''''''''''''''''''''
        ' Event Handler from Ink->Width->Thick Menu Item
        ''''''''''''''''''''''''''''''''''''''''''''''''''
        Private Sub miThick_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles miThick.Click
            myInkCollector.DefaultDrawingAttributes.Width = ThickInkWidth
        End Sub

        ''''''''''''''''''''''''''''''''''''''''''''''''''
        ' Event Handler from File->Exit Menu Item
        ''''''''''''''''''''''''''''''''''''''''''''''''''
        Private Sub miExit_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles miExit.Click
            myInkCollector.Enabled = False
            Me.Dispose()
        End Sub

        ''''''''''''''''''''''''''''''''''''''''''''''''''
        ' Event Handler from Ink->Enabled Menu Item
        ''''''''''''''''''''''''''''''''''''''''''''''''''
        Private Sub miEnabled_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles miEnabled.Click

            ' Toggle the enabled state of the ink collector
            myInkCollector.Enabled = Not myInkCollector.Enabled
            miEnabled.Checked = myInkCollector.Enabled

        End Sub

    End Class

End Namespace

' THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
' ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
' THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
' PARTICULAR PURPOSE.
'
' Copyright (c) Microsoft Corporation. All rights reserved.
'
'
'  File: AutoClaims.vb
'  AutoClaims Sample Application
'
'  This sample program simulates an auto insurance claim
'  form. It uses the InkEdit control to allow the user to 
'  enter vehicle information and the InkPicture control to
'  illustrate car damage.  The car damage diagram allows
'  the user to select from multiple ink layers;  each layer
'  has its own color and visibility setting.  
'
'  The features used are: InkEdit, InkPicture, providing
'  multiple layers of ink within a control.
'
'--------------------------------------------------------------------------

Imports System
Imports System.Windows.Forms
Imports System.Drawing

' The Ink namespace, which contains the Tablet PC Platform API
Imports Microsoft.Ink

Public Class AutoClaims
    Inherits Form

    ' InkLayer represents a given ink layer, which has a
    ' color and visibility setting associated with it.
    Public Class InkLayer

        ' Declare the ink, color, and visibility for this layer
        Public ActiveInk As Ink
        Public ActiveColor As Color
        Public Hidden As Boolean

        ''''''''''''''''''''''''''''''''''''''''''''''''''
        ' Constructor for InkLayer
        ' 
        ' Parameters:
        ' Ink ink           :       The ink
        ' Color color       :       The color of the layer
        ' Boolean hidden    :       True if the layer is hidden
        ''''''''''''''''''''''''''''''''''''''''''''''''''
        Public Sub New(ByVal ink As Ink, ByVal color As Color, ByVal hidden As Boolean)
            ActiveInk = ink
            ActiveColor = color
            hidden = hidden
        End Sub

    End Class

    ' Declare the array of ink layers used the vehicle illustration.
    Dim inkLayers(3) As InkLayer

    ' Declare an empty ink object (used when we don't want to draw
    ' any ink).
    Dim emptyInk As Ink

    ' Declare a value to hold the index of selected ink
    Dim selectedIndex As Integer

    ' Declare a value to hold whether the selected ink is hidden
    Dim selectedHidden As Boolean

#Region " Windows Form Designer generated code "

    Public Sub New()
        MyBase.New()

        'This call is required by the Windows Form Designer.
        InitializeComponent()

        'Add any initialization after the InitializeComponent() call

    End Sub

    'Form overrides dispose to clean up the component list.
    Protected Overloads Overrides Sub Dispose(ByVal disposing As Boolean)
        If disposing Then
            If Not (components Is Nothing) Then
                components.Dispose()
            End If
        End If
        MyBase.Dispose(disposing)
    End Sub

    'Required by the Windows Form Designer
    Private components As System.ComponentModel.IContainer

    'NOTE: The following procedure is required by the Windows Form Designer
    'It can be modified using the Windows Form Designer.  
    'Do not modify it using the code editor.
    Friend WithEvents InkEdModel As Microsoft.Ink.InkEdit
    Friend WithEvents InkEdMake As Microsoft.Ink.InkEdit
    Friend WithEvents InkEdYear As Microsoft.Ink.InkEdit
    Friend WithEvents inkEdName As Microsoft.Ink.InkEdit
    Friend WithEvents inkEdPolicyNumber As Microsoft.Ink.InkEdit
    Friend WithEvents inkPictVehicle As Microsoft.Ink.InkPicture
    Friend WithEvents lblVehicleDamage As System.Windows.Forms.Label
    Friend WithEvents lblVehicleDescription As System.Windows.Forms.Label
    Friend WithEvents lblModel As System.Windows.Forms.Label
    Friend WithEvents lblYear As System.Windows.Forms.Label
    Friend WithEvents lblMake As System.Windows.Forms.Label
    Friend WithEvents lblPolicyNumber As System.Windows.Forms.Label
    Friend WithEvents lblInsuredName As System.Windows.Forms.Label
    Friend WithEvents chHideLayer As System.Windows.Forms.CheckBox
    Friend WithEvents lstAnnotationLayer As System.Windows.Forms.ListBox
    Friend WithEvents pictHorzontalBar2 As System.Windows.Forms.PictureBox
    Friend WithEvents pictVehicleBox As System.Windows.Forms.PictureBox
    Friend WithEvents pictHorzontalBar3 As System.Windows.Forms.PictureBox
    Friend WithEvents pictPolicyBox As System.Windows.Forms.PictureBox
    Friend WithEvents pictHorzontalBar1 As System.Windows.Forms.PictureBox
    Friend WithEvents lblInstructions As System.Windows.Forms.Label
    Friend WithEvents lblTitle As System.Windows.Forms.Label
    Friend WithEvents pictCheckmarks As System.Windows.Forms.PictureBox
    <System.Diagnostics.DebuggerStepThrough()> Private Sub InitializeComponent()
        Dim resources As System.Resources.ResourceManager = New System.Resources.ResourceManager(GetType(AutoClaims))
        Me.InkEdModel = New Microsoft.Ink.InkEdit()
        Me.InkEdMake = New Microsoft.Ink.InkEdit()
        Me.InkEdYear = New Microsoft.Ink.InkEdit()
        Me.inkEdName = New Microsoft.Ink.InkEdit()
        Me.inkEdPolicyNumber = New Microsoft.Ink.InkEdit()
        Me.inkPictVehicle = New Microsoft.Ink.InkPicture()
        Me.lblVehicleDamage = New System.Windows.Forms.Label()
        Me.lblVehicleDescription = New System.Windows.Forms.Label()
        Me.lblModel = New System.Windows.Forms.Label()
        Me.lblYear = New System.Windows.Forms.Label()
        Me.lblMake = New System.Windows.Forms.Label()
        Me.lblPolicyNumber = New System.Windows.Forms.Label()
        Me.lblInsuredName = New System.Windows.Forms.Label()
        Me.chHideLayer = New System.Windows.Forms.CheckBox()
        Me.lstAnnotationLayer = New System.Windows.Forms.ListBox()
        Me.pictHorzontalBar2 = New System.Windows.Forms.PictureBox()
        Me.pictVehicleBox = New System.Windows.Forms.PictureBox()
        Me.pictHorzontalBar3 = New System.Windows.Forms.PictureBox()
        Me.pictPolicyBox = New System.Windows.Forms.PictureBox()
        Me.pictHorzontalBar1 = New System.Windows.Forms.PictureBox()
        Me.lblInstructions = New System.Windows.Forms.Label()
        Me.lblTitle = New System.Windows.Forms.Label()
        Me.pictCheckmarks = New System.Windows.Forms.PictureBox()
        Me.SuspendLayout()
        '
        'InkEdModel
        '
        Me.InkEdModel.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle
        Me.InkEdModel.Cursor = System.Windows.Forms.Cursors.Default
        Me.InkEdModel.Font = New System.Drawing.Font("Microsoft Sans Serif", 8.25!)
        Me.InkEdModel.Location = New System.Drawing.Point(216, 363)
        Me.InkEdModel.Name = "InkEdModel"
        Me.InkEdModel.Size = New System.Drawing.Size(320, 32)
        Me.InkEdModel.TabIndex = 66
        Me.InkEdModel.Text = ""
        Me.InkEdModel.UseMouseForInput = True
        '
        'InkEdMake
        '
        Me.InkEdMake.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle
        Me.InkEdMake.Cursor = System.Windows.Forms.Cursors.Default
        Me.InkEdMake.Font = New System.Drawing.Font("Microsoft Sans Serif", 8.25!)
        Me.InkEdMake.Location = New System.Drawing.Point(216, 323)
        Me.InkEdMake.Name = "InkEdMake"
        Me.InkEdMake.Size = New System.Drawing.Size(320, 32)
        Me.InkEdMake.TabIndex = 65
        Me.InkEdMake.Text = ""
        Me.InkEdMake.UseMouseForInput = True
        '
        'InkEdYear
        '
        Me.InkEdYear.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle
        Me.InkEdYear.Cursor = System.Windows.Forms.Cursors.Default
        Me.InkEdYear.Font = New System.Drawing.Font("Microsoft Sans Serif", 8.25!)
        Me.InkEdYear.Location = New System.Drawing.Point(216, 283)
        Me.InkEdYear.Name = "InkEdYear"
        Me.InkEdYear.Size = New System.Drawing.Size(320, 32)
        Me.InkEdYear.TabIndex = 64
        Me.InkEdYear.Text = ""
        Me.InkEdYear.UseMouseForInput = True
        '
        'inkEdName
        '
        Me.inkEdName.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle
        Me.inkEdName.Cursor = System.Windows.Forms.Cursors.Default
        Me.inkEdName.Font = New System.Drawing.Font("Microsoft Sans Serif", 8.25!)
        Me.inkEdName.Location = New System.Drawing.Point(216, 179)
        Me.inkEdName.Name = "inkEdName"
        Me.inkEdName.Size = New System.Drawing.Size(320, 32)
        Me.inkEdName.TabIndex = 62
        Me.inkEdName.Text = ""
        Me.inkEdName.UseMouseForInput = True
        '
        'inkEdPolicyNumber
        '
        Me.inkEdPolicyNumber.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle
        Me.inkEdPolicyNumber.Cursor = System.Windows.Forms.Cursors.Default
        Me.inkEdPolicyNumber.Font = New System.Drawing.Font("Microsoft Sans Serif", 8.25!)
        Me.inkEdPolicyNumber.Location = New System.Drawing.Point(216, 131)
        Me.inkEdPolicyNumber.Name = "inkEdPolicyNumber"
        Me.inkEdPolicyNumber.Size = New System.Drawing.Size(320, 32)
        Me.inkEdPolicyNumber.TabIndex = 61
        Me.inkEdPolicyNumber.Text = ""
        Me.inkEdPolicyNumber.UseMouseForInput = True
        '
        'inkPictVehicle
        '
        Me.inkPictVehicle.AutoRedraw = False
        Me.inkPictVehicle.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle
        Me.inkPictVehicle.Cursor = System.Windows.Forms.Cursors.Default
        Me.inkPictVehicle.Image = CType(resources.GetObject("inkPictVehicle.Image"), System.Drawing.Bitmap)
        Me.inkPictVehicle.InkEnabled = False
        Me.inkPictVehicle.Location = New System.Drawing.Point(24, 435)
        Me.inkPictVehicle.MarginX = -1
        Me.inkPictVehicle.MarginY = -1
        Me.inkPictVehicle.Name = "inkPictVehicle"
        Me.inkPictVehicle.Size = New System.Drawing.Size(392, 256)
        Me.inkPictVehicle.SizeMode = System.Windows.Forms.PictureBoxSizeMode.StretchImage
        Me.inkPictVehicle.TabIndex = 60
        Me.inkPictVehicle.TabStop = False
        '
        'lblVehicleDamage
        '
        Me.lblVehicleDamage.AutoSize = True
        Me.lblVehicleDamage.BackColor = System.Drawing.Color.White
        Me.lblVehicleDamage.Font = New System.Drawing.Font("Tahoma", 12.0!, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.lblVehicleDamage.ImageAlign = System.Drawing.ContentAlignment.MiddleRight
        Me.lblVehicleDamage.Location = New System.Drawing.Point(24, 411)
        Me.lblVehicleDamage.Name = "lblVehicleDamage"
        Me.lblVehicleDamage.Size = New System.Drawing.Size(256, 20)
        Me.lblVehicleDamage.TabIndex = 57
        Me.lblVehicleDamage.Text = "Description of vehicle damage"
        Me.lblVehicleDamage.TextAlign = System.Drawing.ContentAlignment.MiddleRight
        '
        'lblVehicleDescription
        '
        Me.lblVehicleDescription.AutoSize = True
        Me.lblVehicleDescription.BackColor = System.Drawing.Color.White
        Me.lblVehicleDescription.Font = New System.Drawing.Font("Tahoma", 12.0!, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.lblVehicleDescription.ImageAlign = System.Drawing.ContentAlignment.MiddleRight
        Me.lblVehicleDescription.Location = New System.Drawing.Point(24, 243)
        Me.lblVehicleDescription.Name = "lblVehicleDescription"
        Me.lblVehicleDescription.Size = New System.Drawing.Size(386, 20)
        Me.lblVehicleDescription.TabIndex = 55
        Me.lblVehicleDescription.Text = "Enter the involved vehicle listed on the policy "
        Me.lblVehicleDescription.TextAlign = System.Drawing.ContentAlignment.MiddleRight
        '
        'lblModel
        '
        Me.lblModel.AutoSize = True
        Me.lblModel.BackColor = System.Drawing.Color.Gainsboro
        Me.lblModel.Font = New System.Drawing.Font("Tahoma", 14.25!, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.lblModel.ImageAlign = System.Drawing.ContentAlignment.MiddleRight
        Me.lblModel.Location = New System.Drawing.Point(125, 363)
        Me.lblModel.Name = "lblModel"
        Me.lblModel.Size = New System.Drawing.Size(73, 23)
        Me.lblModel.TabIndex = 54
        Me.lblModel.Text = "Model:"
        Me.lblModel.TextAlign = System.Drawing.ContentAlignment.MiddleRight
        '
        'lblYear
        '
        Me.lblYear.AutoSize = True
        Me.lblYear.BackColor = System.Drawing.Color.Gainsboro
        Me.lblYear.Font = New System.Drawing.Font("Tahoma", 14.25!, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.lblYear.ImageAlign = System.Drawing.ContentAlignment.MiddleRight
        Me.lblYear.Location = New System.Drawing.Point(63, 283)
        Me.lblYear.Name = "lblYear"
        Me.lblYear.Size = New System.Drawing.Size(135, 23)
        Me.lblYear.TabIndex = 53
        Me.lblYear.Text = "Year (YYYY):"
        Me.lblYear.TextAlign = System.Drawing.ContentAlignment.MiddleRight
        '
        'lblMake
        '
        Me.lblMake.AutoSize = True
        Me.lblMake.BackColor = System.Drawing.Color.Gainsboro
        Me.lblMake.Font = New System.Drawing.Font("Tahoma", 14.25!, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.lblMake.ImageAlign = System.Drawing.ContentAlignment.MiddleRight
        Me.lblMake.Location = New System.Drawing.Point(131, 323)
        Me.lblMake.Name = "lblMake"
        Me.lblMake.Size = New System.Drawing.Size(67, 23)
        Me.lblMake.TabIndex = 52
        Me.lblMake.Text = "Make:"
        Me.lblMake.TextAlign = System.Drawing.ContentAlignment.MiddleRight
        '
        'lblPolicyNumber
        '
        Me.lblPolicyNumber.AutoSize = True
        Me.lblPolicyNumber.BackColor = System.Drawing.Color.Gainsboro
        Me.lblPolicyNumber.Font = New System.Drawing.Font("Tahoma", 14.25!, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.lblPolicyNumber.ImageAlign = System.Drawing.ContentAlignment.MiddleRight
        Me.lblPolicyNumber.Location = New System.Drawing.Point(44, 139)
        Me.lblPolicyNumber.Name = "lblPolicyNumber"
        Me.lblPolicyNumber.Size = New System.Drawing.Size(157, 23)
        Me.lblPolicyNumber.TabIndex = 49
        Me.lblPolicyNumber.Text = "Policy Number:"
        Me.lblPolicyNumber.TextAlign = System.Drawing.ContentAlignment.MiddleRight
        '
        'lblInsuredName
        '
        Me.lblInsuredName.AutoSize = True
        Me.lblInsuredName.BackColor = System.Drawing.Color.Gainsboro
        Me.lblInsuredName.Font = New System.Drawing.Font("Tahoma", 14.25!, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.lblInsuredName.ImageAlign = System.Drawing.ContentAlignment.MiddleRight
        Me.lblInsuredName.Location = New System.Drawing.Point(48, 179)
        Me.lblInsuredName.Name = "lblInsuredName"
        Me.lblInsuredName.Size = New System.Drawing.Size(154, 23)
        Me.lblInsuredName.TabIndex = 48
        Me.lblInsuredName.Text = "Insured Name:"
        Me.lblInsuredName.TextAlign = System.Drawing.ContentAlignment.MiddleRight
        '
        'chHideLayer
        '
        Me.chHideLayer.Font = New System.Drawing.Font("Tahoma", 12.0!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.chHideLayer.Location = New System.Drawing.Point(432, 579)
        Me.chHideLayer.Name = "chHideLayer"
        Me.chHideLayer.TabIndex = 59
        Me.chHideLayer.Text = "Hide Layer"
        '
        'lstAnnotationLayer
        '
        Me.lstAnnotationLayer.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle
        Me.lstAnnotationLayer.Font = New System.Drawing.Font("Tahoma", 12.0!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.lstAnnotationLayer.ItemHeight = 19
        Me.lstAnnotationLayer.Items.AddRange(New Object() {"Body", "Windows", "Tires", "Headlights"})
        Me.lstAnnotationLayer.Location = New System.Drawing.Point(432, 435)
        Me.lstAnnotationLayer.Name = "lstAnnotationLayer"
        Me.lstAnnotationLayer.Size = New System.Drawing.Size(104, 135)
        Me.lstAnnotationLayer.TabIndex = 58
        '
        'pictHorzontalBar2
        '
        Me.pictHorzontalBar2.BackColor = System.Drawing.SystemColors.AppWorkspace
        Me.pictHorzontalBar2.Location = New System.Drawing.Point(24, 227)
        Me.pictHorzontalBar2.Name = "pictHorzontalBar2"
        Me.pictHorzontalBar2.Size = New System.Drawing.Size(536, 2)
        Me.pictHorzontalBar2.TabIndex = 56
        Me.pictHorzontalBar2.TabStop = False
        '
        'pictVehicleBox
        '
        Me.pictVehicleBox.BackColor = System.Drawing.Color.Gainsboro
        Me.pictVehicleBox.Location = New System.Drawing.Point(24, 275)
        Me.pictVehicleBox.Name = "pictVehicleBox"
        Me.pictVehicleBox.Size = New System.Drawing.Size(184, 120)
        Me.pictVehicleBox.TabIndex = 51
        Me.pictVehicleBox.TabStop = False
        '
        'pictHorzontalBar3
        '
        Me.pictHorzontalBar3.BackColor = System.Drawing.SystemColors.AppWorkspace
        Me.pictHorzontalBar3.Location = New System.Drawing.Point(24, 403)
        Me.pictHorzontalBar3.Name = "pictHorzontalBar3"
        Me.pictHorzontalBar3.Size = New System.Drawing.Size(536, 2)
        Me.pictHorzontalBar3.TabIndex = 50
        Me.pictHorzontalBar3.TabStop = False
        '
        'pictPolicyBox
        '
        Me.pictPolicyBox.BackColor = System.Drawing.Color.Gainsboro
        Me.pictPolicyBox.Location = New System.Drawing.Point(24, 131)
        Me.pictPolicyBox.Name = "pictPolicyBox"
        Me.pictPolicyBox.Size = New System.Drawing.Size(184, 80)
        Me.pictPolicyBox.TabIndex = 47
        Me.pictPolicyBox.TabStop = False
        '
        'pictHorzontalBar1
        '
        Me.pictHorzontalBar1.BackColor = System.Drawing.SystemColors.AppWorkspace
        Me.pictHorzontalBar1.Location = New System.Drawing.Point(24, 115)
        Me.pictHorzontalBar1.Name = "pictHorzontalBar1"
        Me.pictHorzontalBar1.Size = New System.Drawing.Size(536, 2)
        Me.pictHorzontalBar1.TabIndex = 46
        Me.pictHorzontalBar1.TabStop = False
        '
        'lblInstructions
        '
        Me.lblInstructions.Font = New System.Drawing.Font("Tahoma", 11.25!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.lblInstructions.Location = New System.Drawing.Point(88, 51)
        Me.lblInstructions.Name = "lblInstructions"
        Me.lblInstructions.Size = New System.Drawing.Size(456, 56)
        Me.lblInstructions.TabIndex = 45
        Me.lblInstructions.Text = "Enter the required information for the policy applicable to the auto claim you ar" & _
        "e submitting. Enter the vehicle exactly as it appears on the policy or insurance" & _
        " vehicled."
        '
        'lblTitle
        '
        Me.lblTitle.Font = New System.Drawing.Font("Tahoma", 21.75!, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.lblTitle.Location = New System.Drawing.Point(24, 11)
        Me.lblTitle.Name = "lblTitle"
        Me.lblTitle.Size = New System.Drawing.Size(288, 32)
        Me.lblTitle.TabIndex = 44
        Me.lblTitle.Text = "Auto Claim"
        '
        'pictCheckmarks
        '
        Me.pictCheckmarks.BackgroundImage = CType(resources.GetObject("pictCheckmarks.BackgroundImage"), System.Drawing.Bitmap)
        Me.pictCheckmarks.Location = New System.Drawing.Point(24, 43)
        Me.pictCheckmarks.Name = "pictCheckmarks"
        Me.pictCheckmarks.Size = New System.Drawing.Size(40, 72)
        Me.pictCheckmarks.TabIndex = 63
        Me.pictCheckmarks.TabStop = False
        '
        'AutoClaims
        '
        Me.AutoScaleBaseSize = New System.Drawing.Size(5, 13)
        Me.BackColor = System.Drawing.Color.White
        Me.ClientSize = New System.Drawing.Size(584, 702)
        Me.Controls.AddRange(New System.Windows.Forms.Control() {Me.InkEdModel, Me.InkEdMake, Me.InkEdYear, Me.inkEdName, Me.inkEdPolicyNumber, Me.inkPictVehicle, Me.lblVehicleDamage, Me.lblVehicleDescription, Me.lblModel, Me.lblYear, Me.lblMake, Me.lblPolicyNumber, Me.lblInsuredName, Me.chHideLayer, Me.lstAnnotationLayer, Me.pictHorzontalBar2, Me.pictVehicleBox, Me.pictHorzontalBar3, Me.pictPolicyBox, Me.pictHorzontalBar1, Me.lblInstructions, Me.lblTitle, Me.pictCheckmarks})
        Me.Name = "AutoClaims"
        Me.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen
        Me.Text = "AutoClaims"
        Me.ResumeLayout(False)

    End Sub

#End Region

#Region " Standard Template Code "
    Public Shared Sub Main()
        Application.Run(New AutoClaims())
    End Sub
#End Region

    ''''''''''''''''''''''''''''''''''''''''''''''''''
    ' Event Handler for frmAutoClaim Load Event 
    '
    ' This method occurs when the form is loaded.  It
    ' configures the insertion mode and the ink used by
    ' this application.
    ''''''''''''''''''''''''''''''''''''''''''''''''''
    Private Sub AutoClaims_Load(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles MyBase.Load

        ' Initialize the empty ink
        emptyInk = New Ink()

        ' Initialize the four different layers of ink on the vehicle diagram:  
        ' vehicle body, windows, tires, and headlights.
        inkLayers(0) = New InkLayer(New Ink(), Color.Red, False)
        inkLayers(1) = New InkLayer(New Ink(), Color.Violet, False)
        inkLayers(2) = New InkLayer(New Ink(), Color.LightGreen, False)
        inkLayers(3) = New InkLayer(New Ink(), Color.Aqua, False)

        ' No ink is selected initially
        selectedIndex = -1

        ' By default, select the first ink layer
        lstAnnotationLayer.SelectedIndex = 0
        inkPictVehicle.DefaultDrawingAttributes.Color = inkLayers(lstAnnotationLayer.SelectedIndex).ActiveColor

        ' Currently there are recognizers that do not support regular expression
        ' input scopes, thus raising exceptions.  This try catch handler copes with
        ' these differences in the recognizer. For policy numbers, nothing is being
        ' done in reaction to this exception though since there is no factoid that 
        ' serves the same purpose.
        Try
            inkEdPolicyNumber.Factoid = "(0|1|2|3|4|5|6|7|8|9)(0|1|2|3|4|5|6|7|8|9)(0|1|2|3|4|5|6|7|8|9)-(0|1|2|3|4|5|6|7|" & _
                                                   "8|9)(0|1|2|3|4|5|6|7|8|9)(0|1|2|3|4|5|6|7|8|9)(0|1|2|3|4|5|6|7|8|9)"
        Catch
            ' Do nothing
        End Try

        ' There are also recognizers that do not support common input scopes as well.
        ' This try catch handler copes with these differences in the recognizer. For 
        ' year, the NUMBER factoid is being set in reaction to this exception.
        Try
            InkEdYear.Factoid = "(!IS_DATE_YEAR)"
        Catch
            InkEdYear.Factoid = Factoid.Number
        End Try

    End Sub

    ''''''''''''''''''''''''''''''''''''''''''''''''''
    ' Event Handler for lstAnnotationLayer SelectedIndexChanged Event
    ''''''''''''''''''''''''''''''''''''''''''''''''''
    Private Sub lstAnnotationLayer_SelectedIndexChanged(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles lstAnnotationLayer.SelectedIndexChanged

        ' Provided that the new selected index value is different than
        ' the previous value...
        If (Not (lstAnnotationLayer.SelectedIndex = selectedIndex)) Then
            If (Not inkPictVehicle.CollectingInk) Then

                ' Set the ink and visiblity of the current ink layer
                inkPictVehicle.DefaultDrawingAttributes.Color = inkLayers(lstAnnotationLayer.SelectedIndex).ActiveColor
                chHideLayer.Checked = inkLayers(lstAnnotationLayer.SelectedIndex).Hidden

                ' Set the active ink object to the selected ink
                ' Note that if the current layer is not visible, empty
                ' ink is used to prevent flicker.
                inkPictVehicle.InkEnabled = False
                If (chHideLayer.Checked) Then
                    inkPictVehicle.Ink = emptyInk
                Else
                    inkPictVehicle.Ink = inkLayers(lstAnnotationLayer.SelectedIndex).ActiveInk
                End If
                inkPictVehicle.InkEnabled = Not chHideLayer.Checked

                ' Update the previous selected ink value to the current
                selectedIndex = lstAnnotationLayer.SelectedIndex

                Me.Refresh()
            Else
                ' If ink collection is enabled, the active ink cannot be changed
                ' and it is necessary to restore the selection to its previous value.
                lstAnnotationLayer.SelectedIndex = selectedIndex
                MessageBox.Show("Cannot change active ink while collecting ink.")
            End If
        End If
    End Sub

    ''''''''''''''''''''''''''''''''''''''''''''''''''
    ' Event Handler for chHideLayer CheckChanged Event
    ''''''''''''''''''''''''''''''''''''''''''''''''''
    Private Sub chHideLayer_CheckedChanged(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles chHideLayer.CheckedChanged

        ' Provided that the new checked hidden value is different than
        ' the previous value...
        If (Not (chHideLayer.Checked = selectedHidden)) Then
            If (Not (inkPictVehicle.CollectingInk)) Then

                ' Update the array indicating the visibility of each ink layer
                inkLayers(lstAnnotationLayer.SelectedIndex).Hidden = chHideLayer.Checked

                ' Set the active ink object to the selected ink
                ' Note that if the current layer is not visible, empty
                ' ink is used to prevent flicker.
                inkPictVehicle.InkEnabled = False
                If (chHideLayer.Checked) Then
                    inkPictVehicle.Ink = emptyInk
                Else
                    inkPictVehicle.Ink = inkLayers(lstAnnotationLayer.SelectedIndex).ActiveInk
                End If

                ' Update the previous checkbox value to the current
                selectedHidden = chHideLayer.Checked

                ' If the layer is marked hidden, disable ink collection
                inkPictVehicle.InkEnabled = Not chHideLayer.Checked

                Me.Refresh()
            Else
                ' If ink collection is enabled, the active ink cannot be changed
                ' and it is necessary to restore the checkbox to its previous value.
                chHideLayer.Checked = selectedHidden
                MessageBox.Show("Cannot change visiblity while collecting ink.")
            End If
        End If
    End Sub

    ''''''''''''''''''''''''''''''''''''''''''''''''''
    ' Event Handler for inkPictVehicle Paint Event
    ''''''''''''''''''''''''''''''''''''''''''''''''''
    Private Sub inkPictVehicle_Paint(ByVal sender As System.Object, ByVal e As System.Windows.Forms.PaintEventArgs) Handles inkPictVehicle.Paint

        Dim layer As InkLayer

        ' Cycle through each ink layer.  If it is visible, paint it.
        ' Note that it is necessary to customize the paint
        ' behavior, since we want to hide/show different ink layers.
        For Each layer In inkLayers
            If (Not layer.Hidden) Then
                inkPictVehicle.Renderer.Draw(e.Graphics, layer.ActiveInk.Strokes)
            End If
        Next
    End Sub

End Class



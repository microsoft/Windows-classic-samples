' Copyright (c) Microsoft Corporation. All rights reserved.

' Dialog box for selecting a media item from the Media Library.

Imports System
Imports System.Drawing
Imports System.Collections
Imports System.ComponentModel
Imports System.Windows.Forms
Imports System.IO
Imports System.Runtime.InteropServices
Imports WMPLib

Public Class Library
    Inherits System.Windows.Forms.Form


#Region " Windows Form Designer generated code "

    Public Sub New()
        MyBase.New()
        InitializeComponent()
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

    Private components As System.ComponentModel.IContainer
    Private WithEvents listBox1 As System.Windows.Forms.ListBox
    Private WithEvents btnCancel As System.Windows.Forms.Button
    Private WithEvents btnOK As System.Windows.Forms.Button
    Friend WithEvents GroupBox1 As System.Windows.Forms.GroupBox
    Friend WithEvents radioAudio As System.Windows.Forms.RadioButton
    Friend WithEvents radioVideo As System.Windows.Forms.RadioButton

    <System.Diagnostics.DebuggerStepThrough()> Private Sub InitializeComponent()
        Me.listBox1 = New System.Windows.Forms.ListBox
        Me.btnCancel = New System.Windows.Forms.Button
        Me.btnOK = New System.Windows.Forms.Button
        Me.GroupBox1 = New System.Windows.Forms.GroupBox
        Me.radioVideo = New System.Windows.Forms.RadioButton
        Me.radioAudio = New System.Windows.Forms.RadioButton
        Me.GroupBox1.SuspendLayout()
        Me.SuspendLayout()
        '
        'listBox1
        '
        Me.listBox1.Location = New System.Drawing.Point(24, 16)
        Me.listBox1.Name = "listBox1"
        Me.listBox1.Size = New System.Drawing.Size(248, 199)
        Me.listBox1.TabIndex = 0
        '
        'btnCancel
        '
        Me.btnCancel.DialogResult = System.Windows.Forms.DialogResult.Cancel
        Me.btnCancel.Location = New System.Drawing.Point(296, 168)
        Me.btnCancel.Name = "btnCancel"
        Me.btnCancel.Size = New System.Drawing.Size(75, 32)
        Me.btnCancel.TabIndex = 1
        Me.btnCancel.Text = "&Cancel"
        '
        'btnOK
        '
        Me.btnOK.Location = New System.Drawing.Point(296, 120)
        Me.btnOK.Name = "btnOK"
        Me.btnOK.Size = New System.Drawing.Size(75, 32)
        Me.btnOK.TabIndex = 2
        Me.btnOK.Text = "&OK"
        '
        'GroupBox1
        '
        Me.GroupBox1.Controls.Add(Me.radioVideo)
        Me.GroupBox1.Controls.Add(Me.radioAudio)
        Me.GroupBox1.Location = New System.Drawing.Point(280, 16)
        Me.GroupBox1.Name = "GroupBox1"
        Me.GroupBox1.Size = New System.Drawing.Size(104, 88)
        Me.GroupBox1.TabIndex = 3
        Me.GroupBox1.TabStop = False
        Me.GroupBox1.Text = "Type"
        '
        'radioVideo
        '
        Me.radioVideo.Location = New System.Drawing.Point(16, 56)
        Me.radioVideo.Name = "radioVideo"
        Me.radioVideo.Size = New System.Drawing.Size(64, 16)
        Me.radioVideo.TabIndex = 1
        Me.radioVideo.Text = "Video"
        '
        'radioAudio
        '
        Me.radioAudio.Checked = True
        Me.radioAudio.Location = New System.Drawing.Point(16, 24)
        Me.radioAudio.Name = "radioAudio"
        Me.radioAudio.Size = New System.Drawing.Size(64, 16)
        Me.radioAudio.TabIndex = 0
        Me.radioAudio.TabStop = True
        Me.radioAudio.Text = "Audio"
        '
        'Library
        '
        Me.AutoScaleBaseSize = New System.Drawing.Size(5, 13)
        Me.CancelButton = Me.btnCancel
        Me.ClientSize = New System.Drawing.Size(410, 232)
        Me.Controls.Add(Me.GroupBox1)
        Me.Controls.Add(Me.btnOK)
        Me.Controls.Add(Me.btnCancel)
        Me.Controls.Add(Me.listBox1)
        Me.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog
        Me.MaximizeBox = False
        Me.MinimizeBox = False
        Me.Name = "Library"
        Me.Text = "Choose Media"
        Me.GroupBox1.ResumeLayout(False)
        Me.ResumeLayout(False)

    End Sub

#End Region

    Private Player As AxWMPLib.AxWindowsMediaPlayer
    Private selectedMedia As IWMPMedia
    Private mediaPlaylist As IWMPPlaylist

    ' Property to return the selected media item.

    Public ReadOnly Property TheMedia() As WMPLib.IWMPMedia
        Get
            Return selectedMedia
        End Get
    End Property

    ' Property to set the Player object from the main form class.

    Public WriteOnly Property WMPlayer() As AxWMPLib.AxWindowsMediaPlayer
        Set(ByVal Value As AxWMPLib.AxWindowsMediaPlayer)
            Player = Value
        End Set
    End Property

    ' Fill the list box with the names of media items from the Library.

    Private Sub Fill_listBox1()
        Try
            listBox1.Items.Clear()
            Dim x As Integer = 0
            Dim Count As Integer = mediaPlaylist.count - 1

            For x = 0 To Count
                Dim media As IWMPMedia = mediaPlaylist.Item(x)
                listBox1.Items.Add(media.name)
            Next x

        Catch comExc As COMException
            Dim hr As Integer = comExc.ErrorCode
            Dim Message As String = String.Format("There was an error.\nHRESULT = {1}\n{2}", hr.ToString(), comExc.Message)
            MessageBox.Show(Message, "COM Exception")
        End Try
    End Sub

    Private Sub Library_Load(ByVal sender As Object, ByVal e As System.EventArgs) Handles MyBase.Load
        radioAudio_CheckedChanged(Nothing, Nothing)
    End Sub

    ' Handler for OK button. Close the dialog box and return OK.

    Private Sub btnOK_Click(ByVal sender As Object, ByVal e As System.EventArgs) Handles btnOK.Click
        Me.DialogResult = DialogResult.OK
        Me.Close()
    End Sub


    ' Handle Cancel button. Close the dialog box and return Cancel.

    Private Sub btnCancel_Click(ByVal sender As Object, ByVal e As System.EventArgs) Handles btnCancel.Click
        Me.DialogResult = DialogResult.Cancel
        Close()
    End Sub


    ' Select media item.

    Private Sub listBox1_SelectedIndexChanged(ByVal sender As Object, ByVal e As System.EventArgs) Handles listBox1.SelectedIndexChanged
        Try
            ' Retrieve the selected item as an IWMPMedia.
            selectedMedia = mediaPlaylist.Item(listBox1.SelectedIndex)
        Catch comExc As COMException
            Dim hr As Integer = comExc.ErrorCode
            Dim Message As String = String.Format("There was an error.\nHRESULT = {1}\n{2}", hr.ToString(), comExc.Message)
            MessageBox.Show(Message, "COM Exception")
        End Try
    End Sub

    ' Double-click. Select media item and close dialog.

    Private Sub listBox1_DoubleClick(ByVal sender As Object, ByVal e As System.EventArgs) Handles listBox1.DoubleClick
        btnOK_Click(Me, Nothing)
    End Sub

    Private Sub radioAudio_CheckedChanged(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles radioAudio.CheckedChanged
        Try
            'Show the wait cursor in case this is a lengthy operation.
            Cursor.Current = Cursors.WaitCursor

            mediaPlaylist = Player.mediaCollection.getByAttribute("MediaType", "audio")
            Fill_listBox1()

            ' Restore the default cursor.
            Cursor.Current = Cursors.Default
        Catch nullExc As NullReferenceException
            'No Player object yet
        Catch comExc As COMException
            Dim hr As Integer = comExc.ErrorCode
            Dim Message As String = String.Format("There was an error.\nHRESULT = {1}\n{2}", hr.ToString(), comExc.Message)
            MessageBox.Show(Message, "COM Exception")
        End Try
    End Sub

    Private Sub radioVideo_CheckedChanged(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles radioVideo.CheckedChanged
        Try
            'Show the wait cursor in case this is a lengthy operation.
            Cursor.Current = Cursors.WaitCursor

            mediaPlaylist = Player.mediaCollection.getByAttribute("MediaType", "video")
            Fill_listBox1()

            ' Restore the default cursor.
            Cursor.Current = Cursors.Default
        Catch comExc As COMException
            Dim hr As Integer = comExc.ErrorCode
            Dim Message As String = String.Format("There was an error.\nHRESULT = {1}\n{2}", hr.ToString(), comExc.Message)
            MessageBox.Show(Message, "COM Exception")
        End Try
    End Sub

End Class

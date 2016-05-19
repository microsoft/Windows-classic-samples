' Copyright (c) Microsoft Corporation. All rights reserved.

' This sample application shows how to use the Windows Media Player control in 
' Windows Forms. The control has been placed on Form1 from the Toolbox.
' 
' Functionality demonstrated includes:
' -- using Player controls such as play, pause, stop, fastForward, fastReverse and currentPosition.
' -- getting properties for a media item, such as name, duration, and sourceURL.
' -- handling events from the Player, including error events.
' -- retrieving media items from the Library.
'
' Note about catching COMExceptions: In C++, calls to Windows Media Player SDK methods return HRESULTs. When a method
' returns a failed HRESULT, C++ code should check the value and respond accordingly. When using COM interop to call 
' Windows Media Player SDK methods, failed HRESULTs are raised as COMExceptions. This sample code simply wraps each call 
' to Windows Media Player in a try block and then displays an error message when a COMException is raised. 
' For your own code, you should make decisions about how to handle such error conditions.
'
' Note about localization: This sample uses strings that assume the user is in the "en-US" culture. 


Imports System
Imports System.Drawing
Imports System.Collections
Imports System.ComponentModel
Imports System.Windows.Forms
Imports System.Data
Imports System.Runtime.InteropServices
Imports AxWMPLib
Imports WMPLib

Public Class Form1
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

    Private WithEvents btnPlayPause As System.Windows.Forms.Button
    Private WithEvents btnStop As System.Windows.Forms.Button
    Private WithEvents btnOpen As System.Windows.Forms.Button
    Private WithEvents btnFF As System.Windows.Forms.Button
    Private WithEvents btnRew As System.Windows.Forms.Button
    Private lblSongName As System.Windows.Forms.Label
    Private lblOpenState As System.Windows.Forms.Label
    Private label1 As System.Windows.Forms.Label
    Private label2 As System.Windows.Forms.Label
    Private label3 As System.Windows.Forms.Label
    Private label4 As System.Windows.Forms.Label
    Private lblSourceURL As System.Windows.Forms.Label
    Private lblPlayState As System.Windows.Forms.Label
    Private lblTime As System.Windows.Forms.Label
    Private WithEvents tbSeek As System.Windows.Forms.TrackBar
    Private components As System.ComponentModel.IContainer
    Private WithEvents Player As AxWMPLib.AxWindowsMediaPlayer
    Private WithEvents Timer1 As System.Windows.Forms.Timer

    <System.Diagnostics.DebuggerStepThrough()> Private Sub InitializeComponent()
        Me.components = New System.ComponentModel.Container
        Dim resources As System.Resources.ResourceManager = New System.Resources.ResourceManager(GetType(Form1))
        Me.btnPlayPause = New System.Windows.Forms.Button
        Me.btnStop = New System.Windows.Forms.Button
        Me.btnOpen = New System.Windows.Forms.Button
        Me.label1 = New System.Windows.Forms.Label
        Me.lblSongName = New System.Windows.Forms.Label
        Me.lblOpenState = New System.Windows.Forms.Label
        Me.label2 = New System.Windows.Forms.Label
        Me.label3 = New System.Windows.Forms.Label
        Me.lblPlayState = New System.Windows.Forms.Label
        Me.tbSeek = New System.Windows.Forms.TrackBar
        Me.lblSourceURL = New System.Windows.Forms.Label
        Me.lblTime = New System.Windows.Forms.Label
        Me.btnFF = New System.Windows.Forms.Button
        Me.btnRew = New System.Windows.Forms.Button
        Me.label4 = New System.Windows.Forms.Label
        Me.Player = New AxWMPLib.AxWindowsMediaPlayer
        Me.Timer1 = New System.Windows.Forms.Timer(Me.components)
        CType(Me.tbSeek, System.ComponentModel.ISupportInitialize).BeginInit()
        CType(Me.Player, System.ComponentModel.ISupportInitialize).BeginInit()
        Me.SuspendLayout()
        '
        'btnPlayPause
        '
        Me.btnPlayPause.Enabled = False
        Me.btnPlayPause.Location = New System.Drawing.Point(24, 176)
        Me.btnPlayPause.Name = "btnPlayPause"
        Me.btnPlayPause.Size = New System.Drawing.Size(48, 23)
        Me.btnPlayPause.TabIndex = 1
        Me.btnPlayPause.Text = "&Play"
        '
        'btnStop
        '
        Me.btnStop.Enabled = False
        Me.btnStop.Location = New System.Drawing.Point(72, 176)
        Me.btnStop.Name = "btnStop"
        Me.btnStop.Size = New System.Drawing.Size(48, 23)
        Me.btnStop.TabIndex = 2
        Me.btnStop.Text = "&Stop"
        '
        'btnOpen
        '
        Me.btnOpen.Location = New System.Drawing.Point(416, 128)
        Me.btnOpen.Name = "btnOpen"
        Me.btnOpen.TabIndex = 0
        Me.btnOpen.Text = "&Open..."
        '
        'label1
        '
        Me.label1.Location = New System.Drawing.Point(232, 16)
        Me.label1.Name = "label1"
        Me.label1.Size = New System.Drawing.Size(40, 24)
        Me.label1.TabIndex = 4
        Me.label1.Text = "Song:"
        '
        'lblSongName
        '
        Me.lblSongName.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D
        Me.lblSongName.Location = New System.Drawing.Point(272, 16)
        Me.lblSongName.Name = "lblSongName"
        Me.lblSongName.Size = New System.Drawing.Size(224, 16)
        Me.lblSongName.TabIndex = 5
        '
        'lblOpenState
        '
        Me.lblOpenState.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D
        Me.lblOpenState.Location = New System.Drawing.Point(312, 176)
        Me.lblOpenState.Name = "lblOpenState"
        Me.lblOpenState.Size = New System.Drawing.Size(192, 23)
        Me.lblOpenState.TabIndex = 6
        '
        'label2
        '
        Me.label2.Location = New System.Drawing.Point(248, 176)
        Me.label2.Name = "label2"
        Me.label2.Size = New System.Drawing.Size(64, 16)
        Me.label2.TabIndex = 7
        Me.label2.Text = "OpenState:"
        '
        'label3
        '
        Me.label3.Location = New System.Drawing.Point(248, 200)
        Me.label3.Name = "label3"
        Me.label3.Size = New System.Drawing.Size(64, 23)
        Me.label3.TabIndex = 8
        Me.label3.Text = "Play State:"
        '
        'lblPlayState
        '
        Me.lblPlayState.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D
        Me.lblPlayState.Location = New System.Drawing.Point(312, 200)
        Me.lblPlayState.Name = "lblPlayState"
        Me.lblPlayState.Size = New System.Drawing.Size(192, 23)
        Me.lblPlayState.TabIndex = 9
        '
        'tbSeek
        '
        Me.tbSeek.Location = New System.Drawing.Point(8, 208)
        Me.tbSeek.Maximum = 100
        Me.tbSeek.Name = "tbSeek"
        Me.tbSeek.Size = New System.Drawing.Size(224, 45)
        Me.tbSeek.TabIndex = 5
        Me.tbSeek.TickFrequency = 0
        Me.tbSeek.TickStyle = System.Windows.Forms.TickStyle.None
        '
        'lblSourceURL
        '
        Me.lblSourceURL.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D
        Me.lblSourceURL.Location = New System.Drawing.Point(272, 48)
        Me.lblSourceURL.Name = "lblSourceURL"
        Me.lblSourceURL.Size = New System.Drawing.Size(224, 64)
        Me.lblSourceURL.TabIndex = 11
        '
        'lblTime
        '
        Me.lblTime.Location = New System.Drawing.Point(72, 256)
        Me.lblTime.Name = "lblTime"
        Me.lblTime.Size = New System.Drawing.Size(96, 16)
        Me.lblTime.TabIndex = 12
        Me.lblTime.Text = "00:00"
        Me.lblTime.TextAlign = System.Drawing.ContentAlignment.MiddleCenter
        '
        'btnFF
        '
        Me.btnFF.Enabled = False
        Me.btnFF.Location = New System.Drawing.Point(168, 176)
        Me.btnFF.Name = "btnFF"
        Me.btnFF.Size = New System.Drawing.Size(48, 23)
        Me.btnFF.TabIndex = 4
        Me.btnFF.Text = ">>"
        '
        'btnRew
        '
        Me.btnRew.Enabled = False
        Me.btnRew.Location = New System.Drawing.Point(120, 176)
        Me.btnRew.Name = "btnRew"
        Me.btnRew.Size = New System.Drawing.Size(48, 23)
        Me.btnRew.TabIndex = 3
        Me.btnRew.Text = "<<"
        '
        'label4
        '
        Me.label4.Location = New System.Drawing.Point(232, 48)
        Me.label4.Name = "label4"
        Me.label4.Size = New System.Drawing.Size(32, 23)
        Me.label4.TabIndex = 15
        Me.label4.Text = "URL:"
        '
        'Player
        '
        Me.Player.Enabled = True
        Me.Player.Location = New System.Drawing.Point(24, 8)
        Me.Player.Name = "Player"
        Me.Player.OcxState = CType(resources.GetObject("Player.OcxState"), System.Windows.Forms.AxHost.State)
        Me.Player.Size = New System.Drawing.Size(192, 152)
        Me.Player.TabIndex = 0
        Me.Player.TabStop = False
        '
        'Timer1
        '
        Me.Timer1.Interval = 250
        '
        'Form1
        '
        Me.AutoScaleBaseSize = New System.Drawing.Size(5, 13)
        Me.ClientSize = New System.Drawing.Size(520, 278)
        Me.Controls.Add(Me.Player)
        Me.Controls.Add(Me.label4)
        Me.Controls.Add(Me.btnRew)
        Me.Controls.Add(Me.btnFF)
        Me.Controls.Add(Me.lblTime)
        Me.Controls.Add(Me.lblSourceURL)
        Me.Controls.Add(Me.tbSeek)
        Me.Controls.Add(Me.lblPlayState)
        Me.Controls.Add(Me.label3)
        Me.Controls.Add(Me.label2)
        Me.Controls.Add(Me.lblOpenState)
        Me.Controls.Add(Me.lblSongName)
        Me.Controls.Add(Me.label1)
        Me.Controls.Add(Me.btnOpen)
        Me.Controls.Add(Me.btnStop)
        Me.Controls.Add(Me.btnPlayPause)
        Me.MaximizeBox = False
        Me.Name = "Form1"
        Me.Text = "Visual Basic .NET Windows Media Player Sample"
        CType(Me.tbSeek, System.ComponentModel.ISupportInitialize).EndInit()
        CType(Me.Player, System.ComponentModel.ISupportInitialize).EndInit()
        Me.ResumeLayout(False)

    End Sub

#End Region


    Dim theMedia As IWMPMedia = Nothing  ' Represents the current media to set. 


    ' Centralize UI state control.

    Private Sub SetUIState(ByVal newState As WMPPlayState)
        Dim canFF, canRew, canPlay, canPause, canSeek, canStop As Boolean
        Dim Duration As Double
        Try
            canFF = Player.Ctlcontrols.isAvailable("fastForward")
            canRew = Player.Ctlcontrols.isAvailable("fastReverse")
            canPlay = Player.Ctlcontrols.isAvailable("Play")
            canPause = Player.Ctlcontrols.isAvailable("Pause")
            canSeek = Player.Ctlcontrols.isAvailable("currentPosition")
            canStop = Player.Ctlcontrols.isAvailable("Stop")
            Duration = Player.currentMedia.duration
        Catch comExc As COMException
            Dim hr As Integer = comExc.ErrorCode
            Dim Message As String = String.Format("There was an error.\nHRESULT = {1}\n{2}", hr.ToString(), comExc.Message)
            MessageBox.Show(Message, "COM Exception")
            Return
        End Try

        Select Case newState

            Case WMPPlayState.wmppsStopped
                btnStop.Enabled = canStop
                btnPlayPause.Enabled = canPlay
                btnPlayPause.Text = "&Play"
                btnFF.Enabled = canFF
                btnRew.Enabled = canRew
                Timer1.Stop()
                tbSeek.Value = 0
                lblTime.Text = "00:00"
                If canSeek = True Then
                    tbSeek.Enabled = True
                End If

            Case WMPPlayState.wmppsPlaying
                btnStop.Enabled = canStop
                btnPlayPause.Enabled = canPause
                btnPlayPause.Text = "&Pause"
                btnFF.Enabled = canFF
                btnRew.Enabled = canRew
                If canSeek = True Then
                    tbSeek.Enabled = True
                End If
                If Duration > 0 Then
                    Timer1.Start()
                End If


            Case WMPPlayState.wmppsPaused
                btnStop.Enabled = canStop
                btnPlayPause.Enabled = canPlay
                btnPlayPause.Text = "&Play"
                btnFF.Enabled = canFF
                btnRew.Enabled = canRew
                Timer1.Stop()
                If canSeek = True Then
                    tbSeek.Enabled = True
                End If

            Case WMPPlayState.wmppsReady
                btnStop.Enabled = False
                btnPlayPause.Enabled = False
                btnPlayPause.Text = "&Play"
                btnFF.Enabled = False
                btnRew.Enabled = False
                Timer1.Stop()
                tbSeek.Value = 0
                tbSeek.Enabled = False
                lblTime.Text = "00:00"

            Case WMPPlayState.wmppsScanForward
            Case WMPPlayState.wmppsScanReverse
                btnStop.Enabled = canStop
                btnPlayPause.Enabled = canPause
                btnPlayPause.Text = "&Play"
                btnRew.Enabled = False
                btnFF.Enabled = False
                If canSeek = True Then
                    tbSeek.Enabled = True
                End If
                If Duration > 0 Then
                    Timer1.Start()
                End If

            Case Else
                btnStop.Enabled = canStop
                btnPlayPause.Enabled = canPlay
                btnPlayPause.Text = "&Play"
                btnFF.Enabled = canFF
                btnRew.Enabled = canRew
                tbSeek.Enabled = canSeek
                If Duration > 0 Then
                    Timer1.Start()
                Else
                    Timer1.Stop()
                End If

        End Select
       
    End Sub

#Region " UI Element Events "

    ' Open new audio from the Library.

    Private Sub btnOpen_Click(ByVal sender As Object, ByVal e As System.EventArgs) Handles btnOpen.Click
        Dim libForm As Library = New Library
        libForm.WMPlayer = Player
        libForm.ShowDialog()
        If (libForm.DialogResult = DialogResult.OK) Then
            Try
                theMedia = libForm.TheMedia

                If Not IsNothing(theMedia) Then
                    lblSongName.Text = theMedia.name
                    lblSourceURL.Text = theMedia.sourceURL

                    ' Set the current media. It will play automatically.
                    Player.currentMedia = theMedia
                End If

            Catch comExc As COMException
                Dim hr As Integer = comExc.ErrorCode
                Dim Message As String = String.Format("There was an error.\nHRESULT = {1}\n{2}", hr.ToString(), comExc.Message)
                MessageBox.Show(Message, "COM Exception")
  
            End Try
        End If
    End Sub


    ' Play/Pause transport control button handler.

    Private Sub btnPlayPause_Click(ByVal sender As Object, ByVal e As System.EventArgs) Handles btnPlayPause.Click
        Try
            If (Player.playState = WMPPlayState.wmppsPlaying) Then
                Player.Ctlcontrols.pause()
            Else
                Player.Ctlcontrols.play()
            End If
        Catch comExc As COMException
            Dim hr As Integer = comExc.ErrorCode
            Dim Message As String = String.Format("There was an error.\nHRESULT = {1}\n{2}", hr.ToString(), comExc.Message)
            MessageBox.Show(Message, "COM Exception")
        End Try
    End Sub


    ' Stop button transport control.

    Private Sub btnStop_Click(ByVal sender As Object, ByVal e As System.EventArgs) Handles btnStop.Click

        Try
            Player.Ctlcontrols.stop()
        Catch comExc As COMException
            Dim hr As Integer = comExc.ErrorCode
            Dim Message As String = String.Format("There was an error.\nHRESULT = {1}\n{2}", hr.ToString(), comExc.Message)
            MessageBox.Show(Message, "COM Exception")
        End Try
    End Sub

    ' Rewind button click.

    Private Sub btnRew_Click(ByVal sender As Object, ByVal e As System.EventArgs) Handles btnRew.Click
        Try
            Player.Ctlcontrols.fastReverse()
        Catch comExc As COMException
            Dim hr As Integer = comExc.ErrorCode
            Dim Message As String = String.Format("There was an error.\nHRESULT = {1}\n{2}", hr.ToString(), comExc.Message)
            MessageBox.Show(Message, "COM Exception")
        End Try
    End Sub

    ' Fast-forward button click.

    Private Sub btnFF_Click(ByVal sender As Object, ByVal e As System.EventArgs) Handles btnFF.Click
        Try
            Player.Ctlcontrols.fastForward()
        Catch comExc As COMException
            Dim hr As Integer = comExc.ErrorCode
            Dim Message As String = String.Format("There was an error.\nHRESULT = {1}\n{2}", hr.ToString(), comExc.Message)
            MessageBox.Show(Message, "COM Exception")
        End Try
    End Sub

    ' The user moved the trackbar. Set the new position.
    ' The trackbar has a fixed resolution of 100 ticks.
    ' This code maps that resolution to the file duration.

    Private Sub tbSeek_Scroll(ByVal sender As Object, ByVal e As System.EventArgs) Handles tbSeek.Scroll
        Try
            If (Player.currentMedia.duration <> 0) Then
                Dim newPerc As Double = Convert.ToDouble(tbSeek.Value) / 100
                Dim duration As Integer = Convert.ToInt32(Player.currentMedia.duration * 1000) ' milliseconds
                Dim newPos As Integer = (duration * newPerc) / 1000 ' seconds

                ' Seek the Player
                Player.Ctlcontrols.currentPosition = newPos
            Else
                tbSeek.Value = 0 'No duration available. Just ground the slider.
            End If
        Catch comExc As COMException
            Dim hr As Integer = comExc.ErrorCode
            Dim Message As String = String.Format("There was an error.\nHRESULT = {1}\n{2}", hr.ToString(), comExc.Message)
            MessageBox.Show(Message, "COM Exception")
        End Try
    End Sub

    ' Update the UI based on current media position.
    ' The trackbar has a fixed resolution of 100 ticks.
    ' This code maps that resolution to the file duration.

    Private Sub tmrPos_Tick(ByVal sender As Object, ByVal e As System.EventArgs) Handles Timer1.Tick
        Try

            ' Update the trackbar.
            Dim curPos As Integer = Convert.ToInt32(Player.Ctlcontrols.currentPosition * 1000) ' milliseconds
            Dim duration As Integer = Convert.ToInt32(Player.currentMedia.duration * 1000) ' milliseconds
            If duration > 0 Then
                tbSeek.Value = Convert.ToInt32((curPos * 100) / duration) ' % complete
            End If

            ' Update the time label
            lblTime.Text = Player.Ctlcontrols.currentPositionString

        Catch comExc As COMException
            Dim hr As Integer = comExc.ErrorCode
            Dim Message As String = String.Format("There was an error.\nHRESULT = {1}\n{2}", hr.ToString(), comExc.Message)
            MessageBox.Show(Message, "COM Exception")
        End Try
    End Sub

#End Region

#Region " Windows Media Player Events "

    ' Handle the Player.OpenStateChange event.

    Private Sub Player_OpenStateChange(ByVal sender As System.Object, ByVal e As AxWMPLib._WMPOCXEvents_OpenStateChangeEvent) Handles Player.OpenStateChange

        Select Case e.newState

            Case WMPOpenState.wmposBeginCodecAcquisition
                lblOpenState.Text = "Beginning codec acquisition..."

            Case WMPOpenState.wmposBeginIndividualization
                lblOpenState.Text = "Beginning individualization..."

            Case WMPOpenState.wmposBeginLicenseAcquisition
                lblOpenState.Text = "Beginning license acquisition..."

            Case WMPOpenState.wmposMediaChanging
                lblOpenState.Text = "Media changing..."

            Case WMPOpenState.wmposMediaConnecting
                lblOpenState.Text = "Media connecting..."

            Case WMPOpenState.wmposMediaLoading
                lblOpenState.Text = "Media loading..."

            Case WMPOpenState.wmposMediaLocating
                lblOpenState.Text = "Media locating..."

            Case WMPOpenState.wmposMediaOpen
                lblOpenState.Text = "Media open"

            Case WMPOpenState.wmposMediaWaiting
                lblOpenState.Text = "Media waiting..."

            Case WMPOpenState.wmposOpeningUnknownURL
                lblOpenState.Text = "Opening unknown URL..."

            Case WMPOpenState.wmposPlaylistChanged
                lblOpenState.Text = "Playlist changed..."

            Case WMPOpenState.wmposPlaylistChanging
                lblOpenState.Text = "Playlist changing..."

            Case WMPOpenState.wmposPlaylistConnecting
                lblOpenState.Text = "Playlist connecting..."

            Case WMPOpenState.wmposPlaylistLoading
                lblOpenState.Text = "Playlist loading..."

            Case WMPOpenState.wmposPlaylistLocating
                lblOpenState.Text = "Playlist locating..."

            Case WMPOpenState.wmposPlaylistOpening
                lblOpenState.Text = "Playlist opening..."

            Case WMPOpenState.wmposPlaylistOpenNoMedia
                lblOpenState.Text = "Playlist open, no media"

            Case Else
                lblOpenState.Text = ""

        End Select
    End Sub


    ' Handle the Player.PlayStateChange event.

    Private Sub Player_PlayStateChange(ByVal sender As System.Object, ByVal e As AxWMPLib._WMPOCXEvents_PlayStateChangeEvent) Handles Player.PlayStateChange

        Dim NewState As WMPPlayState = e.newState

        Select Case e.newState

            Case WMPPlayState.wmppsBuffering
                lblPlayState.Text = "Buffering..."

            Case WMPPlayState.wmppsMediaEnded
                lblPlayState.Text = "Media ended"
                SetUIState(NewState)

            Case WMPPlayState.wmppsPaused
                lblPlayState.Text = "Paused"
                SetUIState(NewState)

            Case WMPPlayState.wmppsPlaying
                lblPlayState.Text = "Playing"
                SetUIState(NewState)

            Case WMPPlayState.wmppsReady
                lblPlayState.Text = "Ready"
                SetUIState(NewState)

            Case WMPPlayState.wmppsReconnecting
                lblPlayState.Text = "Reconnecting..."

            Case WMPPlayState.wmppsScanForward
                lblPlayState.Text = "Scannning forward..."
                SetUIState(NewState)

            Case WMPPlayState.wmppsScanReverse
                lblPlayState.Text = "Scanning backwards..."
                SetUIState(NewState)

            Case WMPPlayState.wmppsStopped
                lblPlayState.Text = "Stopped"
                SetUIState(NewState)
                btnStop.Enabled = False

            Case WMPPlayState.wmppsTransitioning
                lblPlayState.Text = "Transitioning..."

            Case WMPPlayState.wmppsWaiting
                lblPlayState.Text = "Waiting..."

            Case Else
                lblPlayState.Text = ""

        End Select
    End Sub


    ' Generic handler for Player error events.
    ' Information returned in the event arguments is limited to the media object
    ' that was the source of the error. Further information can be obtained from
    ' IWMPMedia2.Error. Use IWMErrorItem.Description with caution, because messages
    ' may be more relevant to the Windows Media Player than to your application.

    Private Sub Player_MediaError(ByVal sender As System.Object, ByVal e As AxWMPLib._WMPOCXEvents_MediaErrorEvent) Handles Player.MediaError
        Dim errSource As IWMPMedia2 = e.pMediaObject
        Dim errorItem As IWMPErrorItem = errSource.Error
        Dim errorDesc As String = errorItem.errorDescription
        Dim errorStr As String = "Error " + errorItem.errorCode.ToString("X") + " in " + errSource.sourceURL + _
            vbCrLf + errorDesc
        MessageBox.Show(errorStr, "Player Error")
    End Sub

#End Region

End Class




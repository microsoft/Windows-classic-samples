// Copyright (c) Microsoft Corporation. All rights reserved.

/* This sample application shows how to use the Windows Media Player control in 
 * Windows Forms. The control has been placed on Form1 from the Toolbox.
 * 
 * Functionality demonstrated includes:
 * -- using Player controls such as play, pause, stop, fastForward, fastReverse and currentPosition.
 * -- getting properties for a media item, such as name, duration, and sourceURL.
 * -- handling events from the Player, including error events.
 * -- retrieving media items from the Library.
 * 
 * Note about catching COMExceptions: In C++, calls to Windows Media Player SDK methods return HRESULTs. When a method
 * returns a failed HRESULT, C++ code should check the value and respond accordingly. When using COM interop to call 
 * Windows Media Player SDK methods, failed HRESULTs are raised as COMExceptions. This sample code simply wraps each call 
 * to Windows Media Player in a try block and then displays an error message when a COMException is raised. 
 * For your own code, you should make decisions about how to handle such error conditions.
 * 
 * Note about localization: This sample uses strings that assume the user is in the "en-US" culture. 
 */


using System;
using System.Drawing;
using System.Collections;
using System.ComponentModel;
using System.Windows.Forms;
using System.Data;
using System.Runtime.InteropServices;
using AxWMPLib;
using WMPLib;

namespace WMPCSharp
{
	/// <summary>
	/// Summary description for Form1.
	/// </summary>
	public class Form1 : System.Windows.Forms.Form
	{
        private AxWMPLib.AxWindowsMediaPlayer Player;
        private System.Windows.Forms.Button btnPlayPause;
        private System.Windows.Forms.Button btnStop;
        private System.Windows.Forms.Button btnOpen;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Label lblSongName;
        private System.Windows.Forms.Label lblOpenState;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.Label lblPlayState;
        private System.Windows.Forms.TrackBar tbSeek;
        private System.Windows.Forms.Timer tmrPos;
        private System.Windows.Forms.Label lblSourceURL;
        private System.Windows.Forms.Label lblTime;
        private System.Windows.Forms.Button btnFF;
        private System.Windows.Forms.Button btnRew;
        private System.Windows.Forms.Label label4;
        private System.ComponentModel.IContainer components;

		public Form1()
		{
			InitializeComponent();
		}

		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		protected override void Dispose( bool disposing )
		{
			if( disposing )
			{
				if (components != null) 
				{
					components.Dispose();
				}
			}
			base.Dispose( disposing );
		}

		#region Windows Form Designer generated code

		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent()
		{
            this.components = new System.ComponentModel.Container();
            System.Resources.ResourceManager resources = new System.Resources.ResourceManager(typeof(Form1));
            this.Player = new AxWMPLib.AxWindowsMediaPlayer();
            this.btnPlayPause = new System.Windows.Forms.Button();
            this.btnStop = new System.Windows.Forms.Button();
            this.btnOpen = new System.Windows.Forms.Button();
            this.label1 = new System.Windows.Forms.Label();
            this.lblSongName = new System.Windows.Forms.Label();
            this.lblOpenState = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.label3 = new System.Windows.Forms.Label();
            this.lblPlayState = new System.Windows.Forms.Label();
            this.tbSeek = new System.Windows.Forms.TrackBar();
            this.tmrPos = new System.Windows.Forms.Timer(this.components);
            this.lblSourceURL = new System.Windows.Forms.Label();
            this.lblTime = new System.Windows.Forms.Label();
            this.btnFF = new System.Windows.Forms.Button();
            this.btnRew = new System.Windows.Forms.Button();
            this.label4 = new System.Windows.Forms.Label();
            ((System.ComponentModel.ISupportInitialize)(this.Player)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.tbSeek)).BeginInit();
            this.SuspendLayout();
            // 
            // Player
            // 
            this.Player.Enabled = true;
            this.Player.Location = new System.Drawing.Point(24, 8);
            this.Player.Name = "Player";
            this.Player.OcxState = ((System.Windows.Forms.AxHost.State)(resources.GetObject("Player.OcxState")));
            this.Player.Size = new System.Drawing.Size(192, 152);
            this.Player.TabIndex = 0;
            this.Player.TabStop = false;
            this.Player.PlayStateChange += new AxWMPLib._WMPOCXEvents_PlayStateChangeEventHandler(this.Player_PlayStateChange);
            this.Player.OpenStateChange += new AxWMPLib._WMPOCXEvents_OpenStateChangeEventHandler(this.Player_OpenStateChange);
            this.Player.MediaError += new AxWMPLib._WMPOCXEvents_MediaErrorEventHandler(this.Player_MediaError);
            // 
            // btnPlayPause
            // 
            this.btnPlayPause.Enabled = false;
            this.btnPlayPause.Location = new System.Drawing.Point(24, 176);
            this.btnPlayPause.Name = "btnPlayPause";
            this.btnPlayPause.Size = new System.Drawing.Size(48, 23);
            this.btnPlayPause.TabIndex = 1;
            this.btnPlayPause.Text = "&Play";
            this.btnPlayPause.Click += new System.EventHandler(this.btnPlayPause_Click);
            // 
            // btnStop
            // 
            this.btnStop.Enabled = false;
            this.btnStop.Location = new System.Drawing.Point(72, 176);
            this.btnStop.Name = "btnStop";
            this.btnStop.Size = new System.Drawing.Size(48, 23);
            this.btnStop.TabIndex = 2;
            this.btnStop.Text = "&Stop";
            this.btnStop.Click += new System.EventHandler(this.btnStop_Click);
            // 
            // btnOpen
            // 
            this.btnOpen.Location = new System.Drawing.Point(416, 128);
            this.btnOpen.Name = "btnOpen";
            this.btnOpen.TabIndex = 0;
            this.btnOpen.Text = "&Open...";
            this.btnOpen.Click += new System.EventHandler(this.btnOpen_Click);
            // 
            // label1
            // 
            this.label1.Location = new System.Drawing.Point(232, 16);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(40, 24);
            this.label1.TabIndex = 6;
            this.label1.Text = "Song:";
            // 
            // lblSongName
            // 
            this.lblSongName.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
            this.lblSongName.Location = new System.Drawing.Point(272, 16);
            this.lblSongName.Name = "lblSongName";
            this.lblSongName.Size = new System.Drawing.Size(224, 16);
            this.lblSongName.TabIndex = 7;
            // 
            // lblOpenState
            // 
            this.lblOpenState.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
            this.lblOpenState.Location = new System.Drawing.Point(312, 176);
            this.lblOpenState.Name = "lblOpenState";
            this.lblOpenState.Size = new System.Drawing.Size(192, 23);
            this.lblOpenState.TabIndex = 11;
            // 
            // label2
            // 
            this.label2.Location = new System.Drawing.Point(248, 176);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(64, 16);
            this.label2.TabIndex = 10;
            this.label2.Text = "OpenState:";
            // 
            // label3
            // 
            this.label3.Location = new System.Drawing.Point(248, 200);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(64, 23);
            this.label3.TabIndex = 12;
            this.label3.Text = "Play State:";
            // 
            // lblPlayState
            // 
            this.lblPlayState.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
            this.lblPlayState.Location = new System.Drawing.Point(312, 200);
            this.lblPlayState.Name = "lblPlayState";
            this.lblPlayState.Size = new System.Drawing.Size(192, 23);
            this.lblPlayState.TabIndex = 13;
            // 
            // tbSeek
            // 
            this.tbSeek.Enabled = false;
            this.tbSeek.Location = new System.Drawing.Point(8, 208);
            this.tbSeek.Maximum = 100;
            this.tbSeek.Name = "tbSeek";
            this.tbSeek.Size = new System.Drawing.Size(224, 45);
            this.tbSeek.TabIndex = 5;
            this.tbSeek.TickFrequency = 0;
            this.tbSeek.TickStyle = System.Windows.Forms.TickStyle.None;
            this.tbSeek.Scroll += new System.EventHandler(this.tbSeek_Scroll);
            // 
            // tmrPos
            // 
            this.tmrPos.Interval = 250;
            this.tmrPos.Tick += new System.EventHandler(this.tmrPos_Tick);
            // 
            // lblSourceURL
            // 
            this.lblSourceURL.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
            this.lblSourceURL.Location = new System.Drawing.Point(272, 48);
            this.lblSourceURL.Name = "lblSourceURL";
            this.lblSourceURL.Size = new System.Drawing.Size(224, 64);
            this.lblSourceURL.TabIndex = 9;
            // 
            // lblTime
            // 
            this.lblTime.Location = new System.Drawing.Point(72, 256);
            this.lblTime.Name = "lblTime";
            this.lblTime.Size = new System.Drawing.Size(96, 16);
            this.lblTime.TabIndex = 12;
            this.lblTime.Text = "00:00";
            this.lblTime.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
            // 
            // btnFF
            // 
            this.btnFF.Enabled = false;
            this.btnFF.Location = new System.Drawing.Point(168, 176);
            this.btnFF.Name = "btnFF";
            this.btnFF.Size = new System.Drawing.Size(48, 23);
            this.btnFF.TabIndex = 4;
            this.btnFF.Text = ">>";
            this.btnFF.Click += new System.EventHandler(this.btnFF_Click);
            // 
            // btnRew
            // 
            this.btnRew.Enabled = false;
            this.btnRew.Location = new System.Drawing.Point(120, 176);
            this.btnRew.Name = "btnRew";
            this.btnRew.Size = new System.Drawing.Size(48, 23);
            this.btnRew.TabIndex = 3;
            this.btnRew.Text = "<<";
            this.btnRew.Click += new System.EventHandler(this.btnRew_Click);
            // 
            // label4
            // 
            this.label4.Location = new System.Drawing.Point(232, 48);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(32, 23);
            this.label4.TabIndex = 8;
            this.label4.Text = "URL:";
            // 
            // Form1
            // 
            this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
            this.ClientSize = new System.Drawing.Size(520, 278);
            this.Controls.Add(this.label4);
            this.Controls.Add(this.btnRew);
            this.Controls.Add(this.btnFF);
            this.Controls.Add(this.lblTime);
            this.Controls.Add(this.lblSourceURL);
            this.Controls.Add(this.tbSeek);
            this.Controls.Add(this.lblPlayState);
            this.Controls.Add(this.label3);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.lblOpenState);
            this.Controls.Add(this.lblSongName);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.btnOpen);
            this.Controls.Add(this.btnStop);
            this.Controls.Add(this.btnPlayPause);
            this.Controls.Add(this.Player);
            this.MaximizeBox = false;
            this.Name = "Form1";
            this.Text = "C# Windows Media Player Sample";
            ((System.ComponentModel.ISupportInitialize)(this.Player)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.tbSeek)).EndInit();
            this.ResumeLayout(false);

        }

		#endregion

  		/// <summary>
		/// The main entry point for the application.
		/// </summary>
		[STAThread]
		static void Main() 
		{
			Application.Run(new Form1());
		}

        IWMPMedia theMedia = null;  // Represents the current media to set.
  
        /// <summary>
        /// Centralize UI state control.
        /// </summary>
        /// <param name="newState">State of the player</param>
        private void SetUIState(WMPPlayState newState)
        {
            bool canFF, canRew, canPlay, canPause, canSeek, canStop;    
            double fDuration;

            try
            {
                canFF = Player.Ctlcontrols.get_isAvailable("fastForward");
                canRew = Player.Ctlcontrols.get_isAvailable("fastReverse");
                canPlay = Player.Ctlcontrols.get_isAvailable("Play");
                canPause = Player.Ctlcontrols.get_isAvailable("Pause");
                canSeek = Player.Ctlcontrols.get_isAvailable("currentPosition");
                canStop = Player.Ctlcontrols.get_isAvailable("Stop");
                fDuration = Player.currentMedia.duration;
            }
            catch(COMException comExc)
            {
                int hr = comExc.ErrorCode; 
                String Message = String.Format("There was an error.\nHRESULT = {1}\n{2}", hr.ToString(), comExc.Message);
                MessageBox.Show(Message, "COM Exception");
                return;
            } 
            
            switch (newState)
            {
                case WMPPlayState.wmppsStopped:
                    btnStop.Enabled = canStop;
                    btnPlayPause.Enabled = canPlay;
                    btnPlayPause.Text = "&Play";
                    btnFF.Enabled = canFF;
                    btnRew.Enabled = canRew;
                    tmrPos.Stop();
                    tbSeek.Value = 0;
                    lblTime.Text = "00:00";
                    if(canSeek)
                    {
                        tbSeek.Enabled = true;
                    }
                    break;

                case WMPPlayState.wmppsPlaying:
                    btnStop.Enabled = canStop;
                    btnPlayPause.Enabled = canPause;
                    btnPlayPause.Text = "&Pause";
                    btnFF.Enabled = canFF;
                    btnRew.Enabled = canRew;
                    if(canSeek)
                    {
                        tbSeek.Enabled = true;
                        
                    }
                    if(fDuration > 0)
                    {
                        tmrPos.Start();
                    }
                    break;

                case WMPPlayState.wmppsPaused:
                    btnStop.Enabled = canStop;
                    btnPlayPause.Enabled = canPlay;
                    btnPlayPause.Text = "&Play";
                    btnFF.Enabled = canFF;
                    btnRew.Enabled = canRew;
                    tmrPos.Stop(); 
                    if(canSeek)
                    {
                        tbSeek.Enabled = true;
                    }
                    break;

                case WMPPlayState.wmppsReady:
                    btnStop.Enabled = false;
                    btnPlayPause.Enabled = false;
                    btnPlayPause.Text = "&Play";
                    btnFF.Enabled = false;
                    btnRew.Enabled = false;
                    tmrPos.Stop();
                    tbSeek.Value = 0;
                    tbSeek.Enabled = false;
                    lblTime.Text = "00:00";
                    break;

                case WMPPlayState.wmppsScanForward:
                case WMPPlayState.wmppsScanReverse:
                    btnStop.Enabled = canStop;
                    btnPlayPause.Enabled = canPause;
                    btnPlayPause.Text = "&Play"; 
                    btnRew.Enabled = false;
                    btnFF.Enabled = false;
                    if(canSeek)
                    {
                        tbSeek.Enabled = true;                        
                    }
                    if(fDuration > 0)
                    {
                        tmrPos.Start();
                    }
                    break;

                default:
                    btnStop.Enabled = canStop;
                    btnPlayPause.Enabled = canPlay;
                    btnPlayPause.Text = "&Play";
                    btnFF.Enabled = canFF;
                    btnRew.Enabled = canRew;
                    tbSeek.Enabled = canSeek;
                    if(fDuration > 0)
                    {
                        tmrPos.Start();
                    }
                    else
                    {
                        tmrPos.Stop();
                    }
                    break;
            }            
        }

        #region UI Element Events

        /// <summary>
        /// Open new audio from the Library.
        /// </summary>
        private void btnOpen_Click(object sender, System.EventArgs e)
        {
            Library libForm = new Library();
            libForm.WMPlayer = Player;
            libForm.ShowDialog();
            if (libForm.DialogResult == DialogResult.OK)
            {
                try
                {
                    theMedia = libForm.TheMedia;

                    if(theMedia != null) // Test whether the user chose a media item. 
                    {
                        lblSongName.Text = theMedia.name;
                        lblSourceURL.Text = theMedia.sourceURL;

                        // Set the current media. It will play automatically.
                        Player.currentMedia = theMedia;
                    }
                }      
                catch(COMException comExc)
                {
                    int hr = comExc.ErrorCode; 
                    String Message = String.Format("There was an error.\nHRESULT = {1}\n{2}", hr.ToString(), comExc.Message);
                    MessageBox.Show(Message, "COM Exception");
                }  
             }
        }

        /// <summary>
        /// Play/Pause transport control button handler.
        /// </summary>
        private void btnPlayPause_Click(object sender, System.EventArgs e)
        {
            try
            {
                if(Player.playState == WMPPlayState.wmppsPlaying)
                {
                    Player.Ctlcontrols.pause();
                }
                else
                {
                    Player.Ctlcontrols.play();
                }
            }
            catch(COMException comExc)
            {
                int hr = comExc.ErrorCode; 
                String Message = String.Format("There was an error.\nHRESULT = {1}\n{2}", hr.ToString(), comExc.Message);
                MessageBox.Show(Message, "COM Exception");
            }  
        }

        /// <summary>
        /// Stop button transport control.
        /// </summary>
        private void btnStop_Click(object sender, System.EventArgs e)
        {
            try
            {
                Player.Ctlcontrols.stop();
            }
            catch(COMException comExc)
            {
                int hr = comExc.ErrorCode; 
                String Message = String.Format("There was an error.\nHRESULT = {1}\n{2}", hr.ToString(), comExc.Message);
                MessageBox.Show(Message, "COM Exception");
            }   
        }
        /// <summary>
        /// Rewind button transport control.
        /// </summary>
        private void btnRew_Click(object sender, System.EventArgs e)
        {
            try
            {
                Player.Ctlcontrols.fastReverse();
            }
            catch(COMException comExc)
            {
                int hr = comExc.ErrorCode; 
                String Message = String.Format("There was an error.\nHRESULT = {1}\n{2}", hr.ToString(), comExc.Message);
                MessageBox.Show(Message, "COM Exception");
            }   
        }

        /// <summary>
        /// Fast-forward button transport control.
        /// </summary>
        private void btnFF_Click(object sender, System.EventArgs e)
        {
            try
            {
                Player.Ctlcontrols.fastForward();
            }
            catch(COMException comExc)
            {
                int hr = comExc.ErrorCode; 
                String Message = String.Format("There was an error.\nHRESULT = {1}\n{2}", hr.ToString(), comExc.Message);
                MessageBox.Show(Message, "COM Exception");
            }               
        }

        /// <summary>
        /// The user moved the trackbar. Set the new position.
        /// The trackbar has a fixed resolution of 100 ticks.
        /// This code maps that resolution to the file duration.
        /// </summary>
        private void tbSeek_Scroll(object sender, System.EventArgs e)
        {
            try
            {
                if(Player.currentMedia.duration != 0)  
                {
                    double newPerc = Convert.ToDouble(tbSeek.Value) / 100;
                    int duration = Convert.ToInt32(Player.currentMedia.duration * 1000); // milliseconds
                    double newPos = (duration * newPerc)/1000; // seconds

                    // Seek the Player
                    Player.Ctlcontrols.currentPosition = newPos;
                }
                else
                {
                    tbSeek.Value = 0; //No duration available. Just ground the slider.
                }
            }
            catch(COMException comExc)
            {
                int hr = comExc.ErrorCode; 
                String Message = String.Format("There was an error.\nHRESULT = {1}\n{2}", hr.ToString(), comExc.Message);
                MessageBox.Show(Message, "COM Exception");
            } 
        } 
     
        /// <summary>
        /// Update the UI based on current media position.
        /// The trackbar has a fixed resolution of 100 ticks.
        /// This code maps that resolution to the file duration.
        /// </summary>
        private void tmrPos_Tick(object sender, System.EventArgs e)
        {
            try
            {              
                // Update the trackbar.
                int curPos = Convert.ToInt32(Player.Ctlcontrols.currentPosition * 1000); // milliseconds
                int duration = Convert.ToInt32(Player.currentMedia.duration * 1000);  // milliseconds 
                if(duration > 0)
                {
                    tbSeek.Value = Convert.ToInt32((curPos * 100) / duration);  // % complete
                }

                // Update the time label
                lblTime.Text = Player.Ctlcontrols.currentPositionString;
            }
            catch(COMException comExc)
            {
                int hr = comExc.ErrorCode; 
                String Message = String.Format("There was an error.\nHRESULT = {1}\n{2}", hr.ToString(), comExc.Message);
                MessageBox.Show(Message, "COM Exception");
            }           
        }


        #endregion
             
        #region Windows Media Player Events
        /// <summary>
        /// Handle the Player.OpenStateChange event.
        /// </summary>
        private void Player_OpenStateChange(object sender, AxWMPLib._WMPOCXEvents_OpenStateChangeEvent e)
        {
            switch((WMPOpenState)e.newState)
            { 
                case WMPOpenState.wmposBeginCodecAcquisition:
                    lblOpenState.Text = "Beginning codec acquisition...";
                    break;
                 
                case WMPOpenState.wmposBeginIndividualization:                 
                    lblOpenState.Text = "Beginning individualization...";
                    break;
                 
                case WMPOpenState.wmposBeginLicenseAcquisition:                 
                    lblOpenState.Text = "Beginning license acquisition...";
                    break;
                                 
                case WMPOpenState.wmposMediaChanging:                 
                    lblOpenState.Text = "Media changing...";
                    break;
                 
                case WMPOpenState.wmposMediaConnecting:                 
                    lblOpenState.Text = "Media connecting...";
                    break;
                 
                case WMPOpenState.wmposMediaLoading:                 
                    lblOpenState.Text = "Media loading...";
                    break;
                 
                case WMPOpenState.wmposMediaLocating:                 
                    lblOpenState.Text = "Media locating...";
                    break;
                 
                case WMPOpenState.wmposMediaOpen:                 
                    lblOpenState.Text = "Media open";
                    break;
                 
                case WMPOpenState.wmposMediaWaiting:                 
                    lblOpenState.Text = "Media waiting...";
                    break;
                 
                case WMPOpenState.wmposOpeningUnknownURL:                 
                    lblOpenState.Text = "Opening unknown URL...";
                    break;
                 
                case WMPOpenState.wmposPlaylistChanged:                 
                    lblOpenState.Text = "Playlist changed...";
                    break;
                 
                case WMPOpenState.wmposPlaylistChanging:                 
                    lblOpenState.Text = "Playlist changing...";
                    break;
                 
                case WMPOpenState.wmposPlaylistConnecting:                 
                    lblOpenState.Text = "Playlist connecting...";
                    break;
                 
                case WMPOpenState.wmposPlaylistLoading:                 
                    lblOpenState.Text = "Playlist loading...";
                    break;
                 
                case WMPOpenState.wmposPlaylistLocating:                 
                    lblOpenState.Text = "Playlist locating...";
                    break;
                 
                case WMPOpenState.wmposPlaylistOpening:                 
                    lblOpenState.Text = "Playlist opening...";
                    break;
                 
                case WMPOpenState.wmposPlaylistOpenNoMedia:                 
                    lblOpenState.Text = "Playlist open, no media";
                    break;
                   
                default:
                    lblOpenState.Text = "";
                    break;

            }
        }

        /// <summary>
        /// Handle the Player.PlayStateChange event.
        /// </summary>
        private void Player_PlayStateChange(object sender, AxWMPLib._WMPOCXEvents_PlayStateChangeEvent e)
        {
            WMPPlayState newState = (WMPPlayState)e.newState;
            switch(newState)
            {
                case WMPPlayState.wmppsBuffering:
                    lblPlayState.Text = "Buffering...";
                    break;

                case WMPPlayState.wmppsMediaEnded:
                    lblPlayState.Text = "Media ended";
                    SetUIState(newState);
                    break;

                case WMPPlayState.wmppsPaused:
                    lblPlayState.Text = "Paused";
                    SetUIState(newState);
                    break;

                case WMPPlayState.wmppsPlaying:
                    lblPlayState.Text = "Playing";
                    SetUIState(newState);
                    break;

                case WMPPlayState.wmppsReady:
                    lblPlayState.Text = "Ready";
                    SetUIState(newState);
                    break;

                case WMPPlayState.wmppsReconnecting:
                    lblPlayState.Text = "Reconnecting...";
                    break;

                case WMPPlayState.wmppsScanForward:
                    lblPlayState.Text = "Scanning forward...";
                    SetUIState(newState);
                    break;

                case WMPPlayState.wmppsScanReverse:
                    lblPlayState.Text = "Scanning backwards...";
                    SetUIState(newState);
                    break;

                case WMPPlayState.wmppsStopped:
                    lblPlayState.Text = "Stopped";
                    SetUIState(newState);
                    btnStop.Enabled = false;
                    break;

                case WMPPlayState.wmppsTransitioning:
                    lblPlayState.Text = "Transitioning...";
                    break;

                case WMPPlayState.wmppsWaiting:
                    lblPlayState.Text = "Waiting...";
                    break;

                default:
                    lblPlayState.Text = "";
                    break;
            }
        }

        /// <summary>
        /// Generic handler for Player error events.
        /// </summary>
        /// <remarks>
        /// Information returned in the event arguments is limited to the media object
        /// that was the source of the error. Further information can be obtained from
        /// IWMPMedia2.Error. Use IWMErrorItem.Description with caution, because messages
        /// may be more relevant to the Windows Media Player than to your application.
        /// </remarks>
        private void Player_MediaError(object sender, _WMPOCXEvents_MediaErrorEvent e)
        {
            IWMPMedia2 errSource = e.pMediaObject as IWMPMedia2;
            IWMPErrorItem errorItem = errSource.Error;
            String errorDesc = errorItem.errorDescription;
            String errorStr = "Error " + errorItem.errorCode.ToString("X") + " in " + errSource.sourceURL + "\n" + errorDesc;
            MessageBox.Show(errorStr, "Player Error");
        }
        #endregion

       
  	}
}

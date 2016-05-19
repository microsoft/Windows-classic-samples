// Copyright (c) Microsoft Corporation. All rights reserved.

using System;
using System.Drawing;
using System.Collections;
using System.ComponentModel;
using System.Windows.Forms;
using System.IO;
using System.Runtime.InteropServices;
using WMPLib;

namespace WMPCSharp
{
    /// <summary>
    /// Summary description for Library.
    /// </summary>
    public class Library : System.Windows.Forms.Form
    {
        private System.Windows.Forms.ListBox listBox1;
        private System.Windows.Forms.Button btnCancel;
        private System.Windows.Forms.Button btnOK;
        internal System.Windows.Forms.GroupBox GroupBox1;
        internal System.Windows.Forms.RadioButton radioVideo;
        internal System.Windows.Forms.RadioButton radioAudio;
        private System.ComponentModel.Container components = null;

        public Library()
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
                if(components != null)
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
            this.listBox1 = new System.Windows.Forms.ListBox();
            this.btnCancel = new System.Windows.Forms.Button();
            this.btnOK = new System.Windows.Forms.Button();
            this.GroupBox1 = new System.Windows.Forms.GroupBox();
            this.radioVideo = new System.Windows.Forms.RadioButton();
            this.radioAudio = new System.Windows.Forms.RadioButton();
            this.GroupBox1.SuspendLayout();
            this.SuspendLayout();
            // 
            // listBox1
            // 
            this.listBox1.Location = new System.Drawing.Point(24, 16);
            this.listBox1.Name = "listBox1";
            this.listBox1.Size = new System.Drawing.Size(248, 199);
            this.listBox1.TabIndex = 0;
            this.listBox1.DoubleClick += new System.EventHandler(this.listBox1_DoubleClick);
            this.listBox1.SelectedIndexChanged += new System.EventHandler(this.listBox1_SelectedIndexChanged);
            // 
            // btnCancel
            // 
            this.btnCancel.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.btnCancel.Location = new System.Drawing.Point(296, 168);
            this.btnCancel.Name = "btnCancel";
            this.btnCancel.Size = new System.Drawing.Size(75, 32);
            this.btnCancel.TabIndex = 1;
            this.btnCancel.Text = "&Cancel";
            this.btnCancel.Click += new System.EventHandler(this.btnCancel_Click);
            // 
            // btnOK
            // 
            this.btnOK.Location = new System.Drawing.Point(296, 120);
            this.btnOK.Name = "btnOK";
            this.btnOK.Size = new System.Drawing.Size(75, 32);
            this.btnOK.TabIndex = 2;
            this.btnOK.Text = "&OK";
            this.btnOK.Click += new System.EventHandler(this.btnOK_Click);
            // 
            // GroupBox1
            // 
            this.GroupBox1.Controls.Add(this.radioVideo);
            this.GroupBox1.Controls.Add(this.radioAudio);
            this.GroupBox1.Location = new System.Drawing.Point(280, 16);
            this.GroupBox1.Name = "GroupBox1";
            this.GroupBox1.Size = new System.Drawing.Size(104, 88);
            this.GroupBox1.TabIndex = 4;
            this.GroupBox1.TabStop = false;
            this.GroupBox1.Text = "Type";
            // 
            // radioVideo
            // 
            this.radioVideo.Location = new System.Drawing.Point(16, 56);
            this.radioVideo.Name = "radioVideo";
            this.radioVideo.Size = new System.Drawing.Size(64, 16);
            this.radioVideo.TabIndex = 1;
            this.radioVideo.Text = "&Video";
            this.radioVideo.CheckedChanged += new System.EventHandler(this.radioVideo_CheckedChanged);
            // 
            // radioAudio
            // 
            this.radioAudio.Checked = true;
            this.radioAudio.Location = new System.Drawing.Point(16, 24);
            this.radioAudio.Name = "radioAudio";
            this.radioAudio.Size = new System.Drawing.Size(64, 16);
            this.radioAudio.TabIndex = 0;
            this.radioAudio.TabStop = true;
            this.radioAudio.Text = "&Audio";
            this.radioAudio.CheckedChanged += new System.EventHandler(this.radioAudio_CheckedChanged);
            // 
            // Library
            // 
            this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
            this.ClientSize = new System.Drawing.Size(410, 232);
            this.Controls.Add(this.GroupBox1);
            this.Controls.Add(this.btnOK);
            this.Controls.Add(this.btnCancel);
            this.Controls.Add(this.listBox1);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.Name = "Library";
            this.Text = "Choose Media";
            this.Load += new System.EventHandler(this.Library_Load);
            this.GroupBox1.ResumeLayout(false);
            this.ResumeLayout(false);

        }
        #endregion

        private AxWMPLib.AxWindowsMediaPlayer Player;
        private IWMPMedia theMedia;
        private IWMPPlaylist mediaPlaylist;

        /// <summary>
        /// Property to return the selected media item.
        /// </summary>
        public WMPLib.IWMPMedia TheMedia
        {
            get { return theMedia; }
        } 
      
        /// <summary>
        /// Property to set the Player object from the main form class.
        /// </summary>
        public AxWMPLib.AxWindowsMediaPlayer WMPlayer
        {
            set { Player = value; }
        }

        ///<summary>
        //////Fill the list box with the names of media items from the Library.
        ///<summary>
        private void Fill_listBox1()
        {
            try
            {                
                listBox1.Items.Clear();

                int iCount = mediaPlaylist.count;

                for(int x = 0;x < iCount; x++)
                {
                    IWMPMedia media = mediaPlaylist.get_Item(x);
                    listBox1.Items.Add(media.name);
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
        /// Initialize the dialog box.
        /// </summary>
        private void Library_Load(object sender, System.EventArgs e)
        {
            radioAudio_CheckedChanged(null, null);
        }

        /// <summary>
        /// Handler for OK button. Close the dialog box and return OK.
        /// </summary>
        private void btnOK_Click(object sender, System.EventArgs e)
        {           
            this.DialogResult = DialogResult.OK;
            this.Close();
        }

        /// <summary>
        /// Handler for Cancel button. Close the dialog box and return Cancel.
        /// </summary>
        private void btnCancel_Click(object sender, System.EventArgs e)
        {
            this.DialogResult = DialogResult.Cancel;
            Close();
        }

        /// <summary>
        /// Select media item.
        /// </summary>
        private void listBox1_SelectedIndexChanged(object sender, System.EventArgs e)
        {
            try
            {
                // Retrieve the selected item as an IWMPMedia.
                theMedia = mediaPlaylist.get_Item(listBox1.SelectedIndex);
            }
            catch(COMException comExc)
            {
                int hr = comExc.ErrorCode; 
                String Message = String.Format("There was an error.\nHRESULT = {1}\n{2}", hr.ToString(), comExc.Message);
                MessageBox.Show(Message, "COM Exception");
            }  
        }

        /// <summary>
        /// Double-click. Select media item and close dialog.
        /// </summary>
        private void listBox1_DoubleClick(object sender, System.EventArgs e)
        {
            btnOK_Click(this, null);
        }

        private void radioAudio_CheckedChanged(object sender, System.EventArgs e)
        {
            try
            {
                // Show the wait cursor in case this is a lengthy operation.
                Cursor.Current = Cursors.WaitCursor;

                mediaPlaylist = Player.mediaCollection.getByAttribute("MediaType", "audio");
                Fill_listBox1();

                // Restore the default cursor.
                Cursor.Current = Cursors.Default;
            }
            catch(COMException comExc)
            {
                int hr = comExc.ErrorCode; 
                String Message = String.Format("There was an error.\nHRESULT = {1}\n{2}", hr.ToString(), comExc.Message);
                MessageBox.Show(Message, "COM Exception");
            }          
        }

        private void radioVideo_CheckedChanged(object sender, System.EventArgs e)
        {
            try
            {                
                // Show the wait cursor in case this is a lengthy operation.
                Cursor.Current = Cursors.WaitCursor;

                mediaPlaylist = Player.mediaCollection.getByAttribute("MediaType", "video");
                Fill_listBox1();
                
                // Restore the default cursor.
                Cursor.Current = Cursors.Default;
            }
            catch(COMException comExc)
            {
                int hr = comExc.ErrorCode; 
                String Message = String.Format("There was an error.\nHRESULT = {1}\n{2}", hr.ToString(), comExc.Message);
                MessageBox.Show(Message, "COM Exception");
            }  
        }
    }
}

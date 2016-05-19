// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

using System;
using System.Drawing;
using System.Collections;
using System.ComponentModel;
using System.Windows.Forms;
using System.Data;
using System.IO;
using System.Runtime.InteropServices;
using System.Runtime.Serialization;
using System.Text;
using WMPLib;

// Set assembly attributes.
[assembly: System.Runtime.InteropServices.ComVisible(false)]
[assembly: CLSCompliant(true)]

namespace SchemaReader
{
    /// <summary>    
    /// Form for SchemaReader sample.
    /// </summary>
    public class Form1 : System.Windows.Forms.Form
    {
        string strOutFile;
        bool bShowVals; // Show values in output.
        bool bShowReadOnly; // Show Read-only or Read/Write status.
        IWMPCdrom CD;  // Reference to the current CD/DVD.
        IWMPPlaylistCollection PLCollection;
        IWMPMediaCollection MediaCollection;
        IWMPCdromCollection CDCollection;
        IWMPLibraryServices LibSvcs;
        
        enum ReportType
        {
            none,
            AllSchemas,
            CDTOC,
            CDTrack,
            DVDTOC,
            DVDTitleChap,
            PlaylistCollection,
            CurrentMedia,
            CurrentPlaylist
        };

        enum AttributeSource
        {
            None,
            Audio, //Schema
            Video, //Schema
            Radio, //Schema
            Playlist, //Schema
            Other, //Schema
            Photo, //Schema
            CurrentMedia,
            CurrentPlaylist,
            CDPlaylist,
            CDTrack,
            DVDToc,
            PlaylistCollection
        }

        ReportType rtLast = ReportType.none;

        private AxWMPLib.AxWindowsMediaPlayer Player;
        private System.Windows.Forms.MainMenu mainMenu1;
        private System.Windows.Forms.MenuItem menuItem5;
        private System.Windows.Forms.MenuItem mnuLibraryAttributesByType;
        private System.Windows.Forms.MenuItem mnuCDTOC;
        private System.Windows.Forms.MenuItem mnuDVDTitleChapter;
        private System.Windows.Forms.MenuItem mnuCDTrack;
        private System.Windows.Forms.MenuItem mnuPlaylistCollection;
        private System.Windows.Forms.Button btnTop;
        private System.Windows.Forms.Button btnTitle;
        private System.Windows.Forms.ComboBox cmbDrives;
        private System.Windows.Forms.MenuItem menuItem3;
        private System.Windows.Forms.MenuItem menuItem6;
        private System.Windows.Forms.MenuItem mnuSaveAs;
        private System.Windows.Forms.CheckBox chkVals;
        private System.Windows.Forms.CheckBox chkReadOnly;
        private System.Windows.Forms.MenuItem mnuDVDTOC;
        private System.Windows.Forms.MenuItem mnuSave;
        private System.Windows.Forms.MenuItem mnuOpenMedia;
        private System.Windows.Forms.ListView listView1;
        private System.Windows.Forms.ColumnHeader colSchema;
        private System.Windows.Forms.ColumnHeader colAttribute;
        private System.Windows.Forms.ColumnHeader colWritable;
        private System.Windows.Forms.ColumnHeader colValue;
        private System.Windows.Forms.Label lblStatus;
        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.GroupBox groupBox2;
        private CheckBox chkLegacyMC;
        private GroupBox groupBox3;
        private GroupBox groupBox4;
        private MenuItem mnuCurrentMedia;
        private MenuItem mnuCurrentPlaylist;
        private IContainer components;

        public Form1()
        {
            UseWaitCursor = true;
            //
            // Required for Windows Form Designer support
            //
            InitializeComponent();

            // Initialize global variables
            PLCollection = Player.playlistCollection;
            GetMediaCollection();
            CDCollection = Player.cdromCollection;
            LibSvcs = (IWMPLibraryServices)Player.GetOcx();
        
            // Fill in the combo boxes.
            enumerateCDDrives();

            // Show library attributes listing by default.
            mnuLibraryAttributes_Click(this, null);

            UseWaitCursor = false;
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
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(Form1));
            this.mainMenu1 = new System.Windows.Forms.MainMenu(this.components);
            this.menuItem3 = new System.Windows.Forms.MenuItem();
            this.mnuSave = new System.Windows.Forms.MenuItem();
            this.mnuSaveAs = new System.Windows.Forms.MenuItem();
            this.mnuOpenMedia = new System.Windows.Forms.MenuItem();
            this.menuItem5 = new System.Windows.Forms.MenuItem();
            this.mnuLibraryAttributesByType = new System.Windows.Forms.MenuItem();
            this.mnuCDTOC = new System.Windows.Forms.MenuItem();
            this.mnuCDTrack = new System.Windows.Forms.MenuItem();
            this.mnuDVDTOC = new System.Windows.Forms.MenuItem();
            this.mnuDVDTitleChapter = new System.Windows.Forms.MenuItem();
            this.mnuPlaylistCollection = new System.Windows.Forms.MenuItem();
            this.mnuCurrentMedia = new System.Windows.Forms.MenuItem();
            this.mnuCurrentPlaylist = new System.Windows.Forms.MenuItem();
            this.menuItem6 = new System.Windows.Forms.MenuItem();
            this.btnTop = new System.Windows.Forms.Button();
            this.btnTitle = new System.Windows.Forms.Button();
            this.cmbDrives = new System.Windows.Forms.ComboBox();
            this.chkVals = new System.Windows.Forms.CheckBox();
            this.chkReadOnly = new System.Windows.Forms.CheckBox();
            this.listView1 = new System.Windows.Forms.ListView();
            this.colSchema = new System.Windows.Forms.ColumnHeader();
            this.colAttribute = new System.Windows.Forms.ColumnHeader();
            this.colValue = new System.Windows.Forms.ColumnHeader();
            this.colWritable = new System.Windows.Forms.ColumnHeader();
            this.lblStatus = new System.Windows.Forms.Label();
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.groupBox2 = new System.Windows.Forms.GroupBox();
            this.chkLegacyMC = new System.Windows.Forms.CheckBox();
            this.groupBox3 = new System.Windows.Forms.GroupBox();
            this.groupBox4 = new System.Windows.Forms.GroupBox();
            this.Player = new AxWMPLib.AxWindowsMediaPlayer();
            this.groupBox1.SuspendLayout();
            this.groupBox2.SuspendLayout();
            this.groupBox3.SuspendLayout();
            this.groupBox4.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.Player)).BeginInit();
            this.SuspendLayout();
            // 
            // mainMenu1
            // 
            this.mainMenu1.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
            this.menuItem3,
            this.menuItem5,
            this.menuItem6});
            // 
            // menuItem3
            // 
            this.menuItem3.Index = 0;
            this.menuItem3.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
            this.mnuSave,
            this.mnuSaveAs,
            this.mnuOpenMedia});
            this.menuItem3.Text = "File";
            // 
            // mnuSave
            // 
            this.mnuSave.Index = 0;
            this.mnuSave.Text = "Save";
            this.mnuSave.Click += new System.EventHandler(this.mnuSave_Click);
            // 
            // mnuSaveAs
            // 
            this.mnuSaveAs.Index = 1;
            this.mnuSaveAs.Text = "Save as...";
            this.mnuSaveAs.Click += new System.EventHandler(this.mnuSaveAs_Click);
            // 
            // mnuOpenMedia
            // 
            this.mnuOpenMedia.Index = 2;
            this.mnuOpenMedia.Text = "Open media";
            this.mnuOpenMedia.Click += new System.EventHandler(this.mnuOpenMedia_Click);
            // 
            // menuItem5
            // 
            this.menuItem5.Index = 1;
            this.menuItem5.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
            this.mnuLibraryAttributesByType,
            this.mnuCDTOC,
            this.mnuCDTrack,
            this.mnuDVDTOC,
            this.mnuDVDTitleChapter,
            this.mnuPlaylistCollection,
            this.mnuCurrentMedia,
            this.mnuCurrentPlaylist});
            this.menuItem5.Text = "Capture";
            // 
            // mnuLibraryAttributesByType
            // 
            this.mnuLibraryAttributesByType.Index = 0;
            this.mnuLibraryAttributesByType.Text = "Library Attributes by Media Type";
            this.mnuLibraryAttributesByType.Click += new System.EventHandler(this.mnuLibraryAttributes_Click);
            // 
            // mnuCDTOC
            // 
            this.mnuCDTOC.Index = 1;
            this.mnuCDTOC.Text = "CD TOC Attributes";
            this.mnuCDTOC.Click += new System.EventHandler(this.mnuCDTOC_Click);
            // 
            // mnuCDTrack
            // 
            this.mnuCDTrack.Index = 2;
            this.mnuCDTrack.Text = "CD Track Attributes";
            this.mnuCDTrack.Click += new System.EventHandler(this.mnuCDTrack_Click);
            // 
            // mnuDVDTOC
            // 
            this.mnuDVDTOC.Index = 3;
            this.mnuDVDTOC.Text = "DVD TOC Attributes";
            this.mnuDVDTOC.Click += new System.EventHandler(this.mnuDVDTOC_Click);
            // 
            // mnuDVDTitleChapter
            // 
            this.mnuDVDTitleChapter.Index = 4;
            this.mnuDVDTitleChapter.Text = "DVD Title/Chapter Attributes";
            this.mnuDVDTitleChapter.Click += new System.EventHandler(this.mnuDVDTitleChapter_Click);
            // 
            // mnuPlaylistCollection
            // 
            this.mnuPlaylistCollection.Index = 5;
            this.mnuPlaylistCollection.Text = "PlaylistCollection Attributes";
            this.mnuPlaylistCollection.Click += new System.EventHandler(this.mnuPlaylistCollection_Click);
            // 
            // mnuCurrentMedia
            // 
            this.mnuCurrentMedia.Index = 6;
            this.mnuCurrentMedia.Text = "Current Media Attributes";
            this.mnuCurrentMedia.Click += new System.EventHandler(this.mnuCurrentMedia_Click);
            // 
            // mnuCurrentPlaylist
            // 
            this.mnuCurrentPlaylist.Index = 7;
            this.mnuCurrentPlaylist.Text = "Current Playlist Attributes";
            this.mnuCurrentPlaylist.Click += new System.EventHandler(this.mnuCurrentPlaylist_Click);
            // 
            // menuItem6
            // 
            this.menuItem6.Index = 2;
            this.menuItem6.Text = "";
            // 
            // btnTop
            // 
            this.btnTop.Location = new System.Drawing.Point(40, 24);
            this.btnTop.Name = "btnTop";
            this.btnTop.Size = new System.Drawing.Size(75, 23);
            this.btnTop.TabIndex = 2;
            this.btnTop.Text = "Top Menu";
            this.btnTop.Click += new System.EventHandler(this.btnTop_Click);
            // 
            // btnTitle
            // 
            this.btnTitle.Location = new System.Drawing.Point(40, 56);
            this.btnTitle.Name = "btnTitle";
            this.btnTitle.Size = new System.Drawing.Size(75, 23);
            this.btnTitle.TabIndex = 3;
            this.btnTitle.Text = "Title Menu";
            this.btnTitle.Click += new System.EventHandler(this.btnTitle_Click);
            // 
            // cmbDrives
            // 
            this.cmbDrives.Location = new System.Drawing.Point(34, 38);
            this.cmbDrives.Name = "cmbDrives";
            this.cmbDrives.Size = new System.Drawing.Size(64, 21);
            this.cmbDrives.TabIndex = 4;
            this.cmbDrives.SelectedIndexChanged += new System.EventHandler(this.cmbDrives_SelectedIndexChanged);
            // 
            // chkVals
            // 
            this.chkVals.Location = new System.Drawing.Point(16, 56);
            this.chkVals.Name = "chkVals";
            this.chkVals.Size = new System.Drawing.Size(104, 24);
            this.chkVals.TabIndex = 5;
            this.chkVals.Text = "Values";
            this.chkVals.CheckedChanged += new System.EventHandler(this.chkVals_CheckedChanged);
            // 
            // chkReadOnly
            // 
            this.chkReadOnly.Location = new System.Drawing.Point(16, 32);
            this.chkReadOnly.Name = "chkReadOnly";
            this.chkReadOnly.Size = new System.Drawing.Size(104, 24);
            this.chkReadOnly.TabIndex = 6;
            this.chkReadOnly.Text = "R/O Status";
            this.chkReadOnly.CheckedChanged += new System.EventHandler(this.chkReadOnly_CheckedChanged);
            // 
            // listView1
            // 
            this.listView1.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.colSchema,
            this.colAttribute,
            this.colValue,
            this.colWritable});
            this.listView1.GridLines = true;
            this.listView1.Location = new System.Drawing.Point(8, 216);
            this.listView1.Name = "listView1";
            this.listView1.Size = new System.Drawing.Size(640, 272);
            this.listView1.TabIndex = 8;
            this.listView1.UseCompatibleStateImageBehavior = false;
            this.listView1.View = System.Windows.Forms.View.Details;
            // 
            // colSchema
            // 
            this.colSchema.Text = "Schema";
            this.colSchema.Width = 80;
            // 
            // colAttribute
            // 
            this.colAttribute.Text = "Attribute";
            this.colAttribute.Width = 150;
            // 
            // colValue
            // 
            this.colValue.Text = "Value";
            this.colValue.Width = 330;
            // 
            // colWritable
            // 
            this.colWritable.Text = "ReadOnly";
            // 
            // lblStatus
            // 
            this.lblStatus.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
            this.lblStatus.Location = new System.Drawing.Point(8, 496);
            this.lblStatus.Name = "lblStatus";
            this.lblStatus.Size = new System.Drawing.Size(640, 24);
            this.lblStatus.TabIndex = 9;
            // 
            // groupBox1
            // 
            this.groupBox1.Controls.Add(this.btnTop);
            this.groupBox1.Controls.Add(this.btnTitle);
            this.groupBox1.Location = new System.Drawing.Point(176, 8);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Size = new System.Drawing.Size(160, 104);
            this.groupBox1.TabIndex = 10;
            this.groupBox1.TabStop = false;
            this.groupBox1.Text = "DVD Menus";
            // 
            // groupBox2
            // 
            this.groupBox2.Controls.Add(this.chkVals);
            this.groupBox2.Controls.Add(this.chkReadOnly);
            this.groupBox2.Location = new System.Drawing.Point(16, 8);
            this.groupBox2.Name = "groupBox2";
            this.groupBox2.Size = new System.Drawing.Size(144, 104);
            this.groupBox2.TabIndex = 11;
            this.groupBox2.TabStop = false;
            this.groupBox2.Text = "Log options";
            // 
            // chkLegacyMC
            // 
            this.chkLegacyMC.AutoSize = true;
            this.chkLegacyMC.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.chkLegacyMC.Location = new System.Drawing.Point(10, 41);
            this.chkLegacyMC.Name = "chkLegacyMC";
            this.chkLegacyMC.Size = new System.Drawing.Size(150, 17);
            this.chkLegacyMC.TabIndex = 14;
            this.chkLegacyMC.Text = "Use IWMPLibraryServices";
            this.chkLegacyMC.UseVisualStyleBackColor = true;
            this.chkLegacyMC.CheckedChanged += new System.EventHandler(this.chkLegacyMC_CheckedChanged);
            // 
            // groupBox3
            // 
            this.groupBox3.Controls.Add(this.chkLegacyMC);
            this.groupBox3.Location = new System.Drawing.Point(16, 118);
            this.groupBox3.Name = "groupBox3";
            this.groupBox3.Size = new System.Drawing.Size(178, 90);
            this.groupBox3.TabIndex = 15;
            this.groupBox3.TabStop = false;
            this.groupBox3.Text = "Library";
            // 
            // groupBox4
            // 
            this.groupBox4.Controls.Add(this.cmbDrives);
            this.groupBox4.Location = new System.Drawing.Point(200, 118);
            this.groupBox4.Name = "groupBox4";
            this.groupBox4.Size = new System.Drawing.Size(133, 90);
            this.groupBox4.TabIndex = 16;
            this.groupBox4.TabStop = false;
            this.groupBox4.Text = "CD/DVD Drive";
            // 
            // Player
            // 
            this.Player.Enabled = true;
            this.Player.Location = new System.Drawing.Point(352, 8);
            this.Player.Name = "Player";
            this.Player.OcxState = ((System.Windows.Forms.AxHost.State)(resources.GetObject("Player.OcxState")));
            this.Player.Size = new System.Drawing.Size(288, 200);
            this.Player.TabIndex = 0;
            this.Player.OpenPlaylistSwitch += new AxWMPLib._WMPOCXEvents_OpenPlaylistSwitchEventHandler(this.Player_OpenPlaylistSwitch);
            this.Player.OpenStateChange += new AxWMPLib._WMPOCXEvents_OpenStateChangeEventHandler(this.Player_OpenStateChange);
            // 
            // Form1
            // 
            this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
            this.ClientSize = new System.Drawing.Size(664, 528);
            this.Controls.Add(this.groupBox4);
            this.Controls.Add(this.groupBox3);
            this.Controls.Add(this.groupBox2);
            this.Controls.Add(this.groupBox1);
            this.Controls.Add(this.lblStatus);
            this.Controls.Add(this.listView1);
            this.Controls.Add(this.Player);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedSingle;
            this.Menu = this.mainMenu1;
            this.Name = "Form1";
            this.Text = "Schema and Attribute Viewer";
            this.groupBox1.ResumeLayout(false);
            this.groupBox2.ResumeLayout(false);
            this.groupBox3.ResumeLayout(false);
            this.groupBox3.PerformLayout();
            this.groupBox4.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.Player)).EndInit();
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
        
        
        /// <summary>
        /// Open a digital media file for playback.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void mnuOpenMedia_Click(object sender, System.EventArgs e)
        {
            OpenFileDialog dlg = new OpenFileDialog();
            dlg.RestoreDirectory = true;
            dlg.InitialDirectory = Environment.GetFolderPath(Environment.SpecialFolder.MyMusic);

            try
            {
                dlg.Filter = "Windows Media Audio(.wma)|*.wma|" + "Windows Media Video(.wmv)|*.wmv|" + "Video Track(.IFO)|*.IFO|" + "Windows Media Playlist(.wpl)|*.wpl|" + "Windows Media Metafile(.asx)|*.asx|" + "MP3 File(.mp3)|*.mp3";
                if (dlg.ShowDialog() == DialogResult.OK)
                {
                    Player.URL = dlg.FileName;
                }
            }
            catch (COMException ex)
            {
                lblStatus.Text = "Failed to set player URL: " + ex.Message.ToString();
            }
            catch
            {
                lblStatus.Text = "Failed to set player URL.";
                throw;
            }
        }
        
        /// <summary>
        /// Capture attributes for each type.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void mnuLibraryAttributes_Click(object sender, System.EventArgs e)
        {
            UseWaitCursor = true;

            listView1.Items.Clear();

            lblStatus.Text  = "Library attributes by media type";

            getMetadataFromMedia(AttributeSource.Audio);

            // The mediaCollection will return non-audio attributes 
            // when using the legacy object model only.
            if (chkLegacyMC.Checked == false)
            {
                getMetadataFromMedia(AttributeSource.Video);
                getMetadataFromMedia(AttributeSource.Playlist);
                getMetadataFromMedia(AttributeSource.Other);
                getMetadataFromMedia(AttributeSource.Photo);
            }
            else
            {
                lblStatus.Text = "MediaCollection object only supports Audio attributes.";
            }

            rtLast = ReportType.AllSchemas;

            UseWaitCursor = false;
        }

        /// <summary>
        /// Capture CD TOC attributes.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void mnuCDTOC_Click(object sender, System.EventArgs e)
        {
            UseWaitCursor = true;

            listView1.Items.Clear();

            lblStatus.Text = "CD TOC Attributes";

            getMetadataFromPlaylist(AttributeSource.CDPlaylist);

            rtLast = ReportType.CDTOC;

            UseWaitCursor = false;
        }

        /// <summary>
        /// Capture DVD attributes.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void mnuDVDTitleChapter_Click(object sender, System.EventArgs e)
        {
            UseWaitCursor = true;

            listView1.Items.Clear();

            // To see the implementation for enumerating DVD title and chapter attributes,
            // view the OpenPlaylistSwitch event handler.
            lblStatus.Text = "Wait for OpenPlaylistSwitch event...";

            if(Player.dvd.get_isAvailable("dvdDecoder") == true)
            {
                // This will trigger the OpenPlaylistSwitch event.
                // The CDCollection object is used for both CD and DVD drives.
                // The TOC of a DVD is contained in an IWMPPlaylist.
                Player.currentPlaylist = CDCollection.Item(cmbDrives.SelectedIndex).Playlist;
            }
            else
            {
                lblStatus.Text = "No DVD decoder installed.";
            }

            rtLast = ReportType.DVDTitleChap;

            UseWaitCursor = false;
        }

        /// <summary>
        /// Used to log the DVD attributes.
        /// DVD organization can vary.
        /// Usually, when you open the DVD for playback, you'll see data for titles,
        /// when you play a title, you'll see chapters.
        /// Title 0 is special - it's the table of contents.
        /// Usually, you can click the Top Menu button to go back to the root title.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void Player_OpenPlaylistSwitch(object sender, AxWMPLib._WMPOCXEvents_OpenPlaylistSwitchEvent e)
        {
            UseWaitCursor = true;
            listView1.Items.Clear();

            // Set the button states for the Top Menu and Title Menu buttons.
            SetDVDButtonStates();

            // Get the title playlist.
            IWMPPlaylist pTitle = (IWMPPlaylist)e.pItem;
            IWMPMedia TitleOrChapter = null;

            // Get the chapter as IWMPMedia.
            TitleOrChapter = pTitle.get_Item(1);

            //Get the attribute count from the chapter and loop through the attributes.
            int iAttCount = TitleOrChapter.attributeCount;
            for (int j = 0; j < iAttCount; j++)
            {
                ListViewItem item = new ListViewItem("DVD Title/Ch");
                String name = TitleOrChapter.getAttributeName(j);
                item.SubItems.Add(name);
                item.SubItems.Add(TitleOrChapter.getItemInfo(name));

                // Tell the user about DVD navigation options.
                if ("chapterNum" == name)
                {
                    lblStatus.Text = "DVD Chapter Attributes. To view title attributes, change titles.";
                }
                else if ("titleNum" == name)
                {
                    lblStatus.Text = "DVD Title Attributes. To view chapter attributes, change chapters.";
                }

                bool bRO = TitleOrChapter.isReadOnlyItem(name);

                item.SubItems.Add(bRO.ToString());
                listView1.Items.Add(item);
            }

            ListViewItem item2 = new ListViewItem("");
            listView1.Items.Add(item2);

            UseWaitCursor = false;
        }

        /// <summary>
        /// Capture CD track attributes.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void mnuCDTrack_Click(object sender, System.EventArgs e)
        {
            UseWaitCursor = true;
            listView1.Items.Clear();

            lblStatus.Text = "CD Track Attributes";

            getMetadataFromMedia(AttributeSource.CDTrack);

            rtLast = ReportType.CDTrack;
            UseWaitCursor = false;
        }

        /// <summary>
        /// Capture PlaylistCollection attributes.
        /// Unlike the playlist attributes that can be retrieved by calling
        /// MediaCollection.getByAttribute("Media Type", "playlist"), the
        /// playlist collection attributes should be retrieved by calling
        /// PlaylistCollection.getAll.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void mnuPlaylistCollection_Click(object sender, System.EventArgs e)
        {
            UseWaitCursor = true;
            listView1.Items.Clear();

            lblStatus.Text = "PlaylistCollection Attributes";

            getMetadataFromPlaylist(AttributeSource.PlaylistCollection);

            rtLast = ReportType.PlaylistCollection;
            UseWaitCursor = false;
        }

        /// <summary>
        /// Set a value for the MediaCollection variable based on user settings.
        /// </summary>
        private void GetMediaCollection()
        {
            if (chkLegacyMC.CheckState != CheckState.Checked)
            {
                MediaCollection = Player.mediaCollection;
            }
            else
            {               
                IWMPLibrary library = LibSvcs.getLibraryByType(WMPLibraryType.wmpltLocal, 0);
                MediaCollection = library.mediaCollection;
            }

            runCurrentReport();
        }
    


        /// <summary>
        /// Writes a string to a text file.
        /// </summary>
        /// <param name="str">String to write to text file.</param>
        /// <param name="mode">File open mode.</param>
        private void writeToFile(String str, FileMode mode)
        {
            // Create a StreamWriter object to write
            // the output to a file.
            FileStream sb = new FileStream(strOutFile, mode);
            StreamWriter sw = new StreamWriter(sb); 

            sw.Write(str);
            sw.Close();
        }

        /// <summary>
        /// Builds the output string from the ListView control.
        /// </summary>
        /// <param name="showValues">Log values?</param>
        /// <param name="showRO">Log writability?</param>
        /// <returns></returns>
        private string logStringFromTable(bool showValues, bool showRO)
        { 
            ListView.ListViewItemCollection lvic = listView1.Items;
            int cItems = lvic.Count;
            StringBuilder output = new StringBuilder();

            for(int row = 0; row < cItems; row++)
            {
                ListViewItem item = lvic[row];
                ListViewItem.ListViewSubItemCollection lvsic = item.SubItems;
                int cSubItems = lvsic.Count;

                // The ListView columns in each row are subitems.
                for(int column = 0; column < cSubItems; column++)
                {
                    ListViewItem.ListViewSubItem subItem = lvsic[column];

                    // Obey the checkboxes.
                    // Don't log empty rows.
                    if((2 == column && false == showValues) ||
                       (3 == column && false == showRO)     ||
                       (0 == column && String.IsNullOrEmpty(subItem.Text)))
                    {
                        continue;
                    }

                    switch (column)
                    {
                        case 0:
                            output.Append("Schema: ");
                            break;
                        case 1:
                            output.Append("Name: ");
                            break;
                        case 2:
                            output.Append("Value: ");
                            break;
                        case 3:
                            output.Append("Read-only: ");
                            break;
                    }

                    output.Append(subItem.Text);
                    output.Append("\r\n");
                }

                output.Append("\r\n");
            }

            return output.ToString();

        }

        /// <summary>
        /// Open DVD top menu, if available.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void btnTop_Click(object sender, System.EventArgs e)
        {
            Player.dvd.topMenu();
        }

        /// <summary>
        /// Open DVD title menu, if available.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void btnTitle_Click(object sender, System.EventArgs e)
        {
            Player.dvd.titleMenu();
        }

        /// <summary>
        /// Displays the attribute information for a given media-based attribute source.
        /// </summary>
        /// <param name="Source">An AttributeSource specifying the schema to inspect.</param>
        /// <returns>void</returns>
        private void getMetadataFromMedia(AttributeSource Source)
        {
            IWMPPlaylist playlist = null;
            IWMPMedia media = null;

            string name = "";

            try
            {
                switch (Source)
                {
                    case AttributeSource.CDTrack:
                        playlist = CD.Playlist;
                        break;
                    case AttributeSource.CurrentMedia:
                        playlist = Player.currentPlaylist;
                        break;
                    default:
                        // Get a playlist filled with media for the specified schema.
                        playlist = MediaCollection.getByAttribute("MediaType", Source.ToString());
                        break;
                }

                if (0 != playlist.count)
                {
                    // Get the first item from the playlist.
                    media = playlist.get_Item(0);
                }
                else
                {
                    throw new EmptyPlaylistException();
                }

                int cAttributes = media.attributeCount;

                // Log the attribute name, value, and writability.
                for (int i = 0; i < cAttributes; i++)
                {
                    name = media.getAttributeName(i);

                    ListViewItem item = new ListViewItem(Source.ToString());

                    item.SubItems.Add(name);
                    item.SubItems.Add(media.getItemInfo(name));
                    bool bRO = media.isReadOnlyItem(name);
                    item.SubItems.Add(bRO.ToString());
                    listView1.Items.Add(item);
                }
            }
            catch (EmptyPlaylistException)
            {
                ListViewItem item3 = new ListViewItem(Source.ToString());
                item3.SubItems.Add("EmptyPlaylistException");
                item3.SubItems.Add("Does your library contain media for this type or source?");
                listView1.Items.Add(item3);
            }
            catch (COMException exc)
            {
                lblStatus.Text = "Exception in getMetadata: " + exc.Message;
            }
            catch
            {
                lblStatus.Text = "Exception in getMetadata.";
                throw;
            }
            
            // Insert an empty line in the listview.
            ListViewItem item2 = new ListViewItem("");
            listView1.Items.Add(item2);
                
        }

        /// <summary>
        /// Retrieves the attribute names for a given playlist-based attribute source.
        /// </summary>
        /// <param name="Source">An AttributeSource specifying the type of media to inspect</param>
        /// <returns>void</returns>
        private void getMetadataFromPlaylist(AttributeSource Source)
        {
            IWMPPlaylist playlist = null;
            string name = "";

            try
            {
                switch (Source)
                {
                    // DVDs and CDs use the same CD object.
                    case AttributeSource.DVDToc:
                    case AttributeSource.CDPlaylist:
                        playlist = CD.Playlist;
                        break;
                    case AttributeSource.PlaylistCollection:
                        // Retrieve a playlist from the PlaylistCollection.
                        playlist = PLCollection.getAll().Item(0);
                        break;
                    case AttributeSource.CurrentPlaylist:
                        playlist = Player.currentPlaylist;
                        break;
                    default:
                        // This is strictly for debugging, it should never happen.
                        // If it does, we need a new case here or to fix the calling function.
                        throw new ArgumentOutOfRangeException("Source");
                }

                int cAttributes = playlist.attributeCount;

                // Log the attribute name and writability.
                for (int i = 0; i < cAttributes; i++)
                {
                    name = playlist.get_attributeName(i);
                    ListViewItem item = new ListViewItem(Source.ToString());

                    item.SubItems.Add(name);

                    item.SubItems.Add(playlist.getItemInfo(name));

                    // We'll assume it's read-only.
                    // If we can actually write it, we'll flip the bool.
                    // We do this because there is no IsReadOnlyItem property
                    // available on the playlist object.
                    bool bRO = true;

                    try
                    {
                        // Cache the value.
                        string temp = playlist.getItemInfo(name);
                        // Try to write something.
                        playlist.setItemInfo(name, "random");
                        // Write back the cached value.
                        playlist.setItemInfo(name, temp);
                        bRO = false;
                    }
                    catch (COMException)
                    {
                        // Writing the test value failed.
                        // bRO is true by default, so nothing to do here.
                    }

                    item.SubItems.Add(bRO.ToString());

                    listView1.Items.Add(item);
                }
            }
            catch (NullReferenceException)
            {
                ListViewItem item2 = new ListViewItem(Source.ToString());
                item2.SubItems.Add("NullReferenceException");
                item2.SubItems.Add("Does your library contain a playlist?");
                listView1.Items.Add(item2);
            }
            catch (ArgumentOutOfRangeException exc)
            {
                lblStatus.Text = "Invalid parameter in getMetadataFromPlaylist: " + exc.Message;
            }
            catch (COMException exc)
            {
                lblStatus.Text = "Exception in getMetadata: " + exc.Message;
            }
            catch
            {
                lblStatus.Text = "Exception in getMetadata.";
                throw;
            }
        }

        /// <summary>
        /// Run the current report. Refreshes the log display when needed,
        /// such as when the user switches libraries.
        /// </summary>
        private void runCurrentReport()
        {
            switch (rtLast)
            {
                case ReportType.AllSchemas:
                    mnuLibraryAttributes_Click(this, null);
                    break;
                case ReportType.CDTOC:
                    mnuCDTOC_Click(this, null);
                    break;
                case ReportType.CDTrack:
                    mnuCDTrack_Click(this, null);
                    break;
                case ReportType.DVDTitleChap:
                    mnuDVDTitleChapter_Click(this, null);
                    break;
                case ReportType.DVDTOC:
                    mnuDVDTOC_Click(this, null);
                    break;
                case ReportType.PlaylistCollection:
                    mnuPlaylistCollection_Click(this, null);
                    break;
                case ReportType.CurrentMedia:
                    mnuCurrentMedia_Click(this, null);
                    break;
                default:
                    break;
            }

        }

        /// <summary>
        /// Fills the combo box with CD drive letters.
        /// </summary>
        private void enumerateCDDrives()
        {
            UseWaitCursor = true;
            cmbDrives.Items.Clear();

            int count = CDCollection.count;

            for(int i = 0; i < count; i++)
            {
                cmbDrives.Items.Add(CDCollection.Item(i).driveSpecifier);
            }

            cmbDrives.SelectedIndex = 0;
            UseWaitCursor = false;
        }

        /// <summary>
        /// Get the current CD object.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void cmbDrives_SelectedIndexChanged(object sender, System.EventArgs e)
        {
            CD = Player.cdromCollection.Item(cmbDrives.SelectedIndex);
        }

        /// <summary>
        /// Save output to text file.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void mnuSaveAs_Click(object sender, System.EventArgs e)
        {
            SaveFileDialog dlg = new SaveFileDialog();

            try
            {
                dlg.Filter = "Text files(.txt)|*.txt";
                dlg.RestoreDirectory = true;
                dlg.InitialDirectory = Environment.GetFolderPath(Environment.SpecialFolder.MyDocuments);
                if (dlg.ShowDialog() == DialogResult.OK)
                {
                    strOutFile = dlg.FileName;
                    string retval = logStringFromTable(bShowVals, bShowReadOnly);
                    writeToFile(retval, FileMode.Create);
                }
            }
            catch (FileNotFoundException ex)
            {
                lblStatus.Text = "File not found exception: " + ex.Message.ToString();
            }
            catch
            {
                lblStatus.Text = "Error when opening file for saving.";
                throw;
            }  
        }

        /// <summary>
        /// Cache the values checkbox state.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void chkVals_CheckedChanged(object sender, System.EventArgs e)
        {
            bShowVals = chkReadOnly.Checked;
        }

        /// <summary>
        /// Cache the writability checkbox state.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void chkReadOnly_CheckedChanged(object sender, System.EventArgs e)
        {
            bShowReadOnly = chkReadOnly.Checked;
        }

        /// <summary>
        /// Get the DVD Title 0 attributes.
        /// This is essentially the same as a CD TOC capture.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void mnuDVDTOC_Click(object sender, System.EventArgs e)
        {
            UseWaitCursor = true;
            listView1.Items.Clear();

            lblStatus.Text = "DVD TOC Attributes";

            if(Player.dvd.get_isAvailable("dvdDecoder") == true)
            {
                getMetadataFromPlaylist(AttributeSource.DVDToc);
            }
            else
            {
                lblStatus.Text = "No DVD decoder installed.";
            };
 
            rtLast = ReportType.DVDTOC;
            UseWaitCursor = false;
        }

        /// <summary>
        /// Save using current filename.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void mnuSave_Click(object sender, System.EventArgs e)
        {
            UseWaitCursor = true;

            // Save should behave like Save as if the output file path
            // is undefined.
            if(String.IsNullOrEmpty(strOutFile))
            {
                mnuSaveAs_Click(this, null);
                return;
            }

            string retval = logStringFromTable(bShowVals, bShowReadOnly);
            writeToFile(retval, FileMode.Create);
            UseWaitCursor = false;
        }

        /// <summary>
        /// Capture attributes for current media item.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void mnuCurrentMedia_Click(object sender, EventArgs e)
        {
            UseWaitCursor = true;
            listView1.Items.Clear();

            lblStatus.Text = "Current Media Attributes";

            getMetadataFromMedia(AttributeSource.CurrentMedia);

            rtLast = ReportType.CurrentMedia;
            UseWaitCursor = false;
        }

        /// <summary>
        /// Capture attributes for the current playlist.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void mnuCurrentPlaylist_Click(object sender, EventArgs e)
        {
            UseWaitCursor = true;
            listView1.Items.Clear();

            lblStatus.Text = "Current Playlist Attributes";

            getMetadataFromPlaylist(AttributeSource.CurrentPlaylist);

            rtLast = ReportType.CurrentPlaylist;
            UseWaitCursor = false;
        }


        /// <summary>
        /// Respond to user checkbox selection.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void chkLegacyMC_CheckedChanged(object sender, EventArgs e)
        {
            GetMediaCollection();
        }

        /// <summary>
        /// Player OpenStateChange event handler.
        /// Show current media metadata when the user opens a media file.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e">e.NewState contains the WMPOpenState value for the event.</param>
        private void Player_OpenStateChange(object sender, AxWMPLib._WMPOCXEvents_OpenStateChangeEvent e)
        {
            if ((WMPOpenState)e.newState == WMPOpenState.wmposMediaOpen)
            {
                mnuCurrentMedia_Click(this, null);
                SetDVDButtonStates(); 
            }
        }

        /// <summary>
        /// Manage the DVD menu button states.
        /// </summary>
        private void SetDVDButtonStates()
        {
            btnTop.Enabled = Player.dvd.get_isAvailable("topMenu");
            btnTitle.Enabled = Player.dvd.get_isAvailable("titleMenu");
        }
    }

    /// <summary>
    /// Custom exception for dealing with an empty playlist.
    /// We'll raise this exception if we expect a playlist to contain media items,
    /// but it does not.
    /// </summary>

    [Serializable]
    public class EmptyPlaylistException: System.Exception, ISerializable 
    {
        public EmptyPlaylistException()
        {
        }
        public EmptyPlaylistException(String message) : base(message)
        {
        }
        public EmptyPlaylistException(String message, Exception innerException):
            base(message, innerException) 
        {
        }
        protected EmptyPlaylistException(SerializationInfo info, 
         StreamingContext context) : base(info, context)
        {
        }

    }

}

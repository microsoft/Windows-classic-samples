//-------------------------------------------------------------------------
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//
//  File: InkCollection.cs
//  Simple Ink Collection Sample Application
//
//  This sample program demonstrates how to use a real time stylus 
//  to collect and render pen input. 
//
//  The features used are: RealTimeStylus, DynamicRenderer, 
//  and IStylusAsyncPlugin.
//
//--------------------------------------------------------------------------

using System;
using System.Diagnostics;
using System.Drawing;
using System.Windows.Forms;
using System.Collections;

// The Microsoft Tablet PC namespaces
using Microsoft.Ink;
using Microsoft.StylusInput;
using Microsoft.StylusInput.PluginData;


namespace InkCollection
{

    /// <summary>
    /// The InkCollection Sample Application form class
    /// </summary>
    public class InkCollection : Form, IStylusAsyncPlugin
    {

        // Declare constants for the pen widths used by this application.
        // Note that these constants are in himetric units (1 unit = .01mm)
        private const float ThinInkWidth = 10;
        private const float MediumInkWidth = 100;
        private const float ThickInkWidth = 200;

        // Declare the real time stylus
        private RealTimeStylus myRealTimeStylus;

        // Declare the renderers.  The dynamic renderer is used to render the ink
        // stroke that is currently being collected, whereas the static renderer 
        // is used to render strokes that have already been collected.
        private DynamicRenderer myDynamicRenderer;
        private Renderer myRenderer;

        // Declare the hashtable used to store packet data that is being collected by one
        // or more cursors.  StylusId's are used as the hashtable key to uniquely
        // identify the packet data collected for a given cursor.
        private Hashtable myPackets;

        // Declare the ink object used to store strokes collected by the real time stylus.
        private Ink myInk;

        #region Standard Template Code

        private System.Windows.Forms.MenuItem miMainFile;
        private System.Windows.Forms.MenuItem miExit;
        private System.Windows.Forms.MenuItem miMainInk;
        private System.Windows.Forms.MenuItem miEnabled;
        private System.Windows.Forms.MenuItem miColor;
        private System.Windows.Forms.MenuItem miRed;
        private System.Windows.Forms.MenuItem miGreen;
        private System.Windows.Forms.MenuItem miBlue;
        private System.Windows.Forms.MenuItem miBlack;
        private System.Windows.Forms.MenuItem miWidth;
        private System.Windows.Forms.MenuItem miThin;
        private System.Windows.Forms.MenuItem miMedium;
        private System.Windows.Forms.MenuItem miThick;
        private System.Windows.Forms.MainMenu miMain;
        private System.ComponentModel.Container components = null;
        #endregion

        public InkCollection()
        {
            #region Standard Template Code
            // Required for Windows Form Designer support
            InitializeComponent();
            #endregion
        }

        #region Standard Template Code
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
        #endregion

        #region Windows Form Designer generated code
        /// <summary>
        /// ----- Standard Template Code -----
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.miMain = new System.Windows.Forms.MainMenu();
            this.miMainFile = new System.Windows.Forms.MenuItem();
            this.miExit = new System.Windows.Forms.MenuItem();
            this.miMainInk = new System.Windows.Forms.MenuItem();
            this.miEnabled = new System.Windows.Forms.MenuItem();
            this.miColor = new System.Windows.Forms.MenuItem();
            this.miRed = new System.Windows.Forms.MenuItem();
            this.miGreen = new System.Windows.Forms.MenuItem();
            this.miBlue = new System.Windows.Forms.MenuItem();
            this.miBlack = new System.Windows.Forms.MenuItem();
            this.miWidth = new System.Windows.Forms.MenuItem();
            this.miThin = new System.Windows.Forms.MenuItem();
            this.miMedium = new System.Windows.Forms.MenuItem();
            this.miThick = new System.Windows.Forms.MenuItem();
            // 
            // miMain
            // 
            this.miMain.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
                                                                                   this.miMainFile,
                                                                                   this.miMainInk});
            // 
            // miMainFile
            // 
            this.miMainFile.Index = 0;
            this.miMainFile.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
                                                                                       this.miExit});
            this.miMainFile.Text = "File";
            // 
            // miExit
            // 
            this.miExit.Index = 0;
            this.miExit.Text = "Exit";
            this.miExit.Click += new System.EventHandler(this.miExit_Click);
            // 
            // miMainInk
            // 
            this.miMainInk.Index = 1;
            this.miMainInk.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
                                                                                      this.miEnabled,
                                                                                      this.miColor,
                                                                                      this.miWidth});
            this.miMainInk.Text = "Ink";
            // 
            // miEnabled
            // 
            this.miEnabled.Checked = true;
            this.miEnabled.Index = 0;
            this.miEnabled.Text = "Enabled";
            this.miEnabled.Click += new System.EventHandler(this.miEnabled_Click);
            // 
            // miColor
            // 
            this.miColor.Index = 1;
            this.miColor.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
                                                                                    this.miRed,
                                                                                    this.miGreen,
                                                                                    this.miBlue,
                                                                                    this.miBlack});
            this.miColor.Text = "Color";
            // 
            // miRed
            // 
            this.miRed.Index = 0;
            this.miRed.Text = "Red";
            this.miRed.Click += new System.EventHandler(this.miRed_Click);
            // 
            // miGreen
            // 
            this.miGreen.Index = 1;
            this.miGreen.Text = "Green";
            this.miGreen.Click += new System.EventHandler(this.miGreen_Click);
            // 
            // miBlue
            // 
            this.miBlue.Index = 2;
            this.miBlue.Text = "Blue";
            this.miBlue.Click += new System.EventHandler(this.miBlue_Click);
            // 
            // miBlack
            // 
            this.miBlack.Index = 3;
            this.miBlack.Text = "Black";
            this.miBlack.Click += new System.EventHandler(this.miBlack_Click);
            // 
            // miWidth
            // 
            this.miWidth.Index = 2;
            this.miWidth.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
                                                                                    this.miThin,
                                                                                    this.miMedium,
                                                                                    this.miThick});
            this.miWidth.Text = "Width";
            // 
            // miThin
            // 
            this.miThin.Index = 0;
            this.miThin.Text = "Thin";
            this.miThin.Click += new System.EventHandler(this.miThin_Click);
            // 
            // miMedium
            // 
            this.miMedium.Index = 1;
            this.miMedium.Text = "Medium";
            this.miMedium.Click += new System.EventHandler(this.miMedium_Click);
            // 
            // miThick
            // 
            this.miThick.Index = 2;
            this.miThick.Text = "Thick";
            this.miThick.Click += new System.EventHandler(this.miThick_Click);
            // 
            // InkCollection
            // 
            this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
            this.ClientSize = new System.Drawing.Size(292, 245);
            this.Menu = this.miMain;
            this.Name = "InkCollection";
            this.Text = "RTS Ink Collection Sample";
            this.Load += new System.EventHandler(this.InkCollection_Load);
            this.Paint += new System.Windows.Forms.PaintEventHandler(this.InkCollection_Paint);


        }
        #endregion

        #region Standard Template Code
        /// <summary>
        /// The main entry point for the application.
        /// </summary>
        [STAThread]
        static void Main() 
        {
            Application.Run(new InkCollection());
        }
        #endregion      

        /// <summary>
        /// Event Handler from Form Load Event
        /// Set up the real time stylus for collection
        /// </summary>
        /// <param name="sender">The control that raised the event.</param>
        /// <param name="e">The event arguments.</param>
        private void InkCollection_Load(object sender, System.EventArgs e)
        {

            // Create the renderers.  The dynamic renderer is used to render the ink
            // stroke that is currently being collected, whereas the static renderer 
            // is used to render strokes that have already been collected.
            myDynamicRenderer = new DynamicRenderer(this);
            myRenderer = new Renderer();

            // 
            // If you do not modify the default drawing attributes, the default 
            // drawing attributes will use the following properties and values:
            // 
            //      AntiAliased     = true
            //      Color           = black
            //      FitToCurve      = false
            //      Height          = 1
            //      IgnorePressure  = false
            //      PenTip          = ball
            //      RasterOperation = copy pen
            //      Transparency    = 0
            //      Width           = 53 (2 pixels on a 96 dpi screen)
            // 
            // For an example of how to modify other drawing attributes, uncomment
            // the following lines of code:
            // myDynamicRenderer.DrawingAttributes.PenTip = PenTip.Rectangle;
            // myDynamicRenderer.DrawingAttributes.Height = (.5F)*MediumInkWidth;
            // myDynamicRenderer.DrawingAttributes.Transparency = 128;
            //

            // Create the real time stylus used to receive stylus notifications
            myRealTimeStylus = new RealTimeStylus(this, true);

            // Add the dynamic renderer to the synchronous plugin notification chain.
            // Synchronous notifications occur on the pen thread.
            myRealTimeStylus.SyncPluginCollection.Add(myDynamicRenderer);

            // Add the form to the asynchronous plugin notification chain.  This plugin
            // will be used to collect stylus data into an ink object.  Asynchronous
            // notifications occur on the UI thread.
            myRealTimeStylus.AsyncPluginCollection.Add(this);

            // Enable the real time stylus and the dynamic renderer
            myRealTimeStylus.Enabled = true;
            myDynamicRenderer.Enabled = true;  
      
            // Create the ink object used to store ink collected from the real time stylus
            myPackets = new Hashtable();
            myInk = new Ink();
        }

        /// <summary>
        /// Event Handler from the form Paint event.
        /// </summary>
        /// <param name="sender">The control that raised the event.</param>
        /// <param name="e">The event arguments.</param>
        private void InkCollection_Paint(object sender, System.Windows.Forms.PaintEventArgs e)
        {
            // Refresh the dynamic renderer, since it's possible that a stroke is being
            // collected at the time Paint occurs.  In this case, the portion of the stroke
            // that has already been collected will need to be redrawn.
            myDynamicRenderer.Refresh();

            // Use the static renderer to redraw strokes that have already been collected.
            myRenderer.Draw(e.Graphics, myInk.Strokes);
        }

        #region Menu Event Handlers
        /// <summary>
        /// Event Handler from Ink->Color->Red Menu Item
        /// </summary>
        /// <param name="sender">The control that raised the event.</param>
        /// <param name="e">The event arguments.</param>
        private void miRed_Click(object sender, System.EventArgs e)
        {

            myDynamicRenderer.DrawingAttributes.Color = Color.Red;
        }

        /// <summary>
        /// Event Handler from Ink->Color->Green Menu Item
        /// </summary>
        /// <param name="sender">The control that raised the event.</param>
        /// <param name="e">The event arguments.</param>
        private void miGreen_Click(object sender, System.EventArgs e)
        {
            myDynamicRenderer.DrawingAttributes.Color = Color.Green;
        }

        /// <summary>
        /// Event Handler from Ink->Color->Blue Menu Item
        /// </summary>
        /// <param name="sender">The control that raised the event.</param>
        /// <param name="e">The event arguments.</param>
        private void miBlue_Click(object sender, System.EventArgs e)
        {
            myDynamicRenderer.DrawingAttributes.Color = Color.Blue;
        }

        /// <summary>
        /// Event Handler from Ink->Color->Black Menu Item
        /// </summary>
        /// <param name="sender">The control that raised the event.</param>
        /// <param name="e">The event arguments.</param>
        private void miBlack_Click(object sender, System.EventArgs e)
        {
            myDynamicRenderer.DrawingAttributes.Color = Color.Black;
        }

        /// <summary>
        /// Event Handler from Ink->Width->Thin Menu Item
        /// </summary>
        /// <param name="sender">The control that raised the event.</param>
        /// <param name="e">The event arguments.</param>
        private void miThin_Click(object sender, System.EventArgs e)
        {
            myDynamicRenderer.DrawingAttributes.Width = ThinInkWidth;
        }

        /// <summary>
        /// Event Handler from Ink->Width->Medium Menu Item
        /// </summary>
        /// <param name="sender">The control that raised the event.</param>
        /// <param name="e">The event arguments.</param>
        private void miMedium_Click(object sender, System.EventArgs e)
        {
            myDynamicRenderer.DrawingAttributes.Width = MediumInkWidth;
        }

        /// <summary>
        /// Event Handler from Ink->Width->Thick Menu Item
        /// </summary>
        /// <param name="sender">The control that raised the event.</param>
        /// <param name="e">The event arguments.</param>
        private void miThick_Click(object sender, System.EventArgs e)
        {
            myDynamicRenderer.DrawingAttributes.Width = ThickInkWidth;
        }

        /// <summary>
        /// Event Handler from File->Exit Menu Item
        /// </summary>
        /// <param name="sender">The control that raised the event.</param>
        /// <param name="e">The event arguments.</param>
        private void miExit_Click(object sender, System.EventArgs e)
        {
            myRealTimeStylus.Enabled = false;
            Application.Exit();
        }

        /// <summary>
        /// Event Handler from Ink->Enabled Menu Item
        /// </summary>
        /// <param name="sender">The control that raised the event.</param>
        /// <param name="e">The event arguments.</param>
        private void miEnabled_Click(object sender, System.EventArgs e)
        {
            // Toggle the enabled state of the real time stylus
            myRealTimeStylus.Enabled = !myRealTimeStylus.Enabled;
            miEnabled.Checked = myRealTimeStylus.Enabled;
        }
        #endregion

        #region IStylusAsyncPlugin interface implementation

        /// <summary>
        /// Occurs when the stylus touches the digitizer surface.
        /// Allocate a new array to store the packet data for this stylus.
        /// </summary>
        /// <param name="sender">The real time stylus associated with the notification</param>
        /// <param name="data">The notification data</param>
        public void StylusDown(RealTimeStylus sender, StylusDownData data)
        {
            // Allocate an empty array to store the packet data that will be
            // collected for this stylus.  
            ArrayList collectedPackets = new ArrayList();

            // Add the packet data from StylusDown to the array
            collectedPackets.AddRange(data.GetData());

            // Insert the array into a hashtable using the stylus id as a key.
            myPackets.Add(data.Stylus.Id, collectedPackets);
        }

        /// <summary>
        /// Occurs when the stylus moves on the digitizer surface.
        /// Add new packet data into the packet array for this stylus.
        /// </summary>
        /// <param name="sender">The real time stylus associated with the notification</param>
        /// <param name="data">The notification data</param>
        public void Packets(RealTimeStylus sender, PacketsData data)
        {
            // Use the stylus id as a key to retrieve the packet array for the
            // stylus.  Insert the new packet data into this array.
            ((ArrayList)(myPackets[data.Stylus.Id])).AddRange(data.GetData());
        }

        /// <summary>
        /// Occurs when the stylus leaves the digitizer surface.
        /// Retrieve the packet array for this stylus and use it to create
        /// a new stoke.
        /// </summary>
        /// <param name="sender">The real time stylus associated with the notification</param>
        /// <param name="data">The notification data</param>
        public void StylusUp(RealTimeStylus sender, StylusUpData data)
        {
            // Retrieve the packet array from the hashtable using the cursor id
            // as a key.  Then, clean this entry from the hash since it is no
            // longer needed.
            ArrayList collectedPackets = (ArrayList)myPackets[data.Stylus.Id];
            myPackets.Remove(data.Stylus.Id);

            // Add the packet data from StylusUp to the array
            collectedPackets.AddRange(data.GetData());

            // Create the stroke using the specified drawing attributes.
            int[] packets = (int[])(collectedPackets.ToArray(typeof(int)));
            TabletPropertyDescriptionCollection tabletProperties = myRealTimeStylus.GetTabletPropertyDescriptionCollection(data.Stylus.TabletContextId);

            Stroke stroke = myInk.CreateStroke(packets, tabletProperties);
            if (stroke != null) 
            {
                stroke.DrawingAttributes.Color = myDynamicRenderer.DrawingAttributes.Color;
                stroke.DrawingAttributes.Width = myDynamicRenderer.DrawingAttributes.Width;
            } 
        }

        /// <summary>
        /// Called when the current plugin or the ones previous in the list
        /// threw an exception.
        /// </summary>
        /// <param name="sender">The real time stylus</param>
        /// <param name="data">Error data</param>
        public void Error(RealTimeStylus sender, ErrorData data)
        {
            Debug.Assert(false, null, "An error occurred.  DataId=" + data.DataId + ", " + "Exception=" + data.InnerException);
        }

        /// <summary>
        /// Defines the types of notifications the plugin is interested in.
        /// </summary>
        public DataInterestMask DataInterest
        {
            get
            {
                return DataInterestMask.StylusDown |
                       DataInterestMask.Packets |
                       DataInterestMask.StylusUp |
                       DataInterestMask.Error;
            }
        }

        // The remaining interface methods are not used in this sample application.
        public void RealTimeStylusDisabled(RealTimeStylus sender, RealTimeStylusDisabledData data) {}
        public void RealTimeStylusEnabled(RealTimeStylus sender, RealTimeStylusEnabledData data){}
        public void StylusOutOfRange(RealTimeStylus sender, StylusOutOfRangeData data) {}
        public void StylusInRange(RealTimeStylus sender, StylusInRangeData data) {}
        public void StylusButtonDown(RealTimeStylus sender, StylusButtonDownData data) {}
        public void StylusButtonUp(RealTimeStylus sender, StylusButtonUpData data) {}
        public void CustomStylusDataAdded(RealTimeStylus sender, CustomStylusData data){}
        public void SystemGesture(RealTimeStylus sender, SystemGestureData data){}
        public void InAirPackets(RealTimeStylus sender, InAirPacketsData data){}
        public void TabletAdded(RealTimeStylus sender, TabletAddedData data){}
        public void TabletRemoved(RealTimeStylus sender, TabletRemovedData data) {}

        #endregion
    }
}

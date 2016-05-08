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
//  This sample program is a "Hello World" application, which
//  demonstrates basic functionality of the Tablet PC platform. 
//  It is the simplest program that you can build using the 
//  Tablet PC Platform APIs. This application uses an ink
//  collector to collect and render pen input. 
//
//  The features used are: InkCollector, default tablet, and 
//  modifying default drawing attributes.
//
//--------------------------------------------------------------------------

using System;
using System.Drawing;
using System.Windows.Forms;

// The Ink namespace, which contains the Tablet PC Platform API
using Microsoft.Ink;


namespace Microsoft.Samples.TabletPC.InkCollection
{
    /// <summary>
    /// The InkCollection Sample Application form class
    /// </summary>
    public class InkCollection : System.Windows.Forms.Form
    {

        // Declare the Ink Collector object
        private InkCollector myInkCollector = null;

        // Declare constants for the pen widths used by this application.
        // Note that these constants are in high metric units (1 unit = .01mm)
        private const float ThinInkWidth = 10;
        private const float MediumInkWidth = 100;
        private const float ThickInkWidth = 200;

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
            this.Text = "Ink Collection Sample";
            this.Load += new System.EventHandler(this.InkCollection_Load);

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
        /// Setup the ink collector for collection
        /// </summary>
        /// <param name="sender">The control that raised the event.</param>
        /// <param name="e">The event arguments.</param>
        private void InkCollection_Load(object sender, System.EventArgs e)
        {

            // Create a new ink collector and assign it to this form's window
            myInkCollector = new InkCollector(this.Handle);

            // Set the pen width to be a medium width
            myInkCollector.DefaultDrawingAttributes.Width = MediumInkWidth;

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
            // myInkCollector.DefaultDrawingAttributes.PenTip = PenTip.Rectangle;
            // myInkCollector.DefaultDrawingAttributes.Height = (.5F)*MediumInkWidth;
            // myInkCollector.DefaultDrawingAttributes.Transparency = 128;
            //

            // Turn the ink collector on
            myInkCollector.Enabled = true;        
        }

        /// <summary>
        /// Event Handler from Ink->Color->Red Menu Item
        /// </summary>
        /// <param name="sender">The control that raised the event.</param>
        /// <param name="e">The event arguments.</param>
        private void miRed_Click(object sender, System.EventArgs e)
        {
            myInkCollector.DefaultDrawingAttributes.Color = Color.Red;
        }

        /// <summary>
        /// Event Handler from Ink->Color->Green Menu Item
        /// </summary>
        /// <param name="sender">The control that raised the event.</param>
        /// <param name="e">The event arguments.</param>
        private void miGreen_Click(object sender, System.EventArgs e)
        {
            myInkCollector.DefaultDrawingAttributes.Color = Color.Green;
        }

        /// <summary>
        /// Event Handler from Ink->Color->Blue Menu Item
        /// </summary>
        /// <param name="sender">The control that raised the event.</param>
        /// <param name="e">The event arguments.</param>
        private void miBlue_Click(object sender, System.EventArgs e)
        {
            myInkCollector.DefaultDrawingAttributes.Color = Color.Blue;
        }

        /// <summary>
        /// Event Handler from Ink->Color->Black Menu Item
        /// </summary>
        /// <param name="sender">The control that raised the event.</param>
        /// <param name="e">The event arguments.</param>
        private void miBlack_Click(object sender, System.EventArgs e)
        {
            myInkCollector.DefaultDrawingAttributes.Color = Color.Black;
        }

        /// <summary>
        /// Event Handler from Ink->Width->Thin Menu Item
        /// </summary>
        /// <param name="sender">The control that raised the event.</param>
        /// <param name="e">The event arguments.</param>
        private void miThin_Click(object sender, System.EventArgs e)
        {
            myInkCollector.DefaultDrawingAttributes.Width = ThinInkWidth;
        }

        /// <summary>
        /// Event Handler from Ink->Width->Medium Menu Item
        /// </summary>
        /// <param name="sender">The control that raised the event.</param>
        /// <param name="e">The event arguments.</param>
        private void miMedium_Click(object sender, System.EventArgs e)
        {
            myInkCollector.DefaultDrawingAttributes.Width = MediumInkWidth;
        }

        /// <summary>
        /// Event Handler from Ink->Width->Thick Menu Item
        /// </summary>
        /// <param name="sender">The control that raised the event.</param>
        /// <param name="e">The event arguments.</param>
        private void miThick_Click(object sender, System.EventArgs e)
        {
            myInkCollector.DefaultDrawingAttributes.Width = ThickInkWidth;
        }

        /// <summary>
        /// Event Handler from File->Exit Menu Item
        /// </summary>
        /// <param name="sender">The control that raised the event.</param>
        /// <param name="e">The event arguments.</param>
        private void miExit_Click(object sender, System.EventArgs e)
        {
            myInkCollector.Enabled = false;
            this.Dispose();
        }

        /// <summary>
        /// Event Handler from Ink->Enabled Menu Item
        /// </summary>
        /// <param name="sender">The control that raised the event.</param>
        /// <param name="e">The event arguments.</param>
        private void miEnabled_Click(object sender, System.EventArgs e)
        {
            // Toggle the enabled state of the ink collector
            myInkCollector.Enabled = !myInkCollector.Enabled;
            miEnabled.Checked = myInkCollector.Enabled;
        }

    }
}

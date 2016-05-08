// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//  File: PaperForm.cs
//  PaperForm Ink Controls Sample Application
//
//  This sample program demonstrates how the InkPicture control
//  can be used in conjunction with a DataSet to allow the user
//  to fill in a scanned form.  The DataSet contains a reference
//  to the form image and the location of each element in the form.
//  The InkPicture control uses this information to display the
//  form and calculate recognition results for each form element.
//
//  The features used are: InkPicture used in conjunction with
//  a DataSet to allow the user to write over a scanned form.
//
//--------------------------------------------------------------------------

using System;
using System.Drawing;
using System.Windows.Forms;
using System.Data;
using System.IO;
using System.Text;

// The Ink namespace, which contains the Tablet PC Platform API
using Microsoft.Ink;

namespace PaperForm
{
    /// <summary>
    /// The PaperForm Sample Application form class
    /// </summary>
    public class PaperForm : System.Windows.Forms.Form
    {
        // DataSet containing the form's data
        private DataSet formData = null;

        // Declare the InkPicture control, which contains the scanned image
        private InkPicture inkPicture1 = null;

        // The name of the xml file containing the form data
        private String formDataFile = "formdata.xml";

        #region Standard Template Code

        private System.Windows.Forms.MenuItem menuItem4;
        private System.Windows.Forms.MainMenu mainMenu;
        private System.Windows.Forms.MenuItem miInk;
        private System.Windows.Forms.MenuItem miRecognize;
        private System.Windows.Forms.MenuItem miClear;
        private System.Windows.Forms.MenuItem miExit;
        private System.Windows.Forms.MenuItem miMode;
        private System.Windows.Forms.MenuItem miPen;
        private System.Windows.Forms.MenuItem miEdit;
        private System.Windows.Forms.MenuItem miEraser;
        private System.ComponentModel.Container components = null;
        #endregion    

        public PaperForm()
        {
            #region Standard Template Code
            //
            // Required for Windows Form Designer support
            //
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
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.inkPicture1 = new Microsoft.Ink.InkPicture();
            this.mainMenu = new System.Windows.Forms.MainMenu();
            this.miInk = new System.Windows.Forms.MenuItem();
            this.miRecognize = new System.Windows.Forms.MenuItem();
            this.miClear = new System.Windows.Forms.MenuItem();
            this.menuItem4 = new System.Windows.Forms.MenuItem();
            this.miExit = new System.Windows.Forms.MenuItem();
            this.miMode = new System.Windows.Forms.MenuItem();
            this.miPen = new System.Windows.Forms.MenuItem();
            this.miEdit = new System.Windows.Forms.MenuItem();
            this.miEraser = new System.Windows.Forms.MenuItem();
            this.SuspendLayout();
            // 
            // inkPicture1
            // 
            this.inkPicture1.Cursor = System.Windows.Forms.Cursors.Default;
            this.inkPicture1.InkEnabled = false;
            this.inkPicture1.MarginX = -1;
            this.inkPicture1.MarginY = -1;
            this.inkPicture1.Name = "inkPicture1";
            this.inkPicture1.Size = new System.Drawing.Size(717, 975);
            this.inkPicture1.SizeMode = System.Windows.Forms.PictureBoxSizeMode.AutoSize;
            this.inkPicture1.TabIndex = 4;
            this.inkPicture1.TabStop = false;
            // 
            // mainMenu
            // 
            this.mainMenu.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
                                                                                     this.miInk,
                                                                                     this.miMode});
            // 
            // miInk
            // 
            this.miInk.Index = 0;
            this.miInk.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
                                                                                  this.miRecognize,
                                                                                  this.miClear,
                                                                                  this.menuItem4,
                                                                                  this.miExit});
            this.miInk.Text = "&Ink";
            // 
            // miRecognize
            // 
            this.miRecognize.Index = 0;
            this.miRecognize.Text = "&Recognize";
            this.miRecognize.Click += new System.EventHandler(this.miRecognize_Click);
            // 
            // miClear
            // 
            this.miClear.Index = 1;
            this.miClear.Text = "&Clear";
            this.miClear.Click += new System.EventHandler(this.miClear_Click);
            // 
            // menuItem4
            // 
            this.menuItem4.Index = 2;
            this.menuItem4.Text = "-";
            // 
            // miExit
            // 
            this.miExit.Index = 3;
            this.miExit.Text = "&Exit";
            this.miExit.Click += new System.EventHandler(this.miExit_Click);
            // 
            // miMode
            // 
            this.miMode.Index = 1;
            this.miMode.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
                                                                                   this.miPen,
                                                                                   this.miEdit,
                                                                                   this.miEraser});
            this.miMode.Text = "&Mode";
            // 
            // miPen
            // 
            this.miPen.Checked = true;
            this.miPen.Index = 0;
            this.miPen.RadioCheck = true;
            this.miPen.Text = "&Pen";
            this.miPen.Click += new System.EventHandler(this.miPen_Click);
            // 
            // miEdit
            // 
            this.miEdit.Index = 1;
            this.miEdit.RadioCheck = true;
            this.miEdit.Text = "&Edit";
            this.miEdit.Click += new System.EventHandler(this.miEdit_Click);
            // 
            // miEraser
            // 
            this.miEraser.Index = 2;
            this.miEraser.RadioCheck = true;
            this.miEraser.Text = "E&raser";
            this.miEraser.Click += new System.EventHandler(this.miEraser_Click);
            // 
            // PaperForm
            // 
            this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
            this.AutoScroll = true;
            this.ClientSize = new System.Drawing.Size(719, 817);
            this.Controls.AddRange(new System.Windows.Forms.Control[] {
                                                                          this.inkPicture1});
            this.DockPadding.All = 2;
            this.Menu = this.mainMenu;
            this.Name = "PaperForm";
            this.Text = "Scanned Paper Form Demo";
            this.Load += new System.EventHandler(this.PaperForm_Load);
            this.ResumeLayout(false);

        }
        #endregion

        #region Standard Template Code
        /// <summary>
        /// The main entry point for the application.
        /// </summary>
        [STAThread]
        static void Main() 
        {
            Application.Run(new PaperForm());
        }
        #endregion

        /// <summary>
        /// Event Handler from PaperForm->Load Event
        /// </summary>
        /// <param name="sender">The control that raised the event.</param>
        /// <param name="e">The event arguments.</param>
        private void PaperForm_Load(object sender, System.EventArgs e)
        {

            String applicationPath = Path.GetDirectoryName(Application.ExecutablePath)+"\\";

            // Initialize the form's dataset
            try
            {
                // read the form's data from the given xml file
                formData = new DataSet("FormData");
                formData.ReadXml(applicationPath+formDataFile);

                // The overlay control's background image is referenced in the xml dataset
                inkPicture1.BackgroundImage = 
                    System.Drawing.Image.FromFile(applicationPath+
                    (string) formData.Tables["FormData"].Rows[0]["Image"]);
            }
            catch(FileNotFoundException error)
            {
                // If the xml or the scanned form image file are not available,
                // display an error and exit
                MessageBox.Show("A required data file was not found.  Please verify "+
                                "that the file exists in the same directory as PaperForm.exe "+
                                "and try again." + Environment.NewLine + Environment.NewLine +
                                error.ToString(), 
                                "PaperForm", 
                                MessageBoxButtons.OK);
                Application.Exit();
            }        
            // Enable ink collection for the ink picture control
            inkPicture1.InkEnabled = true;       
        }

        /// <summary>
        /// Event Handler from Clear Menu Item Click Event
        /// </summary>
        /// <param name="sender">The control that raised the event.</param>
        /// <param name="e">The event arguments.</param>
        private void miClear_Click(object sender, System.EventArgs e)
        {
            // Delete the ink strokes and redraw
            inkPicture1.Ink.DeleteStrokes(inkPicture1.Ink.Strokes);
            inkPicture1.Invalidate();       
        }

        /// <summary>
        /// Event Handler from Recognize Menu Item Click Event
        /// </summary>
        /// <param name="sender">The control that raised the event.</param>
        /// <param name="e">The event arguments.</param>
        private void miRecognize_Click(object sender, System.EventArgs e)
        {
            // Get the handwriting recognition results
            Recognize();   
        }

        /// <summary>
        /// Event Handler from Exit Menu Item Click Event
        /// </summary>
        /// <param name="sender">The control that raised the event.</param>
        /// <param name="e">The event arguments.</param>
        private void miExit_Click(object sender, System.EventArgs e)
        {
            inkPicture1.Enabled = false;
            Application.Exit();
        }

        /// <summary>
        /// Event Handler from Pen Menu Item Click Event
        /// </summary>
        /// <param name="sender">The control that raised the event.</param>
        /// <param name="e">The event arguments.</param>
        private void miPen_Click(object sender, System.EventArgs e)
        {
            UpdateEditMode(InkOverlayEditingMode.Ink);       
        }

        /// <summary>
        /// Event Handler from Edit Menu Item Click Event
        /// </summary>
        /// <param name="sender">The control that raised the event.</param>
        /// <param name="e">The event arguments.</param>
        private void miEdit_Click(object sender, System.EventArgs e)
        {
            UpdateEditMode(InkOverlayEditingMode.Select);   
        }

        /// <summary>
        /// Event Handler from Eraser Menu Item Click Event
        /// </summary>
        /// <param name="sender">The control that raised the event.</param>
        /// <param name="e">The event arguments.</param>
        private void miEraser_Click(object sender, System.EventArgs e)
        {
            UpdateEditMode(InkOverlayEditingMode.Delete);
        }

        /// <summary>
        /// UpdateEditMode is a helper method to switch the InkPicture's
        /// editing mode and update the form accordingly.
        /// </summary>
        /// <param name="mode">The new editing mode</param>
        private void UpdateEditMode(InkOverlayEditingMode mode)
        {
            if (!inkPicture1.CollectingInk)
            {
                // Ink collection must be disabled before modifying the edit mode
                inkPicture1.InkEnabled = false;
                inkPicture1.EditingMode = mode;
                inkPicture1.InkEnabled = true;

                // Update the menu UI to the new state
                miPen.Checked = InkOverlayEditingMode.Ink == mode;
                miEdit.Checked = InkOverlayEditingMode.Select == mode;
                miEraser.Checked = InkOverlayEditingMode.Delete == mode;
            }
            else 
            {
                // If user is actively inking, we cannot switch modes.
                MessageBox.Show("Cannot switch the editing mode while collecting ink.");
            }
        }

        /// <summary>
        /// Iterate through each row of the form data and 
        /// display the recognition results
        /// </summary>
        private void Recognize()
        {

            // Check to ensure that the user has at least one recognizer installed
            Recognizers inkRecognizers = new Recognizers();
            if (0 == inkRecognizers.Count)
            {
                MessageBox.Show(this, "There are no handwriting recognizers installed.  You need to have at least one in order to perform the recognition.");
            }
            else
            {
                StringBuilder buffer = new StringBuilder();

                // Iterate through the rows in the "FieldInfo" table
                foreach(DataRow row in formData.Tables["FieldInfo"].Rows)
                {
                    // get the metadata for the field
                    // Note that the DataSet contains a row for each field
                    // in the form.  It is assumed that the rows are in the
                    // same order as the fields in the form.  The DataSet
                    // has the following columns:
                    // Name:  the field's name
                    // Left, Top, Right, Bottom:  the coordinates of the field (in pixels)
                    string fieldname = (string) row["Name"];
                    Point pt1 = new Point((int) row["Left"], (int) row["Top"]);
                    Point pt2 = new Point((int) row["Right"], (int) row["Bottom"]);
                
                    using (Graphics g = CreateGraphics())
                    {

                        // Convert to ink space units
                        inkPicture1.Renderer.PixelToInkSpace(g, ref pt1);
                        inkPicture1.Renderer.PixelToInkSpace(g, ref pt2);

                    }
                    
                    // the rectangle for the region
                    Rectangle rc = new Rectangle(pt1.X, pt1.Y, pt2.X-pt1.X, pt2.Y-pt1.Y);

                    // find the strokes that intersect and lie inside of the rectangle
                    Strokes strokes = inkPicture1.Ink.HitTest(rc,70);

                    // recognize the handwriting
                    if (strokes.Count > 0)
                    {
                        buffer.Append(fieldname + " = " + strokes.ToString() + Environment.NewLine);
                    }
                }

                // Display the results
                if (buffer.Length > 0)
                {
                    MessageBox.Show(this, buffer.ToString());
                }
                else
                {
                    MessageBox.Show(this, "There aren't any recognition results.");
                }
            }
        }
    }
}

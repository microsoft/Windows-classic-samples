// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//  File: InkClipboard.cs
//  Simple Ink Clipboard with Selection Sample Application
//
//  This program demonstrates how to copy and paste ink into
//  another application such as Microsoft Word or Microsoft Paint.
//  It also allows the user to copy a selection of strokes and
//  paste the result into this application's existing ink object.  
//
//  The following clipboard formats are available:
//
//  Metafile
//  Enhanced Metafile
//  Bitmap
//  Ink Serialized Format
//  Text Ink 
//  Sketch Ink

//  TextInk and SketchInk are two flavors of Ink OLE Embeddable Objects
//  that Microsoft Word uses for ink that represents text or drawing 
//  respectively.  

//  It is possible to paste Ink Serialized Format, Text Ink, and
//  Sketch Ink into existing ink.
// 
//  In addition to the clipboard, this sample also illustrates how
//  to select strokes with a lasso.  The user can move selected
//  strokes and modify their drawing attributes.  This functionality
//  is a subset of the selection functionality already provided by 
//  the InkOverlay control;  it is implemented here for illustrative
//  purposes.
//
//  The features used are: InkCollector, Ink Clipboard support, and
//  HitTest with lasso.
//
//--------------------------------------------------------------------------

using System;
using System.Drawing;
using System.Drawing.Drawing2D;
using System.Collections;
using System.Windows.Forms;

// The Ink namespace, which contains the Tablet PC Platform API
using Microsoft.Ink;

namespace Microsoft.Samples.TabletPC.InkClipboard
{

    /// <summary>
    /// Enumeration of all possible application modes:
    /// 
    ///   Ink:             The user is drawing new strokes
    ///   LassoSelection:  The user is selecting strokes with a lasso
    ///   MoveInk:         The user is moving selected strokes (this is not a menu item)
    ///   
    /// </summary>
    public enum ApplicationMode
    {
        Ink,
        LassoSelection,
        MoveInk
    }

    /// <summary>
    /// The InkClipboard with Selection Sample Application form class
    /// </summary>
    public class InkClipboard : System.Windows.Forms.Form
    {

        // --------------- Constants ---------------

        // Declare constants for the pen widths used by this application.
        // Note that these constants are in high metric units (1 unit = .01mm)
        private const float ThinInkWidth = 10;
        private const float MediumInkWidth = 100;
        private const float ThickInkWidth = 200;

        // Declare constant for the size of the border around selected strokes
        private const int SelectedInkWidthIncrease = 105;

        // Declare constant for the size of a lasso point
        private const int DotSize = 6;

        // Declare constant for the spacing between lasso points
        private const int DotSpacing = 7;

        // Declare constant for the padding size of the rectangle that bounds
        // the selected strokes.
        private const int SelectionRectBuffer = 8;

        // Declare constant for the lasso hit test percent (specifies how much
        // of the stroke must fall within the lasso in order to be selected).
        private const float LassoPercent = 50;

        // The index for the x and y packet values.  This index is used
        // to retrieve the lasso point data in the Packets event handler.
        private const int XPacketIndex = 0;
        private const int YPacketIndex = 1;

        // --------------- Fields ---------------

        // Declare the Ink Collector object
        private InkCollector myInkCollector = null;

        // The current application mode:  inking, selecting, or moving the selection
        private ApplicationMode applicationMode = ApplicationMode.Ink;

        // The points in the selection lasso
        private ArrayList lassoPoints = null;

        // The array of rectangle selection handles
        private PictureBox[] selectionHandles;

        // The rectangle that bounds the selected strokes
        private Rectangle selectionRect = Rectangle.Empty;

        // The strokes that have been selected by the lasso
        private Strokes selectedStrokes = null;

        // The location where the move started (used when moving selected strokes)
        private Point startMoveLocation = Point.Empty;

        // The coordinates of the last lasso point drawn (empty if no dot has been drawn)
        private Point lastDrawnLassoDot = Point.Empty;

        // Declare the colors used in the selection lasso
        private Color dotEdgeColor = Color.White;
        private Color dotColor = SystemColors.Highlight;
        private Color connectorColor = Color.Black;

        // Declare the pens used to draw the selection lasso
        private Pen connectorPen = null;
        private Pen dotEdgePen = null;
        private Pen dotPen = null;

        // --------------- Constructor ---------------

        #region Standard Template Code

        private System.Windows.Forms.MenuItem miClear;
        private System.Windows.Forms.MenuItem miExit;
        private System.Windows.Forms.MenuItem miMainEdit;
        private System.Windows.Forms.MenuItem miCut;
        private System.Windows.Forms.MenuItem miCopy;
        private System.Windows.Forms.MenuItem miPaste;
        private System.Windows.Forms.MenuItem miInk;
        private System.Windows.Forms.MenuItem miMainFormat;
        private System.Windows.Forms.MenuItem miISF;
        private System.Windows.Forms.MenuItem miMetafile;
        private System.Windows.Forms.MenuItem miBMP;
        private System.Windows.Forms.MenuItem miEMF;
        private System.Windows.Forms.MenuItem miSInk;
        private System.Windows.Forms.MenuItem miTInk;
        private System.Windows.Forms.MenuItem miNoOLE;
        private System.Windows.Forms.MenuItem miThin;
        private System.Windows.Forms.MenuItem miMedium;
        private System.Windows.Forms.MenuItem miThick;
        private System.Windows.Forms.MenuItem miRed;
        private System.Windows.Forms.MenuItem miGreen;
        private System.Windows.Forms.MenuItem miBlue;
        private System.Windows.Forms.MenuItem miBlack;
        private System.Windows.Forms.PictureBox rightBottomHandle;
        private System.Windows.Forms.PictureBox centerBottomHandle;
        private System.Windows.Forms.PictureBox leftBottomHandle;
        private System.Windows.Forms.PictureBox rightCenterHandle;
        private System.Windows.Forms.PictureBox leftCenterHandle;
        private System.Windows.Forms.PictureBox rightTopHandle;
        private System.Windows.Forms.PictureBox centerTopHandle;
        private System.Windows.Forms.PictureBox leftTopHandle;
        private System.Windows.Forms.MenuItem miLassoSelect;
        private System.Windows.Forms.MainMenu mainMenu;
        private System.Windows.Forms.MenuItem miInkSeparator;
        private System.Windows.Forms.MenuItem miEditSeparator;
        private System.Windows.Forms.MenuItem miFormatSeparator;
        private System.Windows.Forms.MenuItem miStyle;
        private System.Windows.Forms.MenuItem miWidth;
        private System.Windows.Forms.MenuItem miColor;
        private System.Windows.Forms.MenuItem miMainAction;

        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.Container components = null;
        #endregion

        public InkClipboard()
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

                if (myInkCollector != null)
                {
                    myInkCollector.Dispose();
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
            System.Resources.ResourceManager resources = new System.Resources.ResourceManager(typeof(InkClipboard));
            this.mainMenu = new System.Windows.Forms.MainMenu();
            this.miMainAction = new System.Windows.Forms.MenuItem();
            this.miClear = new System.Windows.Forms.MenuItem();
            this.miInkSeparator = new System.Windows.Forms.MenuItem();
            this.miExit = new System.Windows.Forms.MenuItem();
            this.miMainEdit = new System.Windows.Forms.MenuItem();
            this.miCut = new System.Windows.Forms.MenuItem();
            this.miCopy = new System.Windows.Forms.MenuItem();
            this.miPaste = new System.Windows.Forms.MenuItem();
            this.miEditSeparator = new System.Windows.Forms.MenuItem();
            this.miLassoSelect = new System.Windows.Forms.MenuItem();
            this.miInk = new System.Windows.Forms.MenuItem();
            this.miMainFormat = new System.Windows.Forms.MenuItem();
            this.miISF = new System.Windows.Forms.MenuItem();
            this.miMetafile = new System.Windows.Forms.MenuItem();
            this.miEMF = new System.Windows.Forms.MenuItem();
            this.miBMP = new System.Windows.Forms.MenuItem();
            this.miFormatSeparator = new System.Windows.Forms.MenuItem();
            this.miSInk = new System.Windows.Forms.MenuItem();
            this.miTInk = new System.Windows.Forms.MenuItem();
            this.miNoOLE = new System.Windows.Forms.MenuItem();
            this.miStyle = new System.Windows.Forms.MenuItem();
            this.miWidth = new System.Windows.Forms.MenuItem();
            this.miThin = new System.Windows.Forms.MenuItem();
            this.miMedium = new System.Windows.Forms.MenuItem();
            this.miThick = new System.Windows.Forms.MenuItem();
            this.miColor = new System.Windows.Forms.MenuItem();
            this.miRed = new System.Windows.Forms.MenuItem();
            this.miGreen = new System.Windows.Forms.MenuItem();
            this.miBlue = new System.Windows.Forms.MenuItem();
            this.miBlack = new System.Windows.Forms.MenuItem();
            this.rightBottomHandle = new System.Windows.Forms.PictureBox();
            this.centerBottomHandle = new System.Windows.Forms.PictureBox();
            this.leftBottomHandle = new System.Windows.Forms.PictureBox();
            this.rightCenterHandle = new System.Windows.Forms.PictureBox();
            this.leftCenterHandle = new System.Windows.Forms.PictureBox();
            this.rightTopHandle = new System.Windows.Forms.PictureBox();
            this.centerTopHandle = new System.Windows.Forms.PictureBox();
            this.leftTopHandle = new System.Windows.Forms.PictureBox();
            this.SuspendLayout();
            // 
            // mainMenu
            // 
            this.mainMenu.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
                                                                                     this.miMainAction,
                                                                                     this.miMainEdit,
                                                                                     this.miMainFormat,
                                                                                     this.miStyle});
            // 
            // miMainAction
            // 
            this.miMainAction.Index = 0;
            this.miMainAction.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
                                                                                         this.miClear,
                                                                                         this.miInkSeparator,
                                                                                         this.miExit});
            this.miMainAction.Text = "&Action";
            // 
            // miClear
            // 
            this.miClear.Index = 0;
            this.miClear.Text = "&Clear";
            this.miClear.Click += new System.EventHandler(this.miClear_Click);
            // 
            // miInkSeparator
            // 
            this.miInkSeparator.Index = 1;
            this.miInkSeparator.Text = "-";
            // 
            // miExit
            // 
            this.miExit.Index = 2;
            this.miExit.Text = "&Exit";
            this.miExit.Click += new System.EventHandler(this.miExit_Click);
            // 
            // miMainEdit
            // 
            this.miMainEdit.Index = 1;
            this.miMainEdit.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
                                                                                       this.miCut,
                                                                                       this.miCopy,
                                                                                       this.miPaste,
                                                                                       this.miEditSeparator,
                                                                                       this.miLassoSelect,
                                                                                       this.miInk});
            this.miMainEdit.Text = "&Edit";
            this.miMainEdit.Popup += new System.EventHandler(this.miMainEdit_Popup);
            // 
            // miCut
            // 
            this.miCut.Index = 0;
            this.miCut.Shortcut = System.Windows.Forms.Shortcut.CtrlX;
            this.miCut.Text = "Cu&t";
            this.miCut.Click += new System.EventHandler(this.miCut_Click);
            // 
            // miCopy
            // 
            this.miCopy.Index = 1;
            this.miCopy.Shortcut = System.Windows.Forms.Shortcut.CtrlC;
            this.miCopy.Text = "&Copy";
            this.miCopy.Click += new System.EventHandler(this.miCopy_Click);
            // 
            // miPaste
            // 
            this.miPaste.Index = 2;
            this.miPaste.Shortcut = System.Windows.Forms.Shortcut.CtrlV;
            this.miPaste.Text = "&Paste";
            this.miPaste.Click += new System.EventHandler(this.miPaste_Click);
            // 
            // miEditSeparator
            // 
            this.miEditSeparator.Index = 3;
            this.miEditSeparator.Text = "-";
            // 
            // miLassoSelect
            // 
            this.miLassoSelect.Index = 4;
            this.miLassoSelect.RadioCheck = true;
            this.miLassoSelect.Text = "&Select";
            this.miLassoSelect.Click += new System.EventHandler(this.miLassoSelect_Click);
            // 
            // miInk
            // 
            this.miInk.Checked = true;
            this.miInk.Index = 5;
            this.miInk.RadioCheck = true;
            this.miInk.Text = "&Ink";
            this.miInk.Click += new System.EventHandler(this.miInk_Click);
            // 
            // miMainFormat
            // 
            this.miMainFormat.Index = 2;
            this.miMainFormat.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
                                                                                         this.miISF,
                                                                                         this.miMetafile,
                                                                                         this.miEMF,
                                                                                         this.miBMP,
                                                                                         this.miFormatSeparator,
                                                                                         this.miSInk,
                                                                                         this.miTInk,
                                                                                         this.miNoOLE});
            this.miMainFormat.Text = "&Format";
            // 
            // miISF
            // 
            this.miISF.Checked = true;
            this.miISF.Index = 0;
            this.miISF.Text = "&Ink Serialized Format (ISF)";
            this.miISF.Click += new System.EventHandler(this.miISF_Click);
            // 
            // miMetafile
            // 
            this.miMetafile.Checked = true;
            this.miMetafile.Index = 1;
            this.miMetafile.Text = "&Metafile";
            this.miMetafile.Click += new System.EventHandler(this.miMetafile_Click);
            // 
            // miEMF
            // 
            this.miEMF.Checked = true;
            this.miEMF.Index = 2;
            this.miEMF.Text = "&Enhanced Metafile";
            this.miEMF.Click += new System.EventHandler(this.miEMF_Click);
            // 
            // miBMP
            // 
            this.miBMP.Checked = true;
            this.miBMP.Index = 3;
            this.miBMP.Text = "&Bitmap";
            this.miBMP.Click += new System.EventHandler(this.miBMP_Click);
            // 
            // miFormatSeparator
            // 
            this.miFormatSeparator.Index = 4;
            this.miFormatSeparator.Text = "-";
            // 
            // miSInk
            // 
            this.miSInk.Index = 5;
            this.miSInk.RadioCheck = true;
            this.miSInk.Text = "&Sketch Ink";
            this.miSInk.Click += new System.EventHandler(this.miSInk_Click);
            // 
            // miTInk
            // 
            this.miTInk.Checked = true;
            this.miTInk.Index = 6;
            this.miTInk.RadioCheck = true;
            this.miTInk.Text = "&Text Ink";
            this.miTInk.Click += new System.EventHandler(this.miTInk_Click);
            // 
            // miNoOLE
            // 
            this.miNoOLE.Index = 7;
            this.miNoOLE.RadioCheck = true;
            this.miNoOLE.Text = "&No OLE Object Format";
            this.miNoOLE.Click += new System.EventHandler(this.miNoOLE_Click);
            // 
            // miStyle
            // 
            this.miStyle.Index = 3;
            this.miStyle.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
                                                                                    this.miWidth,
                                                                                    this.miColor});
            this.miStyle.Text = "&Style";
            // 
            // miWidth
            // 
            this.miWidth.Index = 0;
            this.miWidth.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
                                                                                    this.miThin,
                                                                                    this.miMedium,
                                                                                    this.miThick});
            this.miWidth.Text = "&Width";
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
            // miColor
            // 
            this.miColor.Index = 1;
            this.miColor.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
                                                                                    this.miRed,
                                                                                    this.miGreen,
                                                                                    this.miBlue,
                                                                                    this.miBlack});
            this.miColor.Text = "&Color";
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
            // rightBottomHandle
            // 
            this.rightBottomHandle.BackColor = System.Drawing.Color.Gray;
            this.rightBottomHandle.Cursor = System.Windows.Forms.Cursors.Default;
            this.rightBottomHandle.Image = ((System.Drawing.Bitmap)(resources.GetObject("rightBottomHandle.Image")));
            this.rightBottomHandle.Location = new System.Drawing.Point(281, 256);
            this.rightBottomHandle.Name = "rightBottomHandle";
            this.rightBottomHandle.Size = new System.Drawing.Size(11, 11);
            this.rightBottomHandle.SizeMode = System.Windows.Forms.PictureBoxSizeMode.AutoSize;
            this.rightBottomHandle.TabIndex = 20;
            this.rightBottomHandle.TabStop = false;
            this.rightBottomHandle.Visible = false;
            // 
            // centerBottomHandle
            // 
            this.centerBottomHandle.Cursor = System.Windows.Forms.Cursors.Default;
            this.centerBottomHandle.Image = ((System.Drawing.Bitmap)(resources.GetObject("centerBottomHandle.Image")));
            this.centerBottomHandle.Location = new System.Drawing.Point(137, 256);
            this.centerBottomHandle.Name = "centerBottomHandle";
            this.centerBottomHandle.Size = new System.Drawing.Size(9, 9);
            this.centerBottomHandle.SizeMode = System.Windows.Forms.PictureBoxSizeMode.AutoSize;
            this.centerBottomHandle.TabIndex = 19;
            this.centerBottomHandle.TabStop = false;
            this.centerBottomHandle.Visible = false;
            // 
            // leftBottomHandle
            // 
            this.leftBottomHandle.BackColor = System.Drawing.Color.Gray;
            this.leftBottomHandle.Cursor = System.Windows.Forms.Cursors.Default;
            this.leftBottomHandle.Image = ((System.Drawing.Bitmap)(resources.GetObject("leftBottomHandle.Image")));
            this.leftBottomHandle.Location = new System.Drawing.Point(1, 256);
            this.leftBottomHandle.Name = "leftBottomHandle";
            this.leftBottomHandle.Size = new System.Drawing.Size(11, 11);
            this.leftBottomHandle.SizeMode = System.Windows.Forms.PictureBoxSizeMode.AutoSize;
            this.leftBottomHandle.TabIndex = 18;
            this.leftBottomHandle.TabStop = false;
            this.leftBottomHandle.Visible = false;
            // 
            // rightCenterHandle
            // 
            this.rightCenterHandle.Cursor = System.Windows.Forms.Cursors.Default;
            this.rightCenterHandle.Image = ((System.Drawing.Bitmap)(resources.GetObject("rightCenterHandle.Image")));
            this.rightCenterHandle.Location = new System.Drawing.Point(281, 128);
            this.rightCenterHandle.Name = "rightCenterHandle";
            this.rightCenterHandle.Size = new System.Drawing.Size(9, 9);
            this.rightCenterHandle.SizeMode = System.Windows.Forms.PictureBoxSizeMode.AutoSize;
            this.rightCenterHandle.TabIndex = 17;
            this.rightCenterHandle.TabStop = false;
            this.rightCenterHandle.Visible = false;
            // 
            // leftCenterHandle
            // 
            this.leftCenterHandle.Cursor = System.Windows.Forms.Cursors.Default;
            this.leftCenterHandle.Image = ((System.Drawing.Bitmap)(resources.GetObject("leftCenterHandle.Image")));
            this.leftCenterHandle.Location = new System.Drawing.Point(1, 128);
            this.leftCenterHandle.Name = "leftCenterHandle";
            this.leftCenterHandle.Size = new System.Drawing.Size(9, 9);
            this.leftCenterHandle.SizeMode = System.Windows.Forms.PictureBoxSizeMode.AutoSize;
            this.leftCenterHandle.TabIndex = 16;
            this.leftCenterHandle.TabStop = false;
            this.leftCenterHandle.Visible = false;
            // 
            // rightTopHandle
            // 
            this.rightTopHandle.BackColor = System.Drawing.Color.Gray;
            this.rightTopHandle.Cursor = System.Windows.Forms.Cursors.Default;
            this.rightTopHandle.Image = ((System.Drawing.Bitmap)(resources.GetObject("rightTopHandle.Image")));
            this.rightTopHandle.Location = new System.Drawing.Point(281, 0);
            this.rightTopHandle.Name = "rightTopHandle";
            this.rightTopHandle.Size = new System.Drawing.Size(11, 11);
            this.rightTopHandle.SizeMode = System.Windows.Forms.PictureBoxSizeMode.AutoSize;
            this.rightTopHandle.TabIndex = 15;
            this.rightTopHandle.TabStop = false;
            this.rightTopHandle.Visible = false;
            // 
            // centerTopHandle
            // 
            this.centerTopHandle.Cursor = System.Windows.Forms.Cursors.Default;
            this.centerTopHandle.Image = ((System.Drawing.Bitmap)(resources.GetObject("centerTopHandle.Image")));
            this.centerTopHandle.Location = new System.Drawing.Point(137, 0);
            this.centerTopHandle.Name = "centerTopHandle";
            this.centerTopHandle.Size = new System.Drawing.Size(9, 9);
            this.centerTopHandle.SizeMode = System.Windows.Forms.PictureBoxSizeMode.AutoSize;
            this.centerTopHandle.TabIndex = 14;
            this.centerTopHandle.TabStop = false;
            this.centerTopHandle.Visible = false;
            // 
            // leftTopHandle
            // 
            this.leftTopHandle.BackColor = System.Drawing.Color.Gray;
            this.leftTopHandle.Cursor = System.Windows.Forms.Cursors.Default;
            this.leftTopHandle.Image = ((System.Drawing.Bitmap)(resources.GetObject("leftTopHandle.Image")));
            this.leftTopHandle.Location = new System.Drawing.Point(1, 0);
            this.leftTopHandle.Name = "leftTopHandle";
            this.leftTopHandle.Size = new System.Drawing.Size(11, 11);
            this.leftTopHandle.SizeMode = System.Windows.Forms.PictureBoxSizeMode.AutoSize;
            this.leftTopHandle.TabIndex = 13;
            this.leftTopHandle.TabStop = false;
            this.leftTopHandle.Visible = false;
            // 
            // InkClipboard
            // 
            this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
            this.ClientSize = new System.Drawing.Size(292, 266);
            this.Controls.AddRange(new System.Windows.Forms.Control[] {
                                                                          this.rightBottomHandle,
                                                                          this.centerBottomHandle,
                                                                          this.leftBottomHandle,
                                                                          this.rightCenterHandle,
                                                                          this.leftCenterHandle,
                                                                          this.rightTopHandle,
                                                                          this.centerTopHandle,
                                                                          this.leftTopHandle});
            this.Menu = this.mainMenu;
            this.Name = "InkClipboard";
            this.Text = "InkClipboard";
            this.MouseDown += new System.Windows.Forms.MouseEventHandler(this.InkClipboard_MouseDown);
            this.Load += new System.EventHandler(this.InkClipboard_Load);
            this.MouseUp += new System.Windows.Forms.MouseEventHandler(this.InkClipboard_MouseUp);
            this.Paint += new System.Windows.Forms.PaintEventHandler(this.InkClipboard_Paint);
            this.MouseMove += new System.Windows.Forms.MouseEventHandler(this.InkClipboard_MouseMove);
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
            Application.Run(new InkClipboard());
        }
        #endregion


        // --------------- Form Events ---------------

        /// <summary>
        /// Event Handler from Form Load Event
        /// Setup the ink collector for collection
        /// </summary>
        /// <param name="sender">The control that raised the event.</param>
        /// <param name="e">The event arguments.</param>
        private void InkClipboard_Load(object sender, System.EventArgs e)
        {
            // Set the application mode to inking
            applicationMode = ApplicationMode.Ink;

            // Initialize the selection data
            lassoPoints = new ArrayList();
            selectionHandles = new PictureBox[] {leftTopHandle,centerTopHandle,rightTopHandle,leftCenterHandle,rightCenterHandle,leftBottomHandle,centerBottomHandle,rightBottomHandle};
            selectionRect = new Rectangle();
            selectedStrokes = null;

            // Create the pens used to draw the lasso selection
            connectorPen = new Pen(connectorColor);     // Pen used to draw dotted lasso connector line
            connectorPen.DashStyle = DashStyle.Dash;
            dotEdgePen = new Pen(dotEdgeColor);         // Pen used to draw the outer edge of the lasso dot
            dotPen = new Pen(dotColor);                 // Pen used to draw the center of the lasso dot

            // Create a new ink collector and assign it to this form's window
            myInkCollector = new InkCollector(this.Handle);

            // Set the ink collector's pen width
            myInkCollector.DefaultDrawingAttributes.Width = MediumInkWidth;

            // Turn the ink collector on
            myInkCollector.Enabled = true;

        }

        /// <summary>
        /// Event Handler from Form Paint Event
        /// 
        /// When no ink is selected, autoredrawing is turned on and
        /// this method doesn't need to do anything.  However, if 
        /// there is a selection, autoredrawing is turned off and
        /// it is necessary to draw the strokes manually
        /// (in order to display the strokes as selected).
        /// 
        /// </summary>
        /// <param name="sender">The control that raised the event.</param>
        /// <param name="e">The event arguments.</param>
        private void InkClipboard_Paint(object sender, System.Windows.Forms.PaintEventArgs e)
        {
            // If AutoRedraw is turned off, perform the drawing manually.
            if (!myInkCollector.AutoRedraw)
            {

                DrawStrokes(e.Graphics);
                
                e.Graphics.DrawRectangle(connectorPen,selectionRect);
            }

            // If the user is drawing a lasso, repaint the lasso points.
            if (ApplicationMode.LassoSelection == applicationMode)
            {
                DrawLasso(e.Graphics, 0);
            }
        }

        /// <summary>
        /// Event Handler from Form Mouse Down Event
        /// </summary>
        /// <param name="sender">The control that raised the event.</param>
        /// <param name="e">The event arguments.</param>
        private void InkClipboard_MouseDown(object sender, System.Windows.Forms.MouseEventArgs e)
        {
            // If the mouse button was pressed down within the
            // current selection area, update the application mode
            // to indicate that the user is moving ink and record
            // the starting location of the move.  This value will 
            // be used in MouseMove to determine how much the selected
            // ink should be moved.
            if (selectionRect.Contains(e.X,e.Y))
            {
                applicationMode = ApplicationMode.MoveInk;

                startMoveLocation = new Point(e.X, e.Y);
            }
                // Otherwise, if there is a selection and the mouse
                // is pressed, clear the selection.
            else if (HasSelection())
            {
                SetSelection(null);
            }
        }

        /// <summary>
        /// Event Handler from Form Mouse Move Event
        /// </summary>
        /// <param name="sender">The control that raised the event.</param>
        /// <param name="e">The event arguments.</param>
        private void InkClipboard_MouseMove(object sender, System.Windows.Forms.MouseEventArgs e)
        {
            // If the selected ink is currently being moved...
            if (applicationMode == ApplicationMode.MoveInk && HasSelection())
            {
                // Compute the ink space distance the mouse has moved and
                // move the selected strokes correspondingly.
                Point moveAmount = new Point(e.X-startMoveLocation.X,e.Y-startMoveLocation.Y);
                using (Graphics g = CreateGraphics())
                {
                    myInkCollector.Renderer.PixelToInkSpace(g, ref moveAmount);
                }
                selectedStrokes.Move(moveAmount.X, moveAmount.Y);

                // Record the location of the mouse since the next
                // move should be relative to it.
                startMoveLocation.X = e.X;
                startMoveLocation.Y = e.Y;

                // Update the selection rectangle
                SetSelection(selectedStrokes);

            }
            else
            {

                // If the mouse has entered the rectangular selection
                // region, disable ink collection and update the cursor
                // to show that the user can move this region.  It is
                // necessary to disable ink collection since ink should not
                // flow from the pen while the user is moving the selection.
                if ( selectionRect.Contains(e.X,e.Y) )
                {
                    if (myInkCollector.Enabled && !myInkCollector.CollectingInk)
                    {
                        myInkCollector.Enabled = false;
                        Cursor = System.Windows.Forms.Cursors.SizeAll;
                    }
                }
                    // Otherwise, enable ink collection and set the cursor
                    // to its default value (if necessary).
                else if (!myInkCollector.Enabled)
                {                                 
                    myInkCollector.Enabled = true;
                    Cursor = System.Windows.Forms.Cursors.Arrow;
                }
            }
        }



        /// <summary>
        /// Event Handler from Form Mouse Up Event
        /// </summary>
        /// <param name="sender">The control that raised the event.</param>
        /// <param name="e">The event arguments.</param>
        private void InkClipboard_MouseUp(object sender, System.Windows.Forms.MouseEventArgs e)
        {
            // If the user was moving selected ink and the
            // mouse is raised, set the application mode back to
            // its original value.
            if (applicationMode == ApplicationMode.MoveInk)
            {
                if (miLassoSelect.Checked)
                {
                    applicationMode = ApplicationMode.LassoSelection;
                }
                else
                {
                    applicationMode = ApplicationMode.Ink;
                }
            }
        }

        /// <summary>
        /// Event Handler from Ink->Clear Menu Item
        /// </summary>
        /// <param name="sender">The control that raised the event.</param>
        /// <param name="e">The event arguments.</param>  
        private void miClear_Click(object sender, System.EventArgs e)
        {
            // Clear the selection and delete all strokes from the ink
            myInkCollector.Ink.DeleteStrokes();
            SetSelection(null);

        }

        /// <summary>
        /// Event Handler from Ink->Exit Menu Item
        /// </summary>
        /// <param name="sender">The control that raised the event.</param>
        /// <param name="e">The event arguments.</param>  
        private void miExit_Click(object sender, System.EventArgs e)
        {
            // Although not required, it is good practice to disable
            // ink collection before exiting
            myInkCollector.Enabled = false;

            Application.Exit();
        }

        /// <summary>
        /// Event Handler from Edit Select Event
        /// 
        /// If the Edit menu is selected, check whether the object
        /// that is currently on the clipboard can be pasted into 
        /// the ink.  Disable the Paste item if it can't be pasted.
        /// Also, disable Copy and Cut if no ink is selected.
        /// 
        ///  
        /// Allowable formats:  Ink Serialized Format or Embeddable OLE
        /// Object (Text Ink or Sketch Ink). 
        /// 
        /// </summary>
        /// <param name="sender">The control that raised the event.</param>
        /// <param name="e">The event arguments.</param>       
        private void miMainEdit_Popup(object sender, System.EventArgs e)
        {
            miPaste.Enabled = myInkCollector.Ink.CanPaste();
            miCopy.Enabled = HasSelection();
            miCut.Enabled = HasSelection();
        }

        /// <summary>
        /// Event Handler from Edit->Cut Menu Item
        /// </summary>
        /// <param name="sender">The control that raised the event.</param>
        /// <param name="e">The event arguments.</param>  
        private void miCut_Click(object sender, System.EventArgs e)
        {

            // Use helper method to cut the selection
            CopyInkToClipboard(InkClipboardModes.Cut);

            SetSelection(null);

        }

        /// <summary>
        /// Event Handler from Edit->Copy Menu Item
        /// </summary>
        /// <param name="sender">The control that raised the event.</param>
        /// <param name="e">The event arguments.</param>  
        private void miCopy_Click(object sender, System.EventArgs e)
        {

            // Use helper method to copy the selection
            CopyInkToClipboard(InkClipboardModes.Copy);

        }

        /// <summary>
        /// Event Handler from Edit->Paste Menu Item
        /// </summary>
        /// <param name="sender">The control that raised the event.</param>
        /// <param name="e">The event arguments.</param>   
        private void miPaste_Click(object sender, System.EventArgs e)
        {
            if (myInkCollector.Ink.CanPaste())
            {
                // Compute the location where the ink should be pasted;
                // this location should be shifted from the origin
                // to account for the width of the selection rectangle's handle.
                Point offset = new Point(leftTopHandle.Width+1,leftTopHandle.Height+1);
                using (Graphics g = CreateGraphics())
                {
                    myInkCollector.Renderer.PixelToInkSpace(g, ref offset);
                }

                // Use Ink API to paste the clipboard data into the Ink
                Strokes pastedStrokes = myInkCollector.Ink.ClipboardPaste(offset);
            
                // If the contents of the clipboard were a valid format 
                // (Ink Serialized Format or Embeddable OLE Object) and at
                // least one stroke was pasted into the ink, use a helper 
                // method to update the stroke selection.  Otherwise,
                // the result will be null and this paste becomes a no-op.  
                if (null != pastedStrokes)
                {
                    SetSelection(pastedStrokes);
                }

            }
            else
            {
                MessageBox.Show("Unable to paste ink.");
            }
        }

        /// <summary>
        /// Event Handler from Edit->Select Menu Item
        /// </summary>
        /// <param name="sender">The control that raised the event.</param>
        /// <param name="e">The event arguments.</param>  
        private void miLassoSelect_Click(object sender, System.EventArgs e)
        {

            if (ApplicationMode.LassoSelection != applicationMode)
            {
                // Set the application mode to lasso selection
                applicationMode = ApplicationMode.LassoSelection;

                // Lasso strokes should be transparent 
                myInkCollector.DefaultDrawingAttributes.Transparency = 255;

                // Use helper method to clear the selection
                SetSelection(null);

                // Update menu item state
                miLassoSelect.Checked = true;
                miInk.Checked = false;  

                // Hook the new packets and stroke events since
                // these events are needed to implement lasso selection.
                // These events are only hooked when they are needed to improve
                // performance.
                myInkCollector.NewPackets += new InkCollectorNewPacketsEventHandler(myInkCollector_NewPackets);
                myInkCollector.Stroke += new InkCollectorStrokeEventHandler(myInkCollector_Stroke);  
            }

        }


        /// <summary>
        /// Event Handler from Edit->Ink Menu Item
        /// </summary>
        /// <param name="sender">The control that raised the event.</param>
        /// <param name="e">The event arguments.</param>  
        private void miInk_Click(object sender, System.EventArgs e)
        {

            if (applicationMode != ApplicationMode.Ink) 
            {
                // Set the application mode to ink collection
                applicationMode = ApplicationMode.Ink;

                // Ink strokes should be opaque
                myInkCollector.DefaultDrawingAttributes.Transparency = 0;

                // Use helper method to clear the selection
                SetSelection(null);

                // Update menu item state
                miLassoSelect.Checked = false;
                miInk.Checked = true; 

                // Unhook the new packets and stroke events since
                // these events are only needed to implement lasso selection.
                // These events are only hooked when they are needed to improve
                // performance.
                myInkCollector.NewPackets -= new InkCollectorNewPacketsEventHandler(myInkCollector_NewPackets);
                myInkCollector.Stroke -= new InkCollectorStrokeEventHandler(myInkCollector_Stroke);

            }
   
        }

        /// <summary>
        /// Event Handler from Format->ISF Menu Item
        /// </summary>
        /// <param name="sender">The control that raised the event.</param>
        /// <param name="e">The event arguments.</param>  
        private void miISF_Click(object sender, System.EventArgs e)
        {
            miISF.Checked = !miISF.Checked;
        }

        /// <summary>
        /// Event Handler from Metafile->ISF Menu Item
        /// </summary>
        /// <param name="sender">The control that raised the event.</param>
        /// <param name="e">The event arguments.</param>  
        private void miMetafile_Click(object sender, System.EventArgs e)
        {
            miMetafile.Checked = !miMetafile.Checked;
        }

        /// <summary>
        /// Event Handler from Format->EMF Menu Item
        /// </summary>
        /// <param name="sender">The control that raised the event.</param>
        /// <param name="e">The event arguments.</param>  
        private void miEMF_Click(object sender, System.EventArgs e)
        {
            miEMF.Checked = !miEMF.Checked;
        }

        /// <summary>
        /// Event Handler from Format->BMP Menu Item
        /// </summary>
        /// <param name="sender">The control that raised the event.</param>
        /// <param name="e">The event arguments.</param>  
        private void miBMP_Click(object sender, System.EventArgs e)
        {
            miBMP.Checked = !miBMP.Checked;
        }

        /// <summary>
        /// Event Handler from Format->Text Ink Menu Item
        /// </summary>
        /// <param name="sender">The control that raised the event.</param>
        /// <param name="e">The event arguments.</param>  
        private void miTInk_Click(object sender, System.EventArgs e)
        {
            miSInk.Checked = false;
            miTInk.Checked = true;
            miNoOLE.Checked = false;
        }

        /// <summary>
        /// Event Handler from Format->Sketch Ink Menu Item
        /// </summary>
        /// <param name="sender">The control that raised the event.</param>
        /// <param name="e">The event arguments.</param>  
        private void miSInk_Click(object sender, System.EventArgs e)
        {
            miSInk.Checked = true;
            miTInk.Checked = false;
            miNoOLE.Checked = false;       
        }

        /// <summary>
        /// Event Handler from Format->No OLE Object Menu Item
        /// </summary>
        /// <param name="sender">The control that raised the event.</param>
        /// <param name="e">The event arguments.</param>  
        private void miNoOLE_Click(object sender, System.EventArgs e)
        {
            miSInk.Checked = false;
            miTInk.Checked = false;
            miNoOLE.Checked = true;       
        }

        /// <summary>
        /// Event Handler from Style->Color->Red Menu Item
        /// </summary>
        /// <param name="sender">The control that raised the event.</param>
        /// <param name="e">The event arguments.</param>
        private void miRed_Click(object sender, System.EventArgs e)
        {
            
            // Use helper method to set the InkCollector and the
            // selected strokes to the selected color.
            SetColor(Color.Red);

        }

        /// <summary>
        /// Event Handler from Style->Color->Green Menu Item
        /// </summary>
        /// <param name="sender">The control that raised the event.</param>
        /// <param name="e">The event arguments.</param>
        private void miGreen_Click(object sender, System.EventArgs e)
        {

            // Use helper method to set the InkCollector and the
            // selected strokes to the selected color.
            SetColor(Color.Green);

        }

        /// <summary>
        /// Event Handler from Style->Color->Blue Menu Item
        /// </summary>
        /// <param name="sender">The control that raised the event.</param>
        /// <param name="e">The event arguments.</param>
        private void miBlue_Click(object sender, System.EventArgs e)
        {

            // Use helper method to set the InkCollector and the
            // selected strokes to the selected color.
            SetColor(Color.Blue);

        }

        /// <summary>
        /// Event Handler from Style->Color->Black Menu Item
        /// </summary>
        /// <param name="sender">The control that raised the event.</param>
        /// <param name="e">The event arguments.</param>
        private void miBlack_Click(object sender, System.EventArgs e)
        {

            // Use helper method to set the InkCollector and the
            // selected strokes to the selected color.
            SetColor(Color.Black);

        }

        /// <summary>
        /// Event Handler from Style->Width->Thin Menu Item
        /// </summary>
        /// <param name="sender">The control that raised the event.</param>
        /// <param name="e">The event arguments.</param>
        private void miThin_Click(object sender, System.EventArgs e)
        {

            // Use helper method to set the InkCollector and the
            // selected strokes to the selected pen width.
            SetWidth(ThinInkWidth);

        }

        /// <summary>
        /// Event Handler from Style->Width->Medium Menu Item
        /// </summary>
        /// <param name="sender">The control that raised the event.</param>
        /// <param name="e">The event arguments.</param>
        private void miMedium_Click(object sender, System.EventArgs e)
        {

            // Use helper method to set the InkCollector and the
            // selected strokes to the selected pen width.
            SetWidth(MediumInkWidth);

        }

        /// <summary>
        /// Event Handler from Style->Width->Thick Menu Item
        /// </summary>
        /// <param name="sender">The control that raised the event.</param>
        /// <param name="e">The event arguments.</param>
        private void miThick_Click(object sender, System.EventArgs e)
        {

            // Use helper method to set the InkCollector and the
            // selected strokes to the selected pen width.
            SetWidth(ThickInkWidth);

        }

        // --------------- Ink Collector Events ---------------

        /// <summary>
        /// Event Handler from Ink Collector's NewPackets event
        /// 
        /// This event is fired when the Ink Collector receives 
        /// new packet data and the user is drawing a lasso.  
        /// In this case,  it is necessary to intercept the new packets
        /// and use them to draw the selection lasso.
        /// </summary>
        /// <param name="sender">The control that raised the event.</param>
        /// <param name="e">The event arguments.</param>
        private void myInkCollector_NewPackets(object sender, InkCollectorNewPacketsEventArgs e)
        {

            // retrieve the size of each packet
            int packetDescriptionLength = e.Stroke.PacketDescription.Length;
        
            // the index where the new lasso points will be added
            int newPacketIndex = lassoPoints.Count;

            using (Graphics g = CreateGraphics())
            {

                // Add each new packet into the lasso points collection
                for (int i = 0; i < e.PacketCount; i++)
                {

                    // retrieve the x and y packet values 
                    Point pt = new Point(e.PacketData[i * packetDescriptionLength + XPacketIndex],
                                         e.PacketData[i * packetDescriptionLength + YPacketIndex]);

                    myInkCollector.Renderer.InkSpaceToPixel(g, ref pt);

                    // If the x or y packet values fall outside of
                    // the drawing area , update them to the nearest
                    // location on the drawing area.  This adjustment
                    // prevents the lasso's boundaries from going 
                    // beyond the drawing area.
                    if (pt.X<0)
                    {
                        pt.X = 0;
                    }
                    if (pt.X>ClientSize.Width)
                    {
                        pt.X = ClientSize.Width;
                    }
                    if (pt.Y<0)
                    {
                        pt.Y = 0;
                    }
                    if (pt.Y>this.ClientSize.Height)
                    {
                        pt.Y = ClientSize.Height;
                    }
    
                    // Add the new point into the lasso points collection
                    lassoPoints.Add(pt); 

                }
              
                // Draw the updated lasso.  In order to improve performance,
                // only the new lasso packets will be drawn.
                DrawLasso(g,newPacketIndex);
            }
        }

        /// <summary>
        /// Event Handler from Ink Collector's Stroke event
        /// 
        /// This event is fired when a new lasso stroke is drawn.  
        /// In this case, it is necessary to update the selected
        /// strokes information.
        /// </summary>
        /// <param name="sender">The control that raised the event.</param>
        /// <param name="e">The event arguments.</param>
        private void myInkCollector_Stroke(object sender, InkCollectorStrokeEventArgs e )
        {

            // This stroke corresponds to the lasso - 
            // cancel it so that it is not added into the ink
            e.Cancel = true;  
         
            Strokes hitStrokes = null;

            // If there are enough lasso points, perform a hit test
            // to determine which strokes were selected. 
            if (lassoPoints.Count > 2)
            {

                // Convert the lasso points from pixels to ink space
                Point[] inkLassoPoints = (Point[])lassoPoints.ToArray(typeof(Point));
                using (Graphics g = CreateGraphics())
                {
                    myInkCollector.Renderer.PixelToInkSpace(g, ref inkLassoPoints);
                }

                // Perform a hit test on this ink collector's ink to
                // determine which points were selected by the lasso stroke.
                //
                // Note that there is a slight inefficiency here since the
                // lasso stroke is part of the ink and, therefore, part of the
                // hit test - even though we don't need it.   It would have 
                // been more efficient to remove the stroke from the ink before 
                // calling HitTest.  However, it is not good practice to modify 
                // the stroke inside of its own event handler.
                hitStrokes = myInkCollector.Ink.HitTest(inkLassoPoints, LassoPercent);
                hitStrokes.Remove(e.Stroke);

            }

            // Reset the lasso points
            lassoPoints.Clear();
            lastDrawnLassoDot = Point.Empty;

            // Use helper method to set the selection
            SetSelection(hitStrokes);

        }

        // --------------- Helper Methods ---------------

        /// <summary>
        /// Helper method to convert a rectangle in pixel coordinates
        /// to ink space coordinates.  (1 ink space unit = .01mm)
        /// 
        /// Note that this method works since there is guaranteed not
        /// to be any rotation in this sample.
        /// </summary>
        /// <param name="g">The graphics display</param>
        /// <param name="r">The rectangle to convert</param>
        private void PixelToInkSpace(Graphics g, ref Rectangle r)
        {

            Point leftTop = new Point(r.X,r.Y);
            Point widthHeight = new Point (r.Width, r.Height);
            myInkCollector.Renderer.PixelToInkSpace(g, ref leftTop);
            myInkCollector.Renderer.PixelToInkSpace(g, ref widthHeight);

            r.Location = leftTop;
            r.Size = new Size(widthHeight);

        }

        /// <summary>
        /// Helper method to convert a rectangle in ink space
        /// coordinates to pixel coordinates.  (1 ink space unit = .01mm)
        /// 
        /// Note that this method works since there is guaranteed not
        /// to be any rotation in this sample.
        /// </summary>
        /// <param name="g">The graphics display</param>
        /// <param name="r">The rectangle to convert</param>
        private void InkSpaceToPixel(Graphics g, ref Rectangle r)
        {
            Point leftTop = new Point(r.X,r.Y);
            Point widthHeight = new Point (r.Width, r.Height);
            myInkCollector.Renderer.InkSpaceToPixel(g, ref leftTop);
            myInkCollector.Renderer.InkSpaceToPixel(g, ref widthHeight);

            r.Location = leftTop;
            r.Size = new Size(widthHeight);

        }

        /// <summary>
        /// This method uses the Ink API to copy ink onto
        /// the clipboard.
        /// </summary>
        /// <param name="clipboardModes">The clipboard modes (such as copy or paste)</param>
        private void CopyInkToClipboard(InkClipboardModes clipboardModes)
        {

            // Declare the ink clipboard formats to put on the clipboard
            InkClipboardFormats formats = new InkClipboardFormats();
            
            // Use selected format menu items to set the clipboard formats
            if (miISF.Checked)
            {
                formats |= InkClipboardFormats.InkSerializedFormat;
            }
            if (miMetafile.Checked)
            {
                formats |= InkClipboardFormats.Metafile;
            }
            if (miEMF.Checked)
            {
                formats |= InkClipboardFormats.EnhancedMetafile;
            }
            if (miBMP.Checked)
            {
                formats |= InkClipboardFormats.Bitmap;
            }
            if (miSInk.Checked)
            {
                formats |= InkClipboardFormats.SketchInk;
            }
            if (miTInk.Checked)
            {
                formats |= InkClipboardFormats.TextInk;
            }

            // If at least one format was selected, invoke the Ink
            // API's ClipboardCopy method.  Note that selectedStrokes
            // could be null, but that this is ok - if selectedStrokes
            // is null, all of the ink is copied.
            if (formats != InkClipboardFormats.None)
            {
                myInkCollector.Ink.ClipboardCopy(selectedStrokes,formats,clipboardModes);
            }
            else
            {
                MessageBox.Show("No clipboard formats selected");
            }
        }

        /// <summary>
        /// Sets the selected strokes and selection rectangle.
        /// </summary>
        /// <param name="strokes">The strokes that should be selected.  If null, the selection becomes empty</param>
        private void SetSelection(Strokes strokes)
        {
            // Tracks whether the rectangle that bounds the selected
            // strokes should be displayed
            bool isSelectionVisible = false;

            // Update the selected strokes collection
            selectedStrokes = strokes;

            // If no strokes are selected, set the selection rectangle
            // to empty
            if (!HasSelection())
            {
                selectionRect = Rectangle.Empty;
            }
                // Otherwise, at least one stroke is selected and it is necessary
                // to display the selection rectangle.
            else
            {
                isSelectionVisible = true;

                // Retrieve the bounding box of the strokes
                selectionRect = selectedStrokes.GetBoundingBox();
                using (Graphics g = CreateGraphics())
                {
                    InkSpaceToPixel(g, ref selectionRect);
                }

                // Pad the selection rectangle so that the selected ink 
                // doesn't overlap with the selection rectangle's handles.
                selectionRect.Inflate(SelectionRectBuffer, SelectionRectBuffer);

                // compute the center of the rectangle that bounds the 
                // selected strokes
                int xAvg = (selectionRect.Right+selectionRect.Left)/2;
                int yAvg = (selectionRect.Top+selectionRect.Bottom)/2;

                // Draw the resize handles
                // top left
                SetLocation(selectionHandles[0],selectionRect.Left, selectionRect.Top);
                // top
                SetLocation(selectionHandles[1],xAvg, selectionRect.Top);
                // top right 
                SetLocation(selectionHandles[2],selectionRect.Right, selectionRect.Top);

                // left 
                SetLocation(selectionHandles[3],selectionRect.Left, yAvg);
                // right
                SetLocation(selectionHandles[4],selectionRect.Right, yAvg);

                // bottom left
                SetLocation(selectionHandles[5],selectionRect.Left, selectionRect.Bottom);
                // bottom
                SetLocation(selectionHandles[6],xAvg, selectionRect.Bottom);
                // bottom right
                SetLocation(selectionHandles[7],selectionRect.Right, selectionRect.Bottom);
            }               

            // Set the visibility of each selection handle in the 
            // selection rectangle.  If there is no selection, all 
            // handles should be hidden.  Otherwise, all handles should
            // be visible.
            foreach(PictureBox pb in selectionHandles)
            {
                pb.Visible = isSelectionVisible;
            }

            // Turn off autoredrawing if there is a selection - otherwise,
            // the selected ink will not be displayed as selected.
            myInkCollector.AutoRedraw = !isSelectionVisible;

            // Since the selection has changed, repaint the screen.
            Refresh();
        }

        /// <summary>
        /// Helper method to set the location of the selection rectangle
        /// handle.
        /// </summary>
        /// <param name="pb">The handle's picture box</param>
        /// <param name="xCenter">The desired x coordinate of the center</param>
        /// <param name="yCenter">The desired y coordinate of the center</param>
        public void SetLocation(PictureBox pb, int xCenter, int yCenter)
        {
            Point location = new Point();

            location.X = xCenter - pb.Width/2;
            location.Y = yCenter - pb.Height/2;

            pb.Location = location;
        }

        /// <summary>
        /// Helper method to determine if there is a selection
        /// </summary>
        /// <returns>True if there is a selection</returns>
        public bool HasSelection()
        {
            return selectedStrokes!=null && selectedStrokes.Count > 0;
        }

        /// <summary>
        /// Draws the selection lasso.
        /// 
        /// Note that this method does NOT draw a point for every piece
        /// of packet data.  Instead, it draws evenly spaced dots that
        /// fall along the lasso path.
        /// </summary>
        /// <param name="g">The graphics device</param>
        /// <param name="newPacketIndex">The index where the new packets begin</param>
        public void DrawLasso(Graphics g, int newPacketIndex)
        {

            // The distance between the current point and the next point
            int     dx, dy;

            // The distance and angle of the line connecting the current 
            // point and the next point
            double  lineLength;
            double  angle;

            // The horizontal and vertical spacing between the dots
            double  segSpaceX, segSpaceY;

            // The total number of dots to draw
            int     totalSegments;

            // For each lasso point, draw the corresponding lasso dots.
            // Note that this method does NOT draw a point for every piece
            // of packet data.  Instead, it draws evenly spaced dots that
            // fall along the lasso path.
            for (int i = newPacketIndex; i < lassoPoints.Count; i++)
            {
                //  Always draw the first packet...
                if (0 == i)
                { 
                    // Update the last drawn lasso dot to this packet
                    lastDrawnLassoDot.X = ((Point)lassoPoints[i]).X;
                    lastDrawnLassoDot.Y = ((Point)lassoPoints[i]).Y;

                    // use a helper method to draw the lasso dot
                    DrawSelectionInkDot(g, lastDrawnLassoDot.X, lastDrawnLassoDot.Y);
                }
                else
                {
                    // Compute the x and y distance between this point and the last drawn
                    // lasso dot
                    dx = ((Point)lassoPoints[i]).X - lastDrawnLassoDot.X;
                    dy = ((Point)lassoPoints[i]).Y - lastDrawnLassoDot.Y;

                    // Compute length of the line between this point and the last drawn
                    // lasso dot
                    lineLength = Math.Sqrt(dx*dx + dy*dy);

                    // If the spacing between this packet and the last drawn lasso
                    // dot is greater than the desired DotSpacing, draw as many
                    // evently spaced lasso points as needed to fill the distance...
                    if (DotSpacing <= lineLength)
                    {

                        // Calculate the angle of the line connecting this point 
                        // to the last drawn lasso point. 
                        angle = Math.Atan2(dy, dx);

                        // Calculate the x and y components of the dot spacing
                        segSpaceX = DotSpacing * Math.Cos(angle);
                        segSpaceY = DotSpacing * Math.Sin(angle);

                        // Calculate the number of dots we will need to draw.
                        totalSegments = (int)(lineLength/ (double)DotSpacing);

                        // draw the lasso dots...
                        for (int j = 0; j < totalSegments; j++)
                        {
                            // The coordinates of the lasso dot are determined by the
                            // previous lasso dot plus the x and y components of the
                            // dot spacing (computed above).
                            lastDrawnLassoDot.X = (int)(lastDrawnLassoDot.X + segSpaceX);
                            lastDrawnLassoDot.Y = (int)(lastDrawnLassoDot.Y + segSpaceY);

                            // use a helper method to draw the lasso dot
                            DrawSelectionInkDot(g, lastDrawnLassoDot.X, lastDrawnLassoDot.Y);

                        }
                    }
                }
            }
        }

        /// <summary>
        /// Helper method to draw a lasso seletion dot
        /// </summary>
        /// <param name="g">The graphics device</param>
        /// <param name="x">The x coordinate of the center of the dot</param>
        /// <param name="y">The y coordinate of the center of the dot</param>
        private void DrawSelectionInkDot(Graphics g, int x, int y)
        {
            //
            // Offset the x and y by 1/2 the dot size (biasing lower-right)
            //
            x = x-(DotSize/2);
            y = y-(DotSize/2);

            // Draw the inside of the lasso dot
            g.FillEllipse(dotPen.Brush,x,y,DotSize,DotSize);

            // Draw the outside edge of the lasso dot
            g.DrawEllipse(dotEdgePen, x,y, DotSize, DotSize);

        }

        /// <summary>
        /// This method is used to render ink when there is
        /// a selection.
        /// </summary>
        /// <param name="g">The graphics device</param>
        private void DrawStrokes(Graphics g)
        {

            // First, draw the strokes that are not part of the 
            // selection.  These strokes will appear the same
            // as the InkCollector would normally draw them.
            Strokes unselectedStrokes = myInkCollector.Ink.Strokes;
            unselectedStrokes.Remove(selectedStrokes);
            myInkCollector.Renderer.Draw(g, unselectedStrokes);

            // Next, draw each selected stroke.  A selected stroke
            // is rendered in white with a selection border.
            foreach (Stroke selectedStroke in selectedStrokes)
            {
                DrawingAttributes da = selectedStroke.DrawingAttributes.Clone();
                da.Color = SystemColors.WindowText;
                da.Width = da.Width+SelectedInkWidthIncrease;
                myInkCollector.Renderer.Draw(g, selectedStroke, da);
                da.Color = Color.White;
                da.Width = da.Width-SelectedInkWidthIncrease;
                myInkCollector.Renderer.Draw(g, selectedStroke, da);
            }
        }

        /// <summary>
        /// Helper method to set the InkCollector and the
        /// selected stroke color.
        /// </summary>
        /// <param name="newColor">The new color</param>
        private void SetColor(Color newColor)
        {
            myInkCollector.DefaultDrawingAttributes.Color = newColor;

            // In addition to updating the ink collector, also update
            // the drawing attributes of all selected strokes.
            if (HasSelection())
            {
                foreach (Stroke s in selectedStrokes)
                {
                    s.DrawingAttributes.Color = newColor;
                }
            }

            Refresh();
        }

        /// <summary>
        /// Helper method to set the InkCollector and the
        /// selected stroke pen width.
        /// </summary>
        /// <param name="newWidth">The new pen width</param>
        private void SetWidth(float newWidth)
        {
            myInkCollector.DefaultDrawingAttributes.Width = newWidth;

            // In addition to updating the ink collector, also update
            // the drawing attributes of all selected strokes.
            if (HasSelection())
            {
                foreach (Stroke s in selectedStrokes)
                {
                    s.DrawingAttributes.Width = newWidth;
                }
            }

            Refresh();
        }

    }
}

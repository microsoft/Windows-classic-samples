// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//  File: InkCollection.cs
//  Simple Ink Collection Sample Application
//
//  This sample program demonstrates how to zoom and scroll
//  ink.  In particular, it allows the user to zoom in and out 
//  of ink in increments.  It also demonstrates how to zoom 
//  into a particular region using a zoom rectangle.  Finally,
//  this sample illustrates that ink can be collected at different
//  zoom levels and how to set up scrolling within the zoomed
//  drawing area.
//
//  In the code below, Renderer's view and object transforms are
//  used to perform zooming and scrolling.  The view transform  
//  applies to the points and the pen width.  The object transform 
//  applies only to the points.  The user can control which transform 
//  is used by changing the Mode->Scale Pen Width menu setting of 
//  this sample.
//
//  The features used are: InkCollector, Renderer.SetViewTransform(),
//  and Renderer.SetObjectTransform().
//
//--------------------------------------------------------------------------

using System;
using System.Drawing;
using System.Windows.Forms;
using System.Drawing.Drawing2D;

// The Ink namespace, which contains the Tablet PC Platform API
using Microsoft.Ink;

namespace Microsoft.Samples.TabletPC.InkZoom
{
    
    /// <summary>
    /// The InkZoom Sample Application form class
    /// </summary>
    public class InkZoom : System.Windows.Forms.Form
    {

        // Declare the Ink Collector object
        private InkCollector myInkCollector = null;

        // The pen used for drawing the zoom rectangle
        Pen blackPen = null;

        // The zoom rectangle
        private Rectangle zoomRectangle = Rectangle.Empty;

        // The current zoom factor (1 = 100% zoom level)
        private float zoomFactor = 1;

        // Declare constants for the width and height of the 
        // drawing area (in ink space coordinates).
        private const int InkSpaceWidth = 50000;
        private const int InkSpaceHeight = 50000;

        // Declare constant for the size of the desired small change
        private const int SmallChangeSize = 2;

        // Declare constant for the pen width used by this application
        private const float MediumInkWidth = 100;

        #region Standard Template Code
        private System.Windows.Forms.HScrollBar hScrollBar;
        private System.Windows.Forms.VScrollBar vScrollBar;
        private System.Windows.Forms.MenuItem miOther;
        private System.Windows.Forms.MenuItem miZoomToRect;
        private System.Windows.Forms.Panel pnlDrawingArea;
        private System.Windows.Forms.MenuItem miFile;
        private System.Windows.Forms.MenuItem miExit;
        private System.Windows.Forms.Label lblZoom;
        private System.Windows.Forms.NumericUpDown zoomUpDown;
        private System.Windows.Forms.Panel pnlZoom;
        private System.Windows.Forms.MenuItem miScalePenWidth;
        private System.Windows.Forms.MenuItem miInk;
        private System.Windows.Forms.MainMenu miMain;
        private System.Windows.Forms.MenuItem miActionSeparator;
        private System.Windows.Forms.MenuItem miModeSepararator;
        private System.Windows.Forms.MenuItem miClear;
        private System.Windows.Forms.ToolBar zoomToolBar;
        private System.ComponentModel.Container components = null;
        #endregion
        
        /// <summary>
        /// The InkZoom Sample Application form class
        /// </summary>
        public InkZoom()
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
            this.hScrollBar = new System.Windows.Forms.HScrollBar();
            this.vScrollBar = new System.Windows.Forms.VScrollBar();
            this.miMain = new System.Windows.Forms.MainMenu();
            this.miFile = new System.Windows.Forms.MenuItem();
            this.miClear = new System.Windows.Forms.MenuItem();
            this.miActionSeparator = new System.Windows.Forms.MenuItem();
            this.miExit = new System.Windows.Forms.MenuItem();
            this.miOther = new System.Windows.Forms.MenuItem();
            this.miInk = new System.Windows.Forms.MenuItem();
            this.miZoomToRect = new System.Windows.Forms.MenuItem();
            this.miModeSepararator = new System.Windows.Forms.MenuItem();
            this.miScalePenWidth = new System.Windows.Forms.MenuItem();
            this.pnlZoom = new System.Windows.Forms.Panel();
            this.lblZoom = new System.Windows.Forms.Label();
            this.zoomUpDown = new System.Windows.Forms.NumericUpDown();
            this.zoomToolBar = new System.Windows.Forms.ToolBar();
            this.pnlDrawingArea = new System.Windows.Forms.Panel();
            this.pnlZoom.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.zoomUpDown)).BeginInit();
            this.SuspendLayout();
            // 
            // hScrollBar
            // 
            this.hScrollBar.Anchor = ((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left) 
                | System.Windows.Forms.AnchorStyles.Right);
            this.hScrollBar.LargeChange = 100;
            this.hScrollBar.Location = new System.Drawing.Point(0, 279);
            this.hScrollBar.Maximum = 1000;
            this.hScrollBar.Name = "hScrollBar";
            this.hScrollBar.Size = new System.Drawing.Size(275, 17);
            this.hScrollBar.TabIndex = 0;
            this.hScrollBar.ValueChanged += new System.EventHandler(this.hScrollBar_ValueChanged);
            this.hScrollBar.Scroll += new System.Windows.Forms.ScrollEventHandler(this.hScrollBar_Scroll);
            // 
            // vScrollBar
            // 
            this.vScrollBar.Anchor = ((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
                | System.Windows.Forms.AnchorStyles.Right);
            this.vScrollBar.LargeChange = 100;
            this.vScrollBar.Location = new System.Drawing.Point(275, 26);
            this.vScrollBar.Maximum = 1000;
            this.vScrollBar.Name = "vScrollBar";
            this.vScrollBar.Size = new System.Drawing.Size(17, 254);
            this.vScrollBar.TabIndex = 1;
            this.vScrollBar.ValueChanged += new System.EventHandler(this.vScrollBar_ValueChanged);
            this.vScrollBar.Scroll += new System.Windows.Forms.ScrollEventHandler(this.vScrollBar_Scroll);
            // 
            // miMain
            // 
            this.miMain.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
                                                                                   this.miFile,
                                                                                   this.miOther});
            // 
            // miFile
            // 
            this.miFile.Index = 0;
            this.miFile.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
                                                                                   this.miClear,
                                                                                   this.miActionSeparator,
                                                                                   this.miExit});
            this.miFile.Text = "&Action";
            // 
            // miClear
            // 
            this.miClear.Index = 0;
            this.miClear.Text = "&Clear";
            this.miClear.Click += new System.EventHandler(this.miClear_Click);
            // 
            // miActionSeparator
            // 
            this.miActionSeparator.Index = 1;
            this.miActionSeparator.Text = "-";
            // 
            // miExit
            // 
            this.miExit.Index = 2;
            this.miExit.Text = "E&xit";
            this.miExit.Click += new System.EventHandler(this.miExit_Click);
            // 
            // miOther
            // 
            this.miOther.Index = 1;
            this.miOther.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
                                                                                    this.miInk,
                                                                                    this.miZoomToRect,
                                                                                    this.miModeSepararator,
                                                                                    this.miScalePenWidth});
            this.miOther.Text = "&Mode";
            // 
            // miInk
            // 
            this.miInk.Checked = true;
            this.miInk.Index = 0;
            this.miInk.RadioCheck = true;
            this.miInk.Text = "&Ink";
            this.miInk.Click += new System.EventHandler(this.miInk_Click);
            // 
            // miZoomToRect
            // 
            this.miZoomToRect.Index = 1;
            this.miZoomToRect.RadioCheck = true;
            this.miZoomToRect.Text = "&Zoom To Rect";
            this.miZoomToRect.Click += new System.EventHandler(this.miZoomToRect_Click);
            // 
            // miModeSepararator
            // 
            this.miModeSepararator.Index = 2;
            this.miModeSepararator.Text = "-";
            // 
            // miScalePenWidth
            // 
            this.miScalePenWidth.Checked = true;
            this.miScalePenWidth.Index = 3;
            this.miScalePenWidth.Text = "&Scale Pen Width";
            this.miScalePenWidth.Click += new System.EventHandler(this.miScalePenWidth_Click);
            // 
            // pnlZoom
            // 
            this.pnlZoom.Anchor = ((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
                | System.Windows.Forms.AnchorStyles.Right);
            this.pnlZoom.BackColor = System.Drawing.SystemColors.Control;
            this.pnlZoom.Controls.AddRange(new System.Windows.Forms.Control[] {
                                                                                  this.lblZoom,
                                                                                  this.zoomUpDown,
                                                                                  this.zoomToolBar});
            this.pnlZoom.ForeColor = System.Drawing.SystemColors.ControlText;
            this.pnlZoom.Name = "pnlZoom";
            this.pnlZoom.Size = new System.Drawing.Size(292, 26);
            this.pnlZoom.TabIndex = 3;
            // 
            // lblZoom
            // 
            this.lblZoom.Location = new System.Drawing.Point(8, 7);
            this.lblZoom.Name = "lblZoom";
            this.lblZoom.Size = new System.Drawing.Size(56, 16);
            this.lblZoom.TabIndex = 3;
            this.lblZoom.Text = "Zoom %:";
            // 
            // zoomUpDown
            // 
            this.zoomUpDown.Location = new System.Drawing.Point(64, 3);
            this.zoomUpDown.Maximum = new System.Decimal(new int[] {
                                                                       5000,
                                                                       0,
                                                                       0,
                                                                       0});
            this.zoomUpDown.Minimum = new System.Decimal(new int[] {
                                                                       10,
                                                                       0,
                                                                       0,
                                                                       0});
            this.zoomUpDown.Name = "zoomUpDown";
            this.zoomUpDown.Size = new System.Drawing.Size(56, 20);
            this.zoomUpDown.TabIndex = 2;
            this.zoomUpDown.Value = new System.Decimal(new int[] {
                                                                     100,
                                                                     0,
                                                                     0,
                                                                     0});
            this.zoomUpDown.ValueChanged += new System.EventHandler(this.xZoomUpDown_ValueChanged);
            // 
            // zoomToolBar
            // 
            this.zoomToolBar.DropDownArrows = true;
            this.zoomToolBar.Name = "zoomToolBar";
            this.zoomToolBar.ShowToolTips = true;
            this.zoomToolBar.Size = new System.Drawing.Size(292, 39);
            this.zoomToolBar.TabIndex = 4;
            // 
            // pnlDrawingArea
            // 
            this.pnlDrawingArea.Anchor = (((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
                | System.Windows.Forms.AnchorStyles.Left) 
                | System.Windows.Forms.AnchorStyles.Right);
            this.pnlDrawingArea.BackColor = System.Drawing.Color.White;
            this.pnlDrawingArea.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
            this.pnlDrawingArea.Location = new System.Drawing.Point(0, 26);
            this.pnlDrawingArea.Name = "pnlDrawingArea";
            this.pnlDrawingArea.Size = new System.Drawing.Size(275, 254);
            this.pnlDrawingArea.TabIndex = 4;
            this.pnlDrawingArea.MouseUp += new System.Windows.Forms.MouseEventHandler(this.pnlDrawingArea_MouseUp);
            this.pnlDrawingArea.MouseMove += new System.Windows.Forms.MouseEventHandler(this.pnlDrawingArea_MouseMove);
            this.pnlDrawingArea.MouseDown += new System.Windows.Forms.MouseEventHandler(this.pnlDrawingArea_MouseDown);
            // 
            // InkZoom
            // 
            this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
            this.ClientSize = new System.Drawing.Size(292, 296);
            this.Controls.AddRange(new System.Windows.Forms.Control[] {
                                                                          this.pnlDrawingArea,
                                                                          this.pnlZoom,
                                                                          this.vScrollBar,
                                                                          this.hScrollBar});
            this.Menu = this.miMain;
            this.Name = "InkZoom";
            this.Text = "InkZoom";
            this.Resize += new System.EventHandler(this.InkZoom_Resize);
            this.MouseDown += new System.Windows.Forms.MouseEventHandler(this.pnlDrawingArea_MouseDown);
            this.Load += new System.EventHandler(this.InkZoom_Load);
            this.MouseUp += new System.Windows.Forms.MouseEventHandler(this.pnlDrawingArea_MouseUp);
            this.MouseMove += new System.Windows.Forms.MouseEventHandler(this.pnlDrawingArea_MouseMove);
            this.pnlZoom.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.zoomUpDown)).EndInit();
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
            Application.Run(new InkZoom());
        }
        #endregion

        /// <summary>
        /// Event Handle from form Load Event
        /// </summary>
        /// <param name="sender">The control that raised the event.</param>
        /// <param name="e">The event arguments.</param>
        private void InkZoom_Load(object sender, System.EventArgs e)
        {
            // Create the pen used to draw the zoom rectangle
            blackPen = new Pen(Color.Black, 1);

            // Create the ink collector and associate it with the form
            myInkCollector = new InkCollector(pnlDrawingArea.Handle);

            // Set the pen width
            myInkCollector.DefaultDrawingAttributes.Width = MediumInkWidth;

            // Enable ink collection
            myInkCollector.Enabled = true;

            // Define ink space size - note that the scroll bars
            // map directly to ink space
            hScrollBar.Minimum = 0;
            hScrollBar.Maximum = InkSpaceWidth;
            vScrollBar.Minimum = 0;
            vScrollBar.Maximum = InkSpaceHeight;

            // Set the scroll bars to map to the current zoom level
            UpdateZoomAndScroll();        
        }

        /// <summary>
        /// Event Handle from xZoomUpDown Click Event
        /// </summary>
        /// <param name="sender">The control that raised the event.</param>
        /// <param name="e">The event arguments.</param>
        private void xZoomUpDown_ValueChanged(object sender, System.EventArgs e)
        {
            // Update the zoom factor to the new value
            zoomFactor = (float)zoomUpDown.Value/100;

            // Apply the new zoom/scroll settings
            UpdateZoomAndScroll();       
        }

        /// <summary>
        /// Event Handle from Horizontal Scroll Event
        /// </summary>
        /// <param name="sender">The control that raised the event.</param>
        /// <param name="e">The event arguments.</param>
        private void hScrollBar_Scroll(object sender, System.Windows.Forms.ScrollEventArgs e)
        {
            // Apply the new zoom/scroll settings
            UpdateZoomAndScroll();
        }

        /// <summary>
        /// Event Handle from Vertical Scroll Event
        /// </summary>
        /// <param name="sender">The control that raised the event.</param>
        /// <param name="e">The event arguments.</param>
        private void vScrollBar_Scroll(object sender, System.Windows.Forms.ScrollEventArgs e)
        {
            // Apply the new zoom/scroll settings
            UpdateZoomAndScroll(); 
        }

        /// <summary>
        /// Event Handle from Horizontal Scroll Value Changed Event
        /// </summary>
        /// <param name="sender">The control that raised the event.</param>
        /// <param name="e">The event arguments.</param>
        private void hScrollBar_ValueChanged(object sender, System.EventArgs e)
        {
            // Apply the new zoom/scroll settings
            UpdateZoomAndScroll();        
        }

        /// <summary>
        /// Event Handle form Vertical Scroll Value Changed Event
        /// </summary>
        /// <param name="sender">The control that raised the event.</param>
        /// <param name="e">The event arguments.</param>
        private void vScrollBar_ValueChanged(object sender, System.EventArgs e)
        {
            // Apply the new zoom/scroll settings
            UpdateZoomAndScroll();
        }

        /// <summary>
        /// Event Handle from ApplyToPenWidth Click Event
        /// </summary>
        /// <param name="sender">The control that raised the event.</param>
        /// <param name="e">The event arguments.</param>
        private void miScalePenWidth_Click(object sender, System.EventArgs e)
        {
            // Toggle the value indicating whether to apply the zoom to pen width
            // If on,  use the view transform.  Otherwise, use the object transform.
            miScalePenWidth.Checked = !miScalePenWidth.Checked;

            // Renderer's view and object transforms are used to perform
            // zooming and scrolling.  The view transform applies to the 
            // points and pen width.  The object transform  applies 
            // only to the points.  Since the sample only uses one of these 
            // transforms at a time, it is necessary to clear out the unused
            // transform and then invoke SetViewTransform to apply the
            // zoom/scroll settings on the view or object transform 
            // (depending if pen width scaling is desired).
            if (miScalePenWidth.Checked)
            {
                myInkCollector.Renderer.SetObjectTransform(null);
            }
            else
            {
                myInkCollector.Renderer.SetViewTransform(null);
            }

            // Apply the new zoom/scroll settings
            UpdateZoomAndScroll();
        }

        /// <summary>
        /// Event Handle from Ink Click Event
        /// </summary>
        /// <param name="sender">The control that raised the event.</param>
        /// <param name="e">The event arguments.</param>
        private void miInk_Click(object sender, System.EventArgs e)
        {
            // Update the Mode menu to be in ink mode
            miInk.Checked = true;
            miZoomToRect.Checked = false;

            // Update the cursor
            // Note that it is necessary to use the full name to avoid a
            // namespace conflict with InkCollector's Cursors property.
            pnlDrawingArea.Cursor = System.Windows.Forms.Cursors.Default;

            // Enable the ink collector
            myInkCollector.Enabled = true;        
        }

        /// <summary>
        /// Event Handle from ZoomToRect Click Event
        /// </summary>
        /// <param name="sender">The control that raised the event.</param>
        /// <param name="e">The event arguments.</param>
        private void miZoomToRect_Click(object sender, System.EventArgs e)
        {
            if (!myInkCollector.CollectingInk)
            {
                // Update the Mode menu to be in zoom to rect mode
                miInk.Checked = false;
                miZoomToRect.Checked = true;

                // Update the cursor
                pnlDrawingArea.Cursor = System.Windows.Forms.Cursors.Cross;

                // Disable the ink collector
                myInkCollector.Enabled = false;
            }
            else 
            {
                // If user is actively inking, we cannot switch modes.
                MessageBox.Show("Cannot Zoom while collecting ink.");
            }
        }

        /// <summary>
        /// Event Handle from Clear Click Event
        /// </summary>
        /// <param name="sender">The control that raised the event.</param>
        /// <param name="e">The event arguments.</param>
        private void miClear_Click(object sender, System.EventArgs e)
        {
            // Set the zoom level to 100%
            zoomFactor = 1;

            // Set the scroll bars to the top left of the screen
            hScrollBar.Value = 0;
            vScrollBar.Value = 0;

            zoomUpDown.Value = 100;

            // remove all strokes from the ink collector
            myInkCollector.Ink.DeleteStrokes();

            // Apply the new zoom/scroll settings
            UpdateZoomAndScroll();

            // Reset the application mode to inking
            miInk.Checked = true;
            miZoomToRect.Checked = false;
            Cursor = System.Windows.Forms.Cursors.Default;
            myInkCollector.Enabled = true;
        }

        /// <summary>
        /// Event Handle from File->Exit Click Event
        /// </summary>
        /// <param name="sender">The control that raised the event.</param>
        /// <param name="e">The event arguments.</param>
        private void miExit_Click(object sender, System.EventArgs e)
        {
            myInkCollector.Enabled = false;
            this.Dispose();
        }

        /// <summary>
        /// Event Handle from form's Resize Event
        /// </summary>
        /// <param name="sender">The control that raised the event.</param>
        /// <param name="e">The event arguments.</param>
        private void InkZoom_Resize(object sender, System.EventArgs e)
        {
            // If the user resizes the window, update the scroll
            // bars to reflect the change.  However, it is necessary
            // to check whether the ink collector is null before 
            // invoking this method since the Resize event also occurs
            // automatically before the ink collector is created.
            if (myInkCollector!=null)
            {
                UpdateScrollBars();
            }
        }

        /// <summary>
        /// Event Handle from MouseDown Event
        /// </summary>
        /// <param name="sender">The control that raised the event.</param>
        /// <param name="e">The event arguments.</param>
        private void pnlDrawingArea_MouseDown(object sender, System.Windows.Forms.MouseEventArgs e)
        {
            // If the user has selected to draw a zoom rectangle, update
            // the zoom rectangle's start point to the mouse location.
            if (miZoomToRect.Checked)
            {           
                zoomRectangle.Location = new Point(e.X,e.Y);
                zoomRectangle.Width = 0;
                zoomRectangle.Height = 0;
            }
        }

        /// <summary>
        /// Event Handle from MouseMove event
        /// </summary>
        /// <param name="sender">The control that raised the event.</param>
        /// <param name="e">The event arguments.</param>
        private void pnlDrawingArea_MouseMove(object sender, System.Windows.Forms.MouseEventArgs e)
        {
            // If the user has selected to draw a zoom rectangle and has a mouse
            // button pressed down, update the zoom rectangle end point to the 
            // new mouse location.
            if ( (miZoomToRect.Checked) &&  (MouseButtons != MouseButtons.None) )
            {
                // Erase the previous zoom rectangle
                Rectangle screenRect = zoomRectangle;
                screenRect.Offset(pnlDrawingArea.PointToScreen(Point.Empty));
                ControlPaint.DrawReversibleFrame(screenRect,Color.Transparent,FrameStyle.Dashed);

                int xSize = e.X - zoomRectangle.Location.X;
                int ySize = e.Y - zoomRectangle.Location.Y;

                int zoomRectSize = Math.Max(Math.Abs(xSize),Math.Abs(ySize));

                zoomRectangle.Width = (xSize>0)?zoomRectSize:-zoomRectSize;
                zoomRectangle.Height = (ySize>0)?zoomRectSize:-zoomRectSize;

                // Draw the current zoom rectangle
                screenRect = zoomRectangle;
                screenRect.Offset(pnlDrawingArea.PointToScreen(Point.Empty));
                ControlPaint.DrawReversibleFrame(screenRect,Color.Transparent,FrameStyle.Dashed);
            }
        }

        /// <summary>
        /// Event Handle from MouseUp Event
        /// </summary>
        /// <param name="sender">The control that raised the event.</param>
        /// <param name="e">The event arguments.</param>
        private void pnlDrawingArea_MouseUp(object sender, System.Windows.Forms.MouseEventArgs e)
        {
            // If the user has selected to draw a zoom rectangle...
            if (miZoomToRect.Checked)
            {
                // Erase the zoom rectangle
                Rectangle screenRect = zoomRectangle;
                screenRect.Offset(pnlDrawingArea.PointToScreen(Point.Empty));
                ControlPaint.DrawReversibleFrame(screenRect,Color.Transparent,FrameStyle.Dashed);

                // Perform the zoom
                ZoomToRectangle();
            } 
        }

        /// <summary>
        /// Helper method to update the ink collector with the current
        /// zoom and scroll values
        /// </summary>
        private void UpdateZoomAndScroll()
        {
            // Create a new transformation matrix
            Matrix m = new Matrix();

            // Apply the current scale factor
            m.Scale(zoomFactor,zoomFactor);

            // Apply the current translation factor - note that since 
            // the scroll bars map directly to ink space, their values
            // can be used directly.
            m.Translate(-hScrollBar.Value, -vScrollBar.Value);

            // Renderer's view and object transforms are used to perform
            // zooming and scrolling.  The view transform applies to the 
            // points and pen width.  The object transform  applies 
            // only to the points.  Depending on whether the user has 
            // selected to apply the transform to pen width, update the 
            // view or object transform.
            if (miScalePenWidth.Checked)
            {
                myInkCollector.Renderer.SetViewTransform(m);
            }
            else
            {
                myInkCollector.Renderer.SetObjectTransform(m);
            }

            // Set the scroll bars to map to the current zoom level
            UpdateScrollBars();

            Refresh();
        }

        /// <summary>
        /// Helper method to update the scroll bars small and large change
        /// values to the new zoom level.  If the scroll bars are no longer
        /// needed, this method will also hide them.
        /// </summary>
        void UpdateScrollBars() 
        {
            if ( (hScrollBar.Width>0) && (vScrollBar.Height>0))
            {
                // The following code sets the small and large change
                // values of the scroll bars.  These values specify
                // desired scroll size when the scroll box is moved a 
                // small/large distance.  In this sample, the large
                // change scrolls by the width of the visible drawing
                // area and the small change scrolls by the constant
                // SmallChangeSize.  Note that it is necessary to account 
                // for the zoom factor when calculating the large and
                // small change values.  Also, since the scroll bar
                // uses ink space units, it is necessary to calculate
                // the small and large change values in ink space.
                //
                // In summary:
                // 1.  Calculate the large change value as the width
                //     of the visible drawing area in ink space units.
                // 2.  Calculate the small change value as the 
                //     SmallChangeSize in ink space units.

                
                // Create a point representing the top left of the drawing area (in pixels)
                Point ptUpperLeft = new Point(0, 0);
                
                // Create a point representing the size of a small change
                Point ptSmallChange = new Point(SmallChangeSize, SmallChangeSize);

                // Create a point representing the bottom right of the drawing area (in pixels)
                Point ptLowerRight = new Point(hScrollBar.Width, vScrollBar.Height);

                using (Graphics g = CreateGraphics())
                {
                    // Convert each of the points to ink space
                    myInkCollector.Renderer.PixelToInkSpace(g, ref ptUpperLeft);
                    myInkCollector.Renderer.PixelToInkSpace(g, ref ptLowerRight);
                    myInkCollector.Renderer.PixelToInkSpace(g, ref ptSmallChange);
                }

                // Set the SmallChange values (in ink space)
                // Note that it is necessary to subract the upper left point
                // value to account for scrolling.
                hScrollBar.SmallChange = ptSmallChange.X - ptUpperLeft.X;
                vScrollBar.SmallChange = ptSmallChange.Y - ptUpperLeft.Y;

                // Set the LargeChange values to the drawing area width (in ink space)
                // Note that it is necessary to subract the upper left point
                // value to account for scrolling.
                hScrollBar.LargeChange = ptLowerRight.X - ptUpperLeft.X;
                vScrollBar.LargeChange = ptLowerRight.Y - ptUpperLeft.Y;

                // If the scroll bars are not needed, hide them
                hScrollBar.Visible = hScrollBar.LargeChange < hScrollBar.Maximum;
                vScrollBar.Visible = vScrollBar.LargeChange < vScrollBar.Maximum;

                // If the horizontal scroll bar value would run off of the drawing area, 
                // adjust it
                if(hScrollBar.Visible && (hScrollBar.Value + hScrollBar.LargeChange > hScrollBar.Maximum)) 
                {
                    hScrollBar.Value = hScrollBar.Maximum - hScrollBar.LargeChange;
                }

                // If the vertical scroll bar value would run off of the drawing area, 
                // adjust it
                if(vScrollBar.Visible && (vScrollBar.Value + vScrollBar.LargeChange > vScrollBar.Maximum))
                {
                    vScrollBar.Value = vScrollBar.Maximum - vScrollBar.LargeChange;
                }
            }
        }

        /// <summary>
        /// Helper method to zoom into a specified rectangle.
        /// </summary>
        public void ZoomToRectangle()
        {

            // Calculate the width/height of the zoom rectangle
            int newWidth = Math.Abs(zoomRectangle.Width);
            int newHeight = Math.Abs(zoomRectangle.Height);

            // Check to ensure that the zoom rectangle isn't just a point...
            if (newWidth>0 && newHeight>0)
            {

                // To determine the new scale factor, take the ratio of the 
                // scaled drawing area to the size of the zoom rectangle.
                // Since the ratio of the width could be different than
                // the ratio of the height, the minimum scale value is used.
                float newZoomFactor = Math.Min((zoomFactor*pnlDrawingArea.Width)/newWidth,
                                          (zoomFactor*pnlDrawingArea.Height)/newHeight);

                // Check whether the zoom rectangle would exceed the allowable
                // zoom level.  If so, display an error.
                if ((int)(newZoomFactor*100) < zoomUpDown.Minimum)
                {
                    MessageBox.Show("Minimum zoom level reached.");
                }
                else if ((int)(newZoomFactor*100) > zoomUpDown.Maximum)
                {
                    MessageBox.Show("Maximum zoom level reached.");
                }
                else
                {

                    // Set the scroll bar values to the zoom rectangle's top left corner
                    // Note that it is necessary to convert to ink space since the scroll
                    // bars use ink space coordinates.
                    Point scrollValue = new Point();
                    scrollValue.X = Math.Min(zoomRectangle.Left,zoomRectangle.Left + zoomRectangle.Width);
                    scrollValue.Y = Math.Min(zoomRectangle.Top,zoomRectangle.Top + zoomRectangle.Height);
                    using (Graphics g = CreateGraphics())
                    {
                        myInkCollector.Renderer.PixelToInkSpace(g,ref scrollValue);
                    }

                    // If the new scroll bar values would fall outside of the 
                    // allowable range, adjust them to the nearest allowable
                    // value.
                    if (scrollValue.X > hScrollBar.Maximum)
                    {
                        scrollValue.X = hScrollBar.Maximum;
                    }
                    else if (scrollValue.X < hScrollBar.Minimum)
                    {
                        scrollValue.X = hScrollBar.Minimum;
                    }

                    if (scrollValue.Y > vScrollBar.Maximum)
                    {
                        scrollValue.Y = vScrollBar.Maximum;
                    }
                    else if (scrollValue.Y < vScrollBar.Minimum)
                    {
                        scrollValue.Y = vScrollBar.Minimum;
                    }


                    // Update the new scale factor to the ratio of the scroll bar
                    // to the zoom rectangle's width
                    zoomFactor = newZoomFactor;
                    zoomUpDown.Value = (decimal)zoomFactor*100;

                    // Apply the new zoom/scroll settings
                    UpdateZoomAndScroll();

                    // Update the scroll bar values.
                    hScrollBar.Value = scrollValue.X;
                    vScrollBar.Value = scrollValue.Y;

                }
            }

            // Reset the zoom rectangle
            zoomRectangle = Rectangle.Empty;

            Refresh();
        }
    }
}

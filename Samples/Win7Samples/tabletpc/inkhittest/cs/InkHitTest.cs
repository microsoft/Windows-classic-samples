// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//  File: InkHitTest.cs
//  Ink Hit Test Sample Application
//
//  This sample illustrates two techniques for finding ink given a 
//  screen location:
//
//      1. Hit Testing:  The hit testing mode determines whether
//         a collection of strokes is either completely inside or 
//         intersected by a circular cursor.  When a hit occurs, the 
//         circular cursor changes from black to red.
//
//      2. Nearest Point:  The nearest point mode calculates the
//         the point on a stroke within an Ink object that is 
//         nearest to the cursor's location.  The sample displays
//         a red line connecting the cursor to this point.
//
//  The features used are: InkCollector, Ink.HitTest,Ink.NearestPoint.
//
//--------------------------------------------------------------------------
    
using System;
using System.Drawing;
using System.Windows.Forms;

// The Ink namespace, which contains the Tablet PC Platform API
using Microsoft.Ink;

namespace Microsoft.Samples.TabletPC.InkHitTest
{

    /// <summary>
    /// Enumeration of all possible application modes:
    /// 
    ///   Ink:             The user is drawing new strokes
    ///   HitTest:         The user is performing hit testing
    ///   NearestPoint:    The app should calculate the nearest point
    ///   
    /// </summary>
    public enum ApplicationMode 
            {
                None = 0,
                Ink = 1,
                HitTest = 2,
                NearestPoint = 3
    };

    /// <summary>
    /// The InkHitTest Sample Application form class
    /// </summary>
    public class InkHitTest : System.Windows.Forms.Form
    {

        // --------------- Constants ---------------

        // The radius of the circle used for hit testing
        // in pixels.
        private const int HitSize = 30;

        // --------------- Fields ---------------

        // The inkcollector is the central object for ink support.
        // Through this object, we will collect ink and then get access
        // to it via the Ink object.
        private InkCollector ic = null;
        
        // The pen that is currently used for drawing
        private Pen     activePen;

        // Black and red pens used for drawing
        private Pen     blackPen;
        private Pen     redPen;

        // Stores the current location of the pen
        private Point   penPt = Point.Empty;

        // Stores the current nearest point (when applicable)
        private Point   nearestPt = Point.Empty;

        // Stores the region of the screen that needs to be invalidated
        private Rectangle invalidateRect;

        private ApplicationMode mode;

        #region Standard Template Code
        private System.Windows.Forms.MainMenu mainMenu1;
        private System.Windows.Forms.MenuItem FileItem;
        private System.Windows.Forms.MenuItem ModeItem;
        private System.Windows.Forms.MenuItem ClearItem;
        private System.Windows.Forms.MenuItem InkItem;
        private System.Windows.Forms.MenuItem HitTestItem;
        private System.Windows.Forms.MenuItem NearestPointItem;
        private System.Windows.Forms.MenuItem miExit;

        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.Container components = null;
        #endregion

        public InkHitTest()
        {
            #region Standard Template Code
            //
            // Required for Windows Form Designer support
            //
            InitializeComponent();
            #endregion
        }

        # region Standard Template Code
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
                if (ic != null)
                {
                    ic.Dispose();
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
            this.mainMenu1 = new System.Windows.Forms.MainMenu();
            this.FileItem = new System.Windows.Forms.MenuItem();
            this.miExit = new System.Windows.Forms.MenuItem();
            this.ModeItem = new System.Windows.Forms.MenuItem();
            this.InkItem = new System.Windows.Forms.MenuItem();
            this.HitTestItem = new System.Windows.Forms.MenuItem();
            this.NearestPointItem = new System.Windows.Forms.MenuItem();
            this.ClearItem = new System.Windows.Forms.MenuItem();
            // 
            // mainMenu1
            // 
            this.mainMenu1.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
                                                                                      this.FileItem,
                                                                                      this.ModeItem,
                                                                                      this.ClearItem});
            // 
            // FileItem
            // 
            this.FileItem.Index = 0;
            this.FileItem.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
                                                                                     this.miExit});
            this.FileItem.Text = "&File";

            // 
            // miExit
            // 
            this.miExit.Index = 0;
            this.miExit.Text = "E&xit";
            this.miExit.Click += new System.EventHandler(this.miExit_Click);
            // 
            // ModeItem
            // 
            this.ModeItem.Index = 1;
            this.ModeItem.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
                                                                                     this.InkItem,
                                                                                     this.HitTestItem,
                                                                                     this.NearestPointItem});
            this.ModeItem.Text = "&Mode";
            // 
            // InkItem
            // 
            this.InkItem.Checked = true;
            this.InkItem.Index = 0;
            this.InkItem.Text = "&Ink";
            this.InkItem.Click += new System.EventHandler(this.enterInkMode);
            // 
            // HitTestItem
            // 
            this.HitTestItem.Index = 1;
            this.HitTestItem.Text = "&Hit Test";
            this.HitTestItem.Click += new System.EventHandler(this.enterHitTestMode);
            // 
            // NearestPointItem
            // 
            this.NearestPointItem.Index = 2;
            this.NearestPointItem.Text = "&Nearest Point";
            this.NearestPointItem.Click += new System.EventHandler(this.enterNearestPointMode);
            // 
            // ClearItem
            // 
            this.ClearItem.Index = 2;
            this.ClearItem.Text = "&Clear!";
            this.ClearItem.Click += new System.EventHandler(this.onClearClick);
            // 
            // InkHitTest
            // 
            this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
            this.ClientSize = new System.Drawing.Size(292, 245);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedSingle;
            this.MaximizeBox = false;
            this.Menu = this.mainMenu1;
            this.Name = "InkHitTest";
            this.Text = "Tablet PC InkHitTest Sample";
            this.Load += new System.EventHandler(this.InkHitTest_Load);
            this.Paint += new System.Windows.Forms.PaintEventHandler(this.InkHitTestForm_Paint);
            this.MouseMove += new System.Windows.Forms.MouseEventHandler(this.onMouseMove);

        }
        #endregion

        #region Standard Template Code
        /// <summary>
        /// The main entry point for the application.
        /// </summary>
        [STAThread]
        static void Main() 
        {
            Application.Run(new InkHitTest());
        }
        #endregion

        // --------------- Form Events ---------------

        /// <summary>
        /// Event Handler from Form Load Event
        /// Setup the ink collector for collection
        /// </summary>
        /// <param name="sender">The control that raised the event.</param>
        /// <param name="e">The event arguments.</param>
        private void InkHitTest_Load(object sender, System.EventArgs e)
        {
            //
            // Setup some useful objects for drawing
            //
            activePen = blackPen = new Pen(Color.Black, 3);
            redPen = new Pen(Color.Red, 3);
            invalidateRect = new Rectangle(0,0,0,0);

            //
            // Create the InkCollector, and turn it on
            //
            ic = new InkCollector(Handle);  // attach it to the form's frame window

            // default to inking mode
            mode = ApplicationMode.Ink;

            // turn the collector on
            ic.Enabled = true;

        }

        /// <summary>
        /// Event Handler from Form Mouse Move Event
        /// Handle Mouse/Pen movement. If we are not in ink mode, then we will
        /// be doing either nearest point or hit test computations.
        /// </summary>
        /// <param name="sender">The control that raised the event.</param>
        /// <param name="e">The event arguments.</param>
        private void onMouseMove(object sender, System.Windows.Forms.MouseEventArgs e)
        {
            // Update the location of the pen
            penPt.X = e.X;
            penPt.Y = e.Y;

            if ( mode == ApplicationMode.HitTest)
            {
                handleHitTest(e);
            }
            else if (mode == ApplicationMode.NearestPoint)
            {
                handleNearestPoint(e);
            }
        }

        /// <summary>
        /// Event Handler from Form Paint Event
        /// 
        /// This sample has a very straightforward repaint algorithm.  We "know" at
        /// all times how to paint the control.  Since AutoRedraw is on by default, 
        /// the ink will take care of painting itself.
        /// 
        /// That leaves just repaints of the circle hit test cursor or the nearest point
        /// line.  To simplify redraw, we just remember a "bounding box" for the area that
        /// we paint in the invalidateRect member variable.  We will invalidate this region
        /// each time something new happens.
        /// 
        /// </summary>
        /// <param name="sender">The control that raised the event.</param>
        /// <param name="e">The event arguments.</param>
        private void InkHitTestForm_Paint(object sender, System.Windows.Forms.PaintEventArgs e)
        {
            if( mode == ApplicationMode.HitTest)
            {           
                e.Graphics.DrawEllipse(activePen, penPt.X - HitSize/2, penPt.Y - HitSize/2, HitSize, HitSize);
            }
            else if( mode == ApplicationMode.NearestPoint )
            {
                e.Graphics.DrawLine(redPen, penPt, nearestPt);
            }       
        }

        /// <summary>
        /// Event Handler from Form Paint Event
        /// Clears all of the ink in the form
        /// </summary>
        /// <param name="sender">The control that raised the event.</param>
        /// <param name="e">The event arguments.</param>
        private void onClearClick(object sender, System.EventArgs e)
        {
            // Retrieve all strokes in the ink object
            Strokes strokesToDelete = ic.Ink.Strokes;

            // Check to ensure that the ink collector isn't currently
            // in the middle of a stroke before clearing the ink.
            // Deleting a stroke that is currently being collected
            // will result in an error condition.
            if (!ic.CollectingInk)
            {

                // Delete the strokes from the ink object
                ic.Ink.DeleteStrokes(strokesToDelete);

                // Put the application in ink mode
                enterInkMode(sender, e);

                //go ahead and redraw the form, now that it has been cleared.
                Refresh();
            }
            else
            {
                MessageBox.Show("Cannot clear ink while the ink collector is busy");
            }
        }

        /// <summary>
        /// Event Handler from Mode->Ink Menu Item
        /// Helper function to enter Ink mode.
        /// In this mode, the user can draw ink.
        /// </summary>
        /// <param name="sender">The control that raised the event.</param>
        /// <param name="e">The event arguments.</param>
        private void enterInkMode(object sender, System.EventArgs e)
        {

            clearCheck();
            mode = ApplicationMode.Ink;
            InkItem.Checked = true;
            ic.Enabled = true;
        
            Invalidate(invalidateRect);
        }

        /// <summary>
        /// Event Handler from Mode->Hit Test Menu Item
        /// Helper function to enter HitTest mode.
        /// In this mode, we highlight the cursor when it is over 
        /// an ink stroke.
        /// </summary>
        /// <param name="sender">The control that raised the event.</param>
        /// <param name="e">The event arguments.</param>
        private void enterHitTestMode(object sender, System.EventArgs e)
        {
            if (!ic.CollectingInk)
            {
                Cursor = System.Windows.Forms.Cursors.Cross;

                // Turn off the ink collector and set flags enable the hit
                // test code.
                clearCheck();
                mode = ApplicationMode.HitTest;
                HitTestItem.Checked = true;
                ic.Enabled = false;

                Invalidate(invalidateRect);
            }
            else 
            {
                // If user is actively inking, we cannot disable the collector.
                MessageBox.Show("Cannot switch to HitTest mode while collecting ink.");
            }
        }

        /// <summary>
        /// Event Handler from Mode->Nearest Point Menu Item
        /// Helper function to enter NearestPoint mode.
        /// In this mode, we draw a line to the nearest point in the ink 
        /// from the current location of the cursor.
        /// </summary>
        /// <param name="sender">The control that raised the event.</param>
        /// <param name="e">The event arguments.</param>
        private void enterNearestPointMode(object sender, System.EventArgs e)
        {
            if (!ic.CollectingInk)
            {
                Cursor = System.Windows.Forms.Cursors.Default;

                // Turn off the ink collector and set flags to enable
                // the nearest point code
                clearCheck();
                mode = ApplicationMode.NearestPoint;
                NearestPointItem.Checked = true;
                ic.Enabled = false;

                Invalidate(invalidateRect);
            }
            else 
            {
                    // If user is actively inking, we cannot disable the collector.
                MessageBox.Show("Cannot switch to Nearest Point mode while collecting ink.");
            }
        }

        /// <summary>
        /// Event Handler from File->Exit Menu Item
        /// </summary>
        /// <param name="sender">The control that raised the event.</param>
        /// <param name="e">The event arguments.</param>
        private void miExit_Click(object sender, System.EventArgs e)
        {
            // Although not required, it is good practice to disable
            // ink collection before exiting
            ic.Enabled = false;
            Application.Exit();       
        }

        // --------------- Helper Methods ---------------

        /// <summary>
        ///  small helper method to clear the Mode menu
        ///  check marks
        /// </summary>
        private void clearCheck()
        {
            switch(mode)
            {
                case ApplicationMode.Ink:
                    InkItem.Checked = false;
                    break;
                case ApplicationMode.HitTest:
                    HitTestItem.Checked = false;
                    break;
                case ApplicationMode.NearestPoint:
                    NearestPointItem.Checked = false;
                    break;
            }

            mode = ApplicationMode.None;
        }

        /// <summary>
        ///  Helper method implementing the hit test functionality
        /// </summary>
        /// <param name="e">The mouse event args from MouseMove</param>
        private void handleHitTest(System.Windows.Forms.MouseEventArgs e)
        {
            // remember two points:
            // 1.  the location of the pen 
            // 2.  the location that is HitSize/2 pixels across from the pen
            //     location.  This will be the radius of our hit test area.
            Point pt1 = new Point(e.X, e.Y);
            Point pt2 = new Point(e.X + HitSize/2, e.Y);

            // Convert these points into ink space coordinates.  Recall that ink is 
            // kept in a high dpi "ink space" by default.  Thus, we need to map
            // screen pixel locations to their corresponding locations in ink
            // space.  The distance between the X coordinates will be used to compute
            // the ink space radius of the hit test circle.  We can make this simplification
            // as long as the renderer has a scalar transformation (it wouldn't work if a 
            // shear or rotation transformation had been applied to the renderer). 
            //
            // For simplicity, we are assuming "square" pixels in this example.  As a result,
            // the hit test 'circle' will really be an ellispe on most monitors.  Applications
            // wishing to draw true circles, etc. will want to take the screen aspect ratio
            // into account for their pixel computations.
            using (Graphics g = CreateGraphics())
            {
                ic.Renderer.PixelToInkSpace(g, ref pt1);
                ic.Renderer.PixelToInkSpace(g, ref pt2);
            }

            // Retrieve the strokes (if any) that are either completely inside or 
            // intersected by the circular region around the pen.
            //
            // Note that this method takes the stroke's drawing attributes into account
            // when it computes the intersection, including the pen width, Bezier smoothing, 
            // and shape of the pen tip.
            Strokes strokes = ic.Ink.HitTest(pt1, (float)(pt2.X - pt1.X));              

            if( strokes.Count > 0 )
            {
                activePen = redPen;
            }
            else
            {
                activePen = blackPen; 
            }           
            
            // Clear out the previous drawing and trigger a draw
            Invalidate(invalidateRect);

            invalidateRect.X = e.X - HitSize/2;
            invalidateRect.Y = e.Y - HitSize/2;
            invalidateRect.Width = invalidateRect.Height = HitSize; 

            // Since the hit test circle has thickness, it is necessary
            // to inflate the invalidation area to account for it (the 
            // invalidation area is the hit test area + the pen width).
            invalidateRect.Inflate((int)activePen.Width, (int)activePen.Width);
            
            Invalidate(invalidateRect);
        }

        /// <summary>
        ///  Helper function implementing the nearest point functionality
        /// </summary>
        /// <param name="e">The mouse event args from MouseMove</param>
        private void handleNearestPoint(System.Windows.Forms.MouseEventArgs e)
        {
            using (Graphics g = CreateGraphics())
            {

                // Remember pen location
                Point inkPenPt = new Point(e.X, e.Y);

                // Convert the pen location into a location in ink space
                ic.Renderer.PixelToInkSpace(g, ref inkPenPt);

                // Get the nearest point.  NearestPoint will return the
                // stroke *and* the index of the actual location within
                // the stroke.  
                // 
                // The location is represented as a floating point value.
                // 3.3 for example, would indicate that the nearest point
                // is 30% along the vector between point index 3 and point
                // index 4 in the stroke.

                float fIndex;
                Stroke stroke = ic.Ink.NearestPoint(inkPenPt, out fIndex);

                // Provided that the stroke isn't null, use the fIndex to
                // compute the ink space coordinates of the nearest point.
                if( stroke != null )
                {

                    // If the findex lies directly over one of the stroke's 
                    // points, retrieve the coordinates of this point.
                    if ((int)fIndex == fIndex)
                    {
                        Point pt = stroke.GetPoint((int)fIndex);

                        nearestPt = new Point(pt.X, pt.Y);
                    }
                        // Otherwise, it is necessary to approximate the coordinates
                        // of the stroke using the points on the stroke that bound 
                        // the nearest point findex
                    else
                    {

                        // Retrieve the two points on the stroke on either side of 
                        // the nearest point findex.
                        Point[] pts = stroke.GetPoints((int)fIndex, 2);

                        // Since we already handled the case where the findex was directly
                        // over one of the stroke's points, the call above should always
                        // return two points.
                        if( pts.Length == 2 )
                        {

                            // To compute the coordinates of the nearest point from 
                            // the findex:  take the difference between the two points,
                            // scale it by the percentage offset, and then add the
                            // percentage offset back in.  This yeilds the following formula:
                            //
                            // X1 * p + X0 * q
                            // Y1 * p + Y0 * q
                            //
                            // Where:
                            // p == percentage offset from point 1
                            // q == percentage offset from point 2 (p-1)
                            //
                            float p = fIndex - (int)fIndex;
                            float q = 1F - p;
                            nearestPt = new Point((int)((float)(pts[1].X) * p + (float)(pts[0].X) * q),
                                (int)((float)(pts[1].Y) * p + (float)(pts[0].Y) * q ));

                        }
                    }
                }
                else
                {
                    // if there is no ink on the page, just reference ourselves.
                    nearestPt = inkPenPt;
                }

                // Now that we have the ink space coordinates of the nearest 
                // point, convert it back into pixel coordinates.  Again, we 
                // assume perfectly square pixels in this example for simplicity.
                ic.Renderer.InkSpaceToPixel(g, ref nearestPt);

                // Clear out the previous drawing and trigger a draw
                Invalidate(invalidateRect);

                invalidateRect.X = Math.Min(e.X, nearestPt.X);
                invalidateRect.Y = Math.Min(e.Y, nearestPt.Y);
                invalidateRect.Width = Math.Abs(e.X - nearestPt.X);
                invalidateRect.Height = Math.Abs(e.Y - nearestPt.Y);

                // Since the red nearest point line has thickness, it is 
                // necessary to inflate the invalidation area to account for it  
                // (the invalidation area is the rectangle around the nearest point
                // line + the pen width).
                invalidateRect.Inflate((int)redPen.Width, (int)redPen.Width);

                Invalidate(invalidateRect);

            }
        }
    }
}

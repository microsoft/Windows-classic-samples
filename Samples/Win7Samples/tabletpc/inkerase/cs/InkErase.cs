// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//  File: InkErase.cs
//  Simple Ink Erasing Sample Application
//
//  This sample program demonstrates how to erase ink using hit
//  testing.  It has the following modes:  
//
//  1.  Ink:  Allows the user to ink strokes.
//  2.  Erase at Cusps:  Displays the cusps as red points drawn over the
//      strokes.  The application reports a hit when the mouse is pressed
//      and a circular region around the cursor intersects the stroke.
//      When a hit occurs, the application splits the stroke at the
//      cusps on either side of the hit and erases the corresponding stroke 
//      segment.  
//  3.  Erase at Intersections:  Same as 2, except stroke intersections 
//      are used.
//  4.  Erase Strokes:  Uses hit testing to determine which strokes to delete.
//
//  This sample application supports the inverted pen - if the
//  pen is inverted in Ink mode, stroke erasing is performed.
//
//  The features used are:  InkCollector, Ink hit testing,
//  deleting and splitting strokes, finding cusps and intersections,
//  and using the inverted pen.
//         
//--------------------------------------------------------------------------
using System;
using System.Drawing;
using System.Windows.Forms;

// The Ink namespace, which contains the Tablet PC Platform API
using Microsoft.Ink;

namespace Microsoft.Samples.TabletPC.InkErase
{

    /// <summary>
    /// Enumeration of all possible application modes:
    /// 
    ///   Ink:             The user is drawing new strokes
    ///   CuspErase:       The user is erasing at cusps
    ///   IntersectErase:  The user is erasing at intersections
    ///   StrokeErase:     The user is erasing strokes
    ///   
    /// </summary>
    public enum ApplicationMode
    {
        Ink,
        CuspErase,
        IntersectErase,
        StrokeErase
    }

    /// <summary>
    /// Summary description for InkErase.
    /// </summary>
    public class InkErase : System.Windows.Forms.Form
    {
        // Declare the Ink Collector object
        private InkCollector myInkCollector = null;

        // Declare constant for the pen width used by this application.
        // Note that this constant is in high metric units (1 unit = .01mm)
        private const float MediumInkWidth = 100;

        // Declare constant for the size of the hit test circle radius.
        // Note that this constant is in high metric units (1 unit = .01mm)
        private const float HitTestRadius = 30;

        // Delcare constant for the radius of the painted cusp/intersection points
        private const int StrokePointRadius = 3;

        // Declare constant for the index of the x and y packet values
        private const int XPacketIndex = 0;
        private const int YPacketIndex = 1;

        // The current application mode:  inking, cusp erasing,
        // intersection erasing, or stroke erasing
        ApplicationMode mode = ApplicationMode.Ink;

        #region Standard Template Code

        private System.Windows.Forms.MenuItem miMainMode;
        private System.Windows.Forms.MenuItem miInk;
        private System.Windows.Forms.MenuItem miMainAction;
        private System.Windows.Forms.MenuItem miClear;
        private System.Windows.Forms.MenuItem miExit;
        private System.Windows.Forms.MenuItem miIntersectErase;
        private System.Windows.Forms.MenuItem miStrokeErase;
        private System.Windows.Forms.MainMenu miMain;
        private System.Windows.Forms.MenuItem miCuspErase;
        private System.Windows.Forms.MenuItem miSeparator;
        private System.ComponentModel.Container components = null;
        #endregion

        /// <summary>
        /// The InkErase Sample Application form class
        /// </summary>
        public InkErase()
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
            this.miMain = new System.Windows.Forms.MainMenu();
            this.miMainAction = new System.Windows.Forms.MenuItem();
            this.miClear = new System.Windows.Forms.MenuItem();
            this.miSeparator = new System.Windows.Forms.MenuItem();
            this.miExit = new System.Windows.Forms.MenuItem();
            this.miMainMode = new System.Windows.Forms.MenuItem();
            this.miInk = new System.Windows.Forms.MenuItem();
            this.miCuspErase = new System.Windows.Forms.MenuItem();
            this.miIntersectErase = new System.Windows.Forms.MenuItem();
            this.miStrokeErase = new System.Windows.Forms.MenuItem();
            // 
            // miMain
            // 
            this.miMain.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
                                                                                   this.miMainAction,
                                                                                   this.miMainMode});
            // 
            // miMainAction
            // 
            this.miMainAction.Index = 0;
            this.miMainAction.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
                                                                                         this.miClear,
                                                                                         this.miSeparator,
                                                                                         this.miExit});
            this.miMainAction.Text = "&Action";
            // 
            // miClear
            // 
            this.miClear.Index = 0;
            this.miClear.Text = "&Clear";
            this.miClear.Click += new System.EventHandler(this.miClear_Click);
            // 
            // miSeparator
            // 
            this.miSeparator.Index = 1;
            this.miSeparator.Text = "-";
            // 
            // miExit
            // 
            this.miExit.Index = 2;
            this.miExit.Text = "E&xit";
            this.miExit.Click += new System.EventHandler(this.miExit_Click);
            // 
            // miMainMode
            // 
            this.miMainMode.Index = 1;
            this.miMainMode.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
                                                                                       this.miInk,
                                                                                       this.miCuspErase,
                                                                                       this.miIntersectErase,
                                                                                       this.miStrokeErase});
            this.miMainMode.Text = "&Mode";
            // 
            // miInk
            // 
            this.miInk.Checked = true;
            this.miInk.Index = 0;
            this.miInk.RadioCheck = true;
            this.miInk.Shortcut = System.Windows.Forms.Shortcut.CtrlI;
            this.miInk.Text = "&Ink";
            this.miInk.Click += new System.EventHandler(this.miInk_Click);
            // 
            // miCuspErase
            // 
            this.miCuspErase.Index = 1;
            this.miCuspErase.RadioCheck = true;
            this.miCuspErase.Shortcut = System.Windows.Forms.Shortcut.CtrlC;
            this.miCuspErase.Text = "Erase at &Cusps";
            this.miCuspErase.Click += new System.EventHandler(this.miCuspErase_Click);
            // 
            // miIntersectErase
            // 
            this.miIntersectErase.Index = 2;
            this.miIntersectErase.RadioCheck = true;
            this.miIntersectErase.Shortcut = System.Windows.Forms.Shortcut.CtrlN;
            this.miIntersectErase.Text = "Erase at I&ntersections";
            this.miIntersectErase.Click += new System.EventHandler(this.miIntersectErase_Click);
            // 
            // miStrokeErase
            // 
            this.miStrokeErase.Index = 3;
            this.miStrokeErase.RadioCheck = true;
            this.miStrokeErase.Shortcut = System.Windows.Forms.Shortcut.CtrlS;
            this.miStrokeErase.Text = "Erase &Strokes";
            this.miStrokeErase.Click += new System.EventHandler(this.miStrokeErase_Click);
            // 
            // InkErase
            // 
            this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
            this.ClientSize = new System.Drawing.Size(292, 267);
            this.Menu = this.miMain;
            this.Name = "InkErase";
            this.Text = "InkErase";
            this.Load += new System.EventHandler(this.InkErase_Load);
            this.Paint += new System.Windows.Forms.PaintEventHandler(this.InkErase_OnPaint);
            this.MouseMove += new System.Windows.Forms.MouseEventHandler(this.InkErase_OnMouseMove);

        }
        #endregion

        #region Standard Template Code
        /// <summary>
        /// The main entry point for the application.
        /// </summary>
        [STAThread]
        static void Main() 
        {
            Application.Run(new InkErase());
        }
        #endregion

        /// <summary>
        /// Event Handle from form's Load event
        /// </summary>
        /// <param name="sender">The control that raised the event.</param>
        /// <param name="e">The event arguments.</param>
        private void InkErase_Load(object sender, System.EventArgs e)
        {
            // Start the application in inking mode
            mode = ApplicationMode.Ink;

            // Create a new ink collector and assign it to this form's window          
            myInkCollector = new InkCollector(Handle);

            // Turn off auto-redrawing since this sample application
            // needs to display the stroke cusps as red points over the strokes.
            // If autoredraw is enabled, the strokes will be drawn over
            // the red points, which will make the cusps hard to see.
            myInkCollector.AutoRedraw = false;

            // Set the pen width to be a medium width
            myInkCollector.DefaultDrawingAttributes.Width = MediumInkWidth;

            // Hook event handle for Cursor down event to myInkCollector_CursorDown.
            // This is necessary since the application needs to check if the cursor
            // is inverted and use the result to determine the visibility of the ink.
            myInkCollector.CursorDown += new InkCollectorCursorDownEventHandler(myInkCollector_CursorDown);

            // Hook event handle for NewPackets event to myInkCollector_NewPackets.
            // This is necessary since the application needs to examine new packets
            // when the cursor is inverted and use them to determine whether
            // any strokes should be erased.
            myInkCollector.NewPackets += new InkCollectorNewPacketsEventHandler(myInkCollector_NewPackets);

            // Hook event handle for the Stroke event to myInkCollector_Stroke.
            // This is necessary since the application needs to cancel strokes drawn
            // while the cursor is inverted.
            myInkCollector.Stroke += new InkCollectorStrokeEventHandler(myInkCollector_Stroke); 

            // Turn the ink collector on
            myInkCollector.Enabled = true;
        }

        /// <summary>
        /// Event Handle from MouseMove event.
        /// </summary>
        /// <param name="sender">The control that raised the event.</param>
        /// <param name="e">The event arguments.</param>
        private void InkErase_OnMouseMove(object sender, MouseEventArgs e)
        {

            // If the application is in erase mode and a mouse button is
            // pressed, perform a hit test to determine which stroke segments
            // to erase (if any).
            if ( (ApplicationMode.Ink != mode) && (MouseButtons.None != MouseButtons) )
            {

                Point pt = new Point(e.X, e.Y); 

                // Convert the specified point from pixel to ink space coordinates
                using (Graphics g = CreateGraphics())
                {
                    myInkCollector.Renderer.PixelToInkSpace(g, ref pt);
                }

                switch(mode)
                {
                    case ApplicationMode.CuspErase:
                        EraseAtCusps(pt);
                        break;
                    case ApplicationMode.IntersectErase:
                        EraseAtIntersections(pt);
                        break;
                    case ApplicationMode.StrokeErase:
                        EraseStrokes(pt,null);
                        break;
                }
            }
        }

        /// <summary>
        /// Event Handle from Paint event.  It is necessary to handle the
        /// paint event since this sample needs to draw red points to indicate
        /// the strokes' cusps.
        /// </summary>
        /// <param name="sender">The control that raised the event.</param>
        /// <param name="e">The event arguments.</param>
        private void InkErase_OnPaint(object sender, PaintEventArgs e)
        {
            // Get the strokes to paint from the ink
            Strokes strokesToPaint = myInkCollector.Ink.Strokes;

            // Draw the strokes - note that it is necessary to manually
            // paint the strokes since auto-redrawing is set to false.
            myInkCollector.Renderer.Draw(e.Graphics, strokesToPaint);

            switch (mode)
            {
                case ApplicationMode.CuspErase:
                    PaintCusps(e.Graphics, strokesToPaint);
                    break;
                case ApplicationMode.IntersectErase:
                    PaintIntersections(e.Graphics, strokesToPaint);
                    break;
            }
        }

        /// <summary>
        /// Event Handle from Action->Clear menu.
        /// </summary>
        /// <param name="sender">The control that raised the event.</param>
        /// <param name="e">The event arguments.</param>
        private void miClear_Click(object sender, System.EventArgs e)
        {
            Strokes strokesToDelete = myInkCollector.Ink.Strokes;
            
            // Check to ensure that the ink collector isn't currently
            // in the middle of a stroke before clearing the ink.
            // Deleting a stroke that is currently being collected
            // will result in an error condition.
            if (!myInkCollector.CollectingInk)
            {
                myInkCollector.Ink.DeleteStrokes(strokesToDelete);

                miInk_Click(sender, e);
            }
            else
            {
                MessageBox.Show("Cannot clear ink while the ink collector is busy.");
            }
        }

        /// <summary>
        /// Event Handle from Action->Exit menu.
        /// </summary>
        /// <param name="sender">The control that raised the event.</param>
        /// <param name="e">The event arguments.</param>
        private void miExit_Click(object sender, System.EventArgs e)
        {
            myInkCollector.Enabled = false;
            Application.Exit();
        }

        /// <summary>
        /// Event Handle from Mode->Ink menu.
        /// </summary>
        /// <param name="sender">The control that raised the event.</param>
        /// <param name="e">The event arguments.</param>
        private void miInk_Click(object sender, System.EventArgs e)
        {
            UpdateApplicationMode(ApplicationMode.Ink);
        }

        /// <summary>
        /// Event Handle from Mode->Cusp Erase menu.
        /// </summary>
        /// <param name="sender">The control that raised the event.</param>
        /// <param name="e">The event arguments.</param>
        private void miCuspErase_Click(object sender, System.EventArgs e)
        {
            UpdateApplicationMode(ApplicationMode.CuspErase);
        }
        
        /// <summary>
        /// Event Handle from Mode->Intersect Erase menu.
        /// </summary>
        /// <param name="sender">The control that raised the event.</param>
        /// <param name="e">The event arguments.</param>
        private void miIntersectErase_Click(object sender, System.EventArgs e)
        {
             UpdateApplicationMode(ApplicationMode.IntersectErase);       
        }

        /// <summary>
        /// Event Handle from Mode->Stroke Erase menu.
        /// </summary>
        /// <param name="sender">The control that raised the event.</param>
        /// <param name="e">The event arguments.</param>
        private void miStrokeErase_Click(object sender, System.EventArgs e)
        {
            UpdateApplicationMode(ApplicationMode.StrokeErase);
        }

        /// <summary>
        /// Helper method to update the application mode
        /// </summary>
        /// <param name="newMode">The new mode</param>
        private void UpdateApplicationMode(ApplicationMode newMode)
        {
            // Turn on/off the ink collector
            myInkCollector.Enabled = (ApplicationMode.Ink == newMode);

            // Update the state of the Ink and Erase menu items
            miInk.Checked = (ApplicationMode.Ink == newMode);
            miCuspErase.Checked  = (ApplicationMode.CuspErase == newMode);
            miIntersectErase.Checked = (ApplicationMode.IntersectErase == newMode);
            miStrokeErase.Checked = (ApplicationMode.StrokeErase == newMode);

            mode = newMode;

            Refresh();
        }

        /// <summary>
        ///  Event Handle from Cursor event
        /// </summary>
        /// <param name="sender">The control that raised the event.</param>
        /// <param name="e">The event arguments.</param>
        public void myInkCollector_CursorDown(object sender, InkCollectorCursorDownEventArgs e)
        {
            // If the pen is inverted, this application will perform stroke
            // erasing; since we do not want to show the pen while the user
            // is erasing, make this stroke transparent.
            if (e.Cursor.Inverted)
            {
                e.Stroke.DrawingAttributes.Transparency = 255;
            }
        }

        /// <summary>
        /// Event Handler from Ink Collector's NewPackets event
        /// 
        /// This event is fired when the Ink Collector receives 
        /// new packet data.  
        /// </summary>
        /// <param name="sender">The control that raised the event.</param>
        /// <param name="e">The event arguments.</param>
        private void myInkCollector_NewPackets(object sender, InkCollectorNewPacketsEventArgs e)
        {
            // If the cursor is inverted
            if (e.Cursor.Inverted)
            {

                // retrieve the size of each packet
                int packetSize = e.Stroke.PacketSize;
        
                Point pt = Point.Empty;

                // Perform a hit test with each point and delete
                // the hit strokes
                for (int i = 0; i < e.PacketCount; i++)
                {
                    // retrieve the x and y packet values 
                    pt.X = e.PacketData[i*packetSize+XPacketIndex];
                    pt.Y = e.PacketData[i*packetSize+YPacketIndex];

                    EraseStrokes(pt,e.Stroke);
                }
            }
        }

        /// <summary>
        ///  Event Handle from Stroke event
        /// </summary>
        /// <param name="sender">The control that raised the event.</param>
        /// <param name="e">The event arguments.</param>
        public void myInkCollector_Stroke(object sender, InkCollectorStrokeEventArgs e )
        {
            if (e.Cursor.Inverted)
            {
                e.Cancel = true;
            }
        }

        /// <summary>
        /// Helper method to paint the stroke collection's cusps
        /// </summary>
        /// <param name="g">The graphics object to use for painting</param>
        /// <param name="strokesToPaint">The collection of strokes to paint</param>
        private void PaintCusps(Graphics g, Strokes strokesToPaint)
        {
            // now draw PolylineCusp points
            foreach (Stroke currentStroke in strokesToPaint)
            {

                // Retrieve the cusps of the stroke.  The cusps mark the points where
                // the stroke changes direction abruptly.  A segment is defined as the
                // points between two cusps.
                int[] cusps = currentStroke.PolylineCusps;

                // Draw each cusp in the stroke
                foreach (int i in cusps)
                {
                    
                    // Get the X, Y position of the cusp
                    Point pt = currentStroke.GetPoint(i);

                    // Convert the X, Y position to Window based pixel coordinates
                    myInkCollector.Renderer.InkSpaceToPixel(g, ref pt);

                    // Draw a red circle as the cusp position
                    g.DrawEllipse(Pens.Red, pt.X-3, pt.Y-3, 6, 6);
                }
            }
        }

        /// <summary>
        /// Helper method to paint the stroke collection's intersections
        /// </summary>
        /// <param name="g">The graphics object to use for painting</param>
        /// <param name="strokesToPaint">The collection of strokes to paint</param>
        private void PaintIntersections(Graphics g, Strokes strokesToPaint)
        {
            // Draw the intersections of each stroke as little red circles
            foreach (Stroke currentStroke in strokesToPaint)
            {

                // Get the intersections of the stroke
                float[] intersections = currentStroke.FindIntersections(strokesToPaint);

                Point[] points = currentStroke.GetPoints();

                // Draw each intersection in the stroke
                foreach (float fi in intersections)
                {     
                    // Get the point before the FINDEX
                    Point ptIntersect = currentStroke.GetPoint((int)fi);

                    // Find the fractional part of the FINDEX
                    float fiFraction = fi - (int)fi;

                    // if the fi does not have a fractional part, we have already
                    // found the intersection point.  Otherwise, use the FINDEX to 
                    // calculate the interpolated intersection point on the stroke
                    if (fiFraction > 0.0f)
                    {
                        Point ptNextIntersect = currentStroke.GetPoint((int)fi + 1);
                        ptIntersect.X += (int)((ptNextIntersect.X - ptIntersect.X) * fiFraction);
                        ptIntersect.Y += (int)((ptNextIntersect.Y - ptIntersect.Y) * fiFraction);

                    }

                    // Convert the X, Y position to Window based pixel coordinates
                    myInkCollector.Renderer.InkSpaceToPixel(g, ref ptIntersect);

                    // Draw a red circle as the intersection position
                    g.DrawEllipse(Pens.Red, ptIntersect.X-3, ptIntersect.Y-3, 6, 6);
                }
            }
        }

        /// <summary>
        /// Helper method that performs a hit test using the specified point.
        /// When a hit occurs, the application splits the stroke at the cusps
        /// on either side of the hit and erases the corresponding stroke segment. 
        /// </summary>
        /// <param name="pt">The point to use for hit testing.</param>
        private void EraseAtCusps(Point pt)
        {
            
            // Declare the collection of strokes returned from HitTest
            Strokes strokesHit = null;

            // Use HitTest to find the collection of strokes that are intersected
            // by the point.  The HitTestRadius constant is used to specify the 
            // radius of the hit test circle in ink space coordinates (1 unit = .01mm).
            strokesHit = myInkCollector.Ink.HitTest(pt, HitTestRadius);

            // Loop over each stroke returned from the hit test to determine 
            // which portion to erase...
            foreach (Stroke currentStroke in strokesHit)
            {

                // Retrieve the cusps of the stroke.  The cusps mark the points where
                // the stroke changes direction abruptly.  A segment is defined as the
                // points between two cusps.
                int[] cusps = currentStroke.PolylineCusps;

                // If there are 1 or two cusps, it's a single stroke - delete the 
                // entire stroke.
                if (cusps.Length <= 2)
                {
                    myInkCollector.Ink.DeleteStroke(currentStroke);
                }

                // If there are more than 2 cusps, determine which cusps bound the 
                // hit-tested portion of the stroke, split the stroke at these cusps,
                // and delete the stroke that defines the segment we hit-tested.
                else
                {
                    // Get the FINDEX of the nearest point on the stroke.  An FINDEX 
                    // is a float value representing a location somewhere between two
                    // points in the stroke.  For instance, 0.0 is the first point in
                    // the stroke. 1.0 is the second point in the stroke. 0.5 is halfway
                    // between the first and second points. 
                    float findex = currentStroke.NearestPoint(pt);

                    // Declare the stroke segment to delete
                    Stroke strokeToDelete = null;

                    // Cycle through each cusp of the stroke to determine
                    // which cusps bound the hit-tested portion of the stroke...
                    for(int i = cusps.Length-2; i>=0; i--)
                    {
                        // If this cusp is less than the findex, then split
                        // the stroke at this cusp and the cusp immediately
                        // after it
                        if (cusps[i]<=findex)
                        {
                            // Provided we aren't at the end of the stroke, split at 
                            // cusp i+1.
                            if (i < (cusps.Length-2))
                            {
                                strokeToDelete = currentStroke.Split(cusps[i+1]);
                            }
                           
                            // If the hit occurred between the first and second cusp,
                            // delete the stroke.  Keep in mind that the stroke has
                            // already been split at index 1, so the delete will only
                            // remove the beginning portion of the stroke (as desired).
                            if (i==0)
                            {
                                myInkCollector.Ink.DeleteStroke(currentStroke);
                            }
                            // Otherwise, split the stroke at the current cusp and
                            // delete the result.  Keep in mind that the stroke has
                            // already been split at index i+1, so the delete will
                            // remove the segment from cusp i to i+1.
                            else
                            {
                                strokeToDelete = currentStroke.Split(cusps[i]);
                                myInkCollector.Ink.DeleteStroke(strokeToDelete);
                            }

                            break;
                        }
                    } 
                }
            }

            if (strokesHit.Count > 0)
            {
                // Repaint the screen to reflect the change
                this.Refresh();
            }
        }

        /// <summary>
        /// Helper method that performs a hit test using the specified point.
        /// When a hit occurs, the application splits the stroke at the intersections
        /// on either side of the hit and erases the corresponding stroke segment. 
        /// </summary>
        /// <param name="pt">The point to use for hit testing.</param>
        private void EraseAtIntersections(Point pt)
        {

            // Use HitTest to find the collection of strokes that are intersected
            // by the point.  The HitTestRadius constant is used to specify the 
            // radius of the hit test circle in ink space coordinates (1 unit = .01mm).
            Strokes strokesHit = myInkCollector.Ink.HitTest(pt, HitTestRadius);

            // Loop over each stroke returned from the hit test to determine 
            // which portion to erase...
            foreach (Stroke currentStroke in strokesHit)
            {

                // Retrieve the intersection of the stroke.  
                float[] intersections = currentStroke.FindIntersections(myInkCollector.Ink.Strokes);

                // If there aren't any intersections, delete the entire stroke.
                if (intersections.Length <= 0)
                {
                    myInkCollector.Ink.DeleteStroke(currentStroke);
                }

                // If there is at least one intersection, determine which
                // intersections bound the hit-tested portion of the stroke,
                // split the stroke at these intersections, and delete the stroke
                // that defines the segment we hit-tested.
                else
                {
                    // Get the FINDEX of the nearest point on the stroke.  An FINDEX 
                    // is a float value representing a location somewhere between two
                    // points in the stroke.  For instance, 0.0 is the first point in
                    // the stroke. 1.0 is the second point in the stroke. 0.5 is halfway
                    // between the first and second points. 
                    float findex = currentStroke.NearestPoint(pt);

                    // If the hit occured before the first intersection, split the stroke
                    // at the current intersection and delete the beginning of the
                    // stroke
                    if (findex < intersections[0])
                    {
                        currentStroke.Split (intersections[0]);
                        myInkCollector.Ink.DeleteStroke(currentStroke);
                    }
                    else
                    {
                        // Declare the stroke segment to delete
                        Stroke strokeToDelete = null;

                        // Cycle through each intersection of the stroke to determine
                        // which intersections bound the hit-tested portion of the stroke...
                        for(int i = intersections.Length-1; i>=0; i--)
                        {

                            // If this intersection is less than the findex, the intersection
                            // occurs between this intersection and the one after it
                            if (intersections[i]<=findex)
                            {
                                // Provided we aren't at the end of the stroke, split at 
                                // intersection i+1.
                                if (i < (intersections.Length-1))
                                {
                                    strokeToDelete = currentStroke.Split(intersections[i+1]);
                                }
    
                                // Split the stroke at the current intersection.  Keep in
                                // mind that the stroke has already been split at index i+1,
                                // so the delete will remove the segment from intersection i
                                // to i+1.
                                strokeToDelete = currentStroke.Split(intersections[i]);
                                myInkCollector.Ink.DeleteStroke(strokeToDelete);

                                break;
                            }
                        }
                    } 
                }
            }

            if (strokesHit.Count > 0)
            {
                // Repaint the screen to reflect the change
                this.Refresh();
            }
        }

        /// <summary>
        /// Helper method that performs a hit test using the specified point.
        /// It deletes all strokes that were hit by the point
        /// </summary>
        /// <param name="pt">The point to use for hit testing</param>
        private void EraseStrokes(Point pt, Stroke currentStroke)
        {

            // Use HitTest to find the collection of strokes that are intersected
            // by the point.  The HitTestRadius constant is used to specify the 
            // radius of the hit test circle in ink space coordinates (1 unit = .01mm).
            Strokes strokesHit = myInkCollector.Ink.HitTest(pt, HitTestRadius);

            if (null!=currentStroke && strokesHit.Contains(currentStroke))
            {
                strokesHit.Remove(currentStroke);
            }

            // Delete all strokes that were hit by the point
            myInkCollector.Ink.DeleteStrokes(strokesHit);

            if (strokesHit.Count > 0)
            {
                // Repaint the screen to reflect the change
                this.Refresh();
            }
        }
    }
}

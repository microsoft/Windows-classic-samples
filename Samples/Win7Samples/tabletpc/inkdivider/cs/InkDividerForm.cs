// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//  File: InkDividerForm.cs
//  Simple Ink Divider Sample Application
//
//  This sample is based on the Ink Collection sample. An InkDivider 
//  is created and asked to do layout analysis. In Automatic Layout 
//  Analysis mode (accessible through Mode->Automatic Layout Analysis 
//  menu item) the Ink Divider is invoked whenever the ink is changed. 
//  Otherwise, the Ink Division can be invoked through File->Divide 
//  menu item. In either case, the default recognizer is used, if 
//  available. The divide method is called. The UI is then updated by 
//  drawing a bounding rectangle around each parsed unit. Besides using 
//  different colors, these rectangles are inflated by different amounts 
//  to ensure that none of the rectangles is obscured by others. 
//  The following table specifies the color and inflation for each units:
//
//      |-----------|-----------|-----------|
//      | Unit      |  Color    | Inflation |
//      |-----------|-----------|-----------|
//      | Word      | Green     |    1      |
//      |-----------|-----------|-----------|
//      | Line      | Magenta   |    3      |
//      |-----------------------|-----------|
//      | Paragraph | Blue      |    5      |
//      |-----------------------|-----------|
//      | Drawing   | Red       |    1      |
//      |-----------------------------------|
//
//  The application has the capability of erasing strokes. Two menu items
//  Mode->Ink and Mode->Erase are provided to switch between inking and
//  erasing modes. As the new ink is added or deleted to or from the Ink 
//  object, the ink divider is kept updated. It makes InkDivider.Divide()
//  method to work fast.
//  The menu item for Mode->Automatic Layout Analysis is checked by default.
//  With this option checked, the InkCollector object's Stroke and StrokeDeleted 
//  event handlers call this method every time a stroke is made or deleted. With 
//  more than a few strokes present, the call to the Divider object's Divide 
//  method creates a noticeable delay. In practice, call the Divide method only 
//  when you need the division result.
//
//  The features used are: InkDivider, InkOverlay, Erasing Ink, 
//      InkCollectorStrokeEventHandler, InkOverlayStrokesDeletingEventHandler, 
//      InkOverlayStrokesDeletedEventHandler and InkSpaceToPixel.
//
//--------------------------------------------------------------------------

using System;
using System.Drawing;
using System.Windows.Forms;

// The Ink namespace, which contains the Tablet PC Platform API
using Microsoft.Ink;


namespace Microsoft.Samples.TabletPC.InkDivider
{
    /// <summary>
    /// InkDivider sample application form class.
    /// </summary>
    public class InkDividerForm : System.Windows.Forms.Form
    {
        // Declare the Ink Overlay object
        private InkOverlay myInkOverlay = null;

        // Declare the ink divider object
        private Divider myInkDivider = null;

        // Collection of Bounding Boxes for words, drawings, lines and paragraphs
        Rectangle[] myWordBoundingBoxes;
        Rectangle[] myDrawingBoundingBoxes;
        Rectangle[] myLineBoundingBoxes;
        Rectangle[] myParagraphBoundingBoxes;
        
        #region Standard Template Code
        private System.Windows.Forms.MainMenu mainMenu;
        private System.Windows.Forms.Panel DrawArea;
        private System.Windows.Forms.StatusBar statusBar;
        private System.Windows.Forms.StatusBarPanel statusBarPanelDummy;
        private System.Windows.Forms.StatusBarPanel statusBarPanelDrawing;
        private System.Windows.Forms.StatusBarPanel statusBarPanelParagraph;
        private System.Windows.Forms.StatusBarPanel statusBarPanelWord;
        private System.Windows.Forms.StatusBarPanel statusBarPanelLine;
        private System.Windows.Forms.MenuItem miFile;
        private System.Windows.Forms.MenuItem miDivide;
        private System.Windows.Forms.MenuItem miExit;
        private System.Windows.Forms.MenuItem miMode;
        private System.Windows.Forms.MenuItem miInk;
        private System.Windows.Forms.MenuItem miErase;
        private System.Windows.Forms.MenuItem miAutomaticLayoutAnalysis;
        private System.Windows.Forms.MenuItem miSeparate;
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.Container components = null;
        #endregion

        public InkDividerForm()
        {
            #region Standard Template Code
            //
            // Required for Windows Form Designer support
            //
            InitializeComponent();

            //
            // Add any constructor code after InitializeComponent call to be added here
            //
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
                
                // dispose the ink overlay's resources
                if (myInkOverlay != null)
                {
                    myInkOverlay.Dispose();
                }
                
                // dispose the divider's resources
                if (myInkDivider != null)
                {
                    // dispose the recognizer context that we associated
                    // with the divider
                    if (myInkDivider.RecognizerContext != null)
                    {
                        myInkDivider.RecognizerContext.Dispose();
                    }
                    
                    // dispose the strokes that we associated
                    // with the divider
                    if (myInkDivider.Strokes != null)
                    {
                        myInkDivider.Strokes.Dispose();
                    }
                    myInkDivider.Dispose();
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
            this.mainMenu = new System.Windows.Forms.MainMenu();
            this.miFile = new System.Windows.Forms.MenuItem();
            this.miDivide = new System.Windows.Forms.MenuItem();
            this.miExit = new System.Windows.Forms.MenuItem();
            this.miMode = new System.Windows.Forms.MenuItem();
            this.miInk = new System.Windows.Forms.MenuItem();
            this.miErase = new System.Windows.Forms.MenuItem();
            this.miSeparate = new System.Windows.Forms.MenuItem();
            this.miAutomaticLayoutAnalysis = new System.Windows.Forms.MenuItem();
            this.DrawArea = new System.Windows.Forms.Panel();
            this.statusBar = new System.Windows.Forms.StatusBar();
            this.statusBarPanelDummy = new System.Windows.Forms.StatusBarPanel();
            this.statusBarPanelWord = new System.Windows.Forms.StatusBarPanel();
            this.statusBarPanelLine = new System.Windows.Forms.StatusBarPanel();
            this.statusBarPanelParagraph = new System.Windows.Forms.StatusBarPanel();
            this.statusBarPanelDrawing = new System.Windows.Forms.StatusBarPanel();
            ((System.ComponentModel.ISupportInitialize)(this.statusBarPanelDummy)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.statusBarPanelWord)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.statusBarPanelLine)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.statusBarPanelParagraph)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.statusBarPanelDrawing)).BeginInit();
            this.SuspendLayout();
            // 
            // mainMenu
            // 
            this.mainMenu.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
                                                                                     this.miFile,
                                                                                     this.miMode});
            // 
            // miFile
            // 
            this.miFile.Index = 0;
            this.miFile.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
                                                                                   this.miDivide,
                                                                                   this.miExit});
            this.miFile.Text = "&File";
            // 
            // miDivide
            // 
            this.miDivide.Index = 0;
            this.miDivide.Text = "&Divide";
            this.miDivide.Click += new System.EventHandler(this.miDivide_Click);
            // 
            // miExit
            // 
            this.miExit.Index = 1;
            this.miExit.Text = "E&xit";
            this.miExit.Click += new System.EventHandler(this.miExit_Click);
            // 
            // miMode
            // 
            this.miMode.Index = 1;
            this.miMode.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
                                                                                   this.miInk,
                                                                                   this.miErase,
                                                                                   this.miSeparate,
                                                                                   this.miAutomaticLayoutAnalysis});
            this.miMode.Text = "&Mode";
            // 
            // miInk
            // 
            this.miInk.Index = 0;
            this.miInk.Text = "&Ink";
            this.miInk.Click += new System.EventHandler(this.miInk_Click);
            // 
            // miErase
            // 
            this.miErase.Index = 1;
            this.miErase.Text = "&Erase";
            this.miErase.Click += new System.EventHandler(this.miErase_Click);
            // 
            // miSeparate
            // 
            this.miSeparate.Index = 2;
            this.miSeparate.Text = "-";
            // 
            // miAutomaticLayoutAnalysis
            // 
            this.miAutomaticLayoutAnalysis.Checked = true;
            this.miAutomaticLayoutAnalysis.Index = 3;
            this.miAutomaticLayoutAnalysis.Text = "&Automatic Layout Analysis";
            this.miAutomaticLayoutAnalysis.Click += new System.EventHandler(this.miAutomaticLayoutAnalysis_Click);
            // 
            // DrawArea
            // 
            this.DrawArea.Anchor = (((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
                | System.Windows.Forms.AnchorStyles.Left) 
                | System.Windows.Forms.AnchorStyles.Right);
            this.DrawArea.BackColor = System.Drawing.SystemColors.Window;
            this.DrawArea.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
            this.DrawArea.Name = "DrawArea";
            this.DrawArea.Size = new System.Drawing.Size(560, 420);
            this.DrawArea.TabIndex = 2;
            this.DrawArea.Paint += new System.Windows.Forms.PaintEventHandler(this.DrawArea_Paint);
            // 
            // statusBar
            // 
            this.statusBar.Location = new System.Drawing.Point(0, 420);
            this.statusBar.Name = "statusBar";
            this.statusBar.Panels.AddRange(new System.Windows.Forms.StatusBarPanel[] {
                                                                                         this.statusBarPanelDummy,
                                                                                         this.statusBarPanelWord,
                                                                                         this.statusBarPanelLine,
                                                                                         this.statusBarPanelParagraph,
                                                                                         this.statusBarPanelDrawing});
            this.statusBar.RightToLeft = System.Windows.Forms.RightToLeft.No;
            this.statusBar.ShowPanels = true;
            this.statusBar.Size = new System.Drawing.Size(560, 21);
            this.statusBar.TabIndex = 3;
            // 
            // statusBarPanelDummy
            // 
            this.statusBarPanelDummy.AutoSize = System.Windows.Forms.StatusBarPanelAutoSize.Spring;
            this.statusBarPanelDummy.BorderStyle = System.Windows.Forms.StatusBarPanelBorderStyle.None;
            this.statusBarPanelDummy.Width = 200;
            // 
            // statusBarPanelWord
            // 
            this.statusBarPanelWord.AutoSize = System.Windows.Forms.StatusBarPanelAutoSize.Contents;
            this.statusBarPanelWord.Text = "Green: Word";
            this.statusBarPanelWord.Width = 79;
            // 
            // statusBarPanelLine
            // 
            this.statusBarPanelLine.AutoSize = System.Windows.Forms.StatusBarPanelAutoSize.Contents;
            this.statusBarPanelLine.Text = "Magenta: Line";
            this.statusBarPanelLine.Width = 86;
            // 
            // statusBarPanelParagraph
            // 
            this.statusBarPanelParagraph.AutoSize = System.Windows.Forms.StatusBarPanelAutoSize.Contents;
            this.statusBarPanelParagraph.Text = "Blue: Paragraph";
            this.statusBarPanelParagraph.Width = 96;
            // 
            // statusBarPanelDrawing
            // 
            this.statusBarPanelDrawing.AutoSize = System.Windows.Forms.StatusBarPanelAutoSize.Contents;
            this.statusBarPanelDrawing.Text = "Red: Drawing";
            this.statusBarPanelDrawing.Width = 83;
            // 
            // InkDividerForm
            // 
            this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
            this.ClientSize = new System.Drawing.Size(560, 441);
            this.Controls.AddRange(new System.Windows.Forms.Control[] {
                                                                          this.statusBar,
                                                                          this.DrawArea});
            this.Menu = this.mainMenu;
            this.Name = "InkDividerForm";
            this.Text = "Ink Divider";
            this.Load += new System.EventHandler(this.InkDividerForm_Load);
            ((System.ComponentModel.ISupportInitialize)(this.statusBarPanelDummy)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.statusBarPanelWord)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.statusBarPanelLine)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.statusBarPanelParagraph)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.statusBarPanelDrawing)).EndInit();
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
            Application.Run(new InkDividerForm());
        }
        #endregion


        /// <summary>
        /// Paint method gets called everytime when the window is refreshed.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void DrawArea_Paint(object sender, System.Windows.Forms.PaintEventArgs e)
        {
            // Create the Pen used to draw bounding boxes.
            // First set of bounding boxes drawn here are the bounding boxes of paragraphs.
            // These boxes are drawn with Blue pen.
            Pen penBox = new Pen(Color.Blue, 2);

            // First, draw the bounding boxes for Paragraphs
            if(null != myParagraphBoundingBoxes)
            {
                // Draw bounding boxes for Paragraphs
                e.Graphics.DrawRectangles(penBox, myParagraphBoundingBoxes);
            }

            // Next, draw the bounding boxes for Lines
            if(null != myLineBoundingBoxes)
            {
                // Color is Magenta pen
                penBox.Color = Color.Magenta;
                // Draw the bounding boxes for Lines
                e.Graphics.DrawRectangles(penBox, myLineBoundingBoxes);
            }
            
            // Then, draw the bounding boxes for Words
            if(null != myWordBoundingBoxes)
            {
                // Color is Green
                penBox.Color = Color.Green;
                // Draw bounding boxes for Words
                e.Graphics.DrawRectangles(penBox, myWordBoundingBoxes);
            }
            
            // Finally, draw the boxes for Drawings
            if(null != myDrawingBoundingBoxes)
            {
                // Color is Red pen
                penBox.Color = Color.Red;
                // Draw bounding boxes for Drawings
                e.Graphics.DrawRectangles(penBox, myDrawingBoundingBoxes);
            }
        }

        /// <summary>
        /// Event Handler from Form Load Event
        /// Setup the ink overlay for collection
        /// </summary>
        /// <param name="sender">The control that raised the event.</param>
        /// <param name="e">The event arguments.</param>
        private void InkDividerForm_Load(object sender, System.EventArgs e)
        {
            // Create the ink overlay and associate it with the form
            myInkOverlay = new Microsoft.Ink.InkOverlay(DrawArea.Handle);

            // Hook event handler for the Stroke event to myInkOverlay_Stroke.
            // This is necessary since the application needs to pass the strokes 
            // to the ink divider.
            myInkOverlay.Stroke += new InkCollectorStrokeEventHandler(myInkOverlay_Stroke); 

            // Hook the event handler for StrokeDeleting event to myInkOverlay_StrokeDeleting.
            // This is necessary as the application needs to remove the strokes from 
            // ink divider object as well.
            myInkOverlay.StrokesDeleting += new InkOverlayStrokesDeletingEventHandler(myInkOverlay_StrokeDeleting);

            // Hook the event handler for StrokeDeleted event to myInkOverlay_StrokeDeleted.
            // This is necessary to update the layout analysis result when automatic layout analysis
            // option is selected.
            myInkOverlay.StrokesDeleted += new InkOverlayStrokesDeletedEventHandler(myInkOverlay_StrokeDeleted);

            // Create the ink divider object
            myInkDivider = new Divider();

            // Add a default recognizer context to the divider object
            // without adding the recognizer context, the divider would
            // not use a recognizer to do its word segmentation and would
            // have less accurate results.
            // Adding the recognizer context will slow down the call to 
            // myInkDivider.Divide though.
            // It is possible that there is no recognizer installed on the
            // machine for this language. In that case the divider will
            // not use a recognizer to improve its accuracy.
            // Get the default recognizer if any
            try
            {
                Recognizers recognizers = new Recognizers();
                myInkDivider.RecognizerContext = recognizers.GetDefaultRecognizer().CreateRecognizerContext();
            }
            catch (InvalidOperationException)
            {
                //We are in the case where no default recognizers can be found
            }

            // The LineHeight property helps the InkDivider distinguish between 
            // drawing and handwriting. The value should be the expected height 
            // of the user's handwriting in ink space units (0.01mm).
            // Here we set the LineHeight to 840, which is about 1/3 of an inch.
            myInkDivider.LineHeight = 840;

            // Assign ink overlay's strokes collection to the ink divider
            // This strokes collection will be updated in the event handler
            myInkDivider.Strokes = myInkOverlay.Ink.Strokes;

            // Enable ink collection
            myInkOverlay.Enabled = true;

            // Set check for ink menu item
            miInk.Checked = true;
        }

        /// <summary>
        /// Helper function to obtain array of rectangles from the 
        /// division result of the division type of interest. Each rectangle
        /// is inflated by the amount specified in the third parameter. This
        /// is done to ensure the visibility of all rectangles.
        /// </summary>
        /// <param name="divResult">Ink Divider division result</param>
        /// <param name="divType">Division type</param>
        /// <param name="inflate">Number of Pixels by which the rectangles are inflated</param>
        /// <returns> Array of rectangles containing bounding boxes of 
        /// division type specified by divType. The rectangles are in pixel unit.</returns>
        private Rectangle[] GetUnitBBoxes(DivisionResult divResult, InkDivisionType divType, int inflate)
        {
            // Declare the array of rectangles to hold the result
            Rectangle[] divRects;

            // Get the division units from the division result of division type
            DivisionUnits units = divResult.ResultByType(divType);
            
            // If there is at least one unit, we construct the rectangles
            if((null != units) && (0 < units.Count))
            {
                // We need to convert rectangles from ink units to
                // pixel units. For that, we need Graphics object
                // to pass to InkRenderer.InkSpaceToPixel method
                using (Graphics g = DrawArea.CreateGraphics())
                {
            
                    // Construct the rectangles
                    divRects = new Rectangle[units.Count];
                    
                    // InkRenderer.InkSpaceToPixel takes Point as parameter. 
                    // Create two Point objects to point to (Top, Left) and
                    // (Width, Height) properties of ractangle. (Width, Height)
                    // is used instead of (Right, Bottom) because (Right, Bottom)
                    // are read-only properties on Rectangle
                    Point ptLocation = new Point();
                    Point ptSize = new Point();
                    
                    // Index into the bounding boxes
                    int i = 0;

                    // Iterate through the collection of division units to obtain the bounding boxes
                    foreach(DivisionUnit unit in units)
                    {
                        // Get the bounding box of the strokes of the division unit
                        divRects[i] = unit.Strokes.GetBoundingBox();
                        
                        // The bounding box is in ink space unit. Convert them into pixel unit. 
                        ptLocation = divRects[i].Location;
                        ptSize.X = divRects[i].Width;
                        ptSize.Y = divRects[i].Height;
                        
                        // Convert the Location from Ink Space to Pixel Space
                        myInkOverlay.Renderer.InkSpaceToPixel(g, ref ptLocation);
                        
                        // Convert the Size from Ink Space to Pixel Space
                        myInkOverlay.Renderer.InkSpaceToPixel(g, ref ptSize);
                        
                        // Assign the result back to the corresponding properties
                        divRects[i].Location = ptLocation;
                        divRects[i].Width = ptSize.X;
                        divRects[i].Height = ptSize.Y;
                        
                        // Inflate the rectangle by inflate pixels in both directions
                        divRects[i].Inflate(inflate, inflate);

                        // Increment the index
                        ++i;
                    }

                } // Relinquish the Graphics object
            }
            else
            {
                // Otherwise we return null
                divRects = null;
            }

            // Return the Rectangle[] object
            return divRects;
        }

        /// <summary>
        /// Helper function that calls Ink Divider to perform the ink division.
        /// This function is called by File->Divide menu handler and strokes
        /// event handler.
        /// </summary>
        private void DivideInk()
        {
            // Ink Divider produces result based on its own Strokes object
            // Invoke the Ink Divider
            DivisionResult divResult = myInkDivider.Divide();
            
            // Call helper function to get the bounding boxes for Words
            // Rectangles are inflated by 1 pixel in all direction to 
            // avoid overlapping with stroke
            myWordBoundingBoxes = GetUnitBBoxes(divResult, InkDivisionType.Segment, 1);
            
            // Call helper function to get the bounding boxes for Lines
            // Rectangles are inflated by 3 pixels in all directions
            myLineBoundingBoxes = GetUnitBBoxes(divResult, InkDivisionType.Line, 3);
            
            // Call helper function to get the bounding boxes for Paragraphs
            // Rectangles are inflated by 5 pixels in all directions
            myParagraphBoundingBoxes = GetUnitBBoxes(divResult, InkDivisionType.Paragraph, 5);
            
            // Call helper function to get the bounding boxes for Drawings
            // The rectangles are inflated by 1 pixel in all directions
            myDrawingBoundingBoxes = GetUnitBBoxes(divResult, InkDivisionType.Drawing, 1);

            // Update the form to reflect these changes
            DrawArea.Refresh();
        }

        /// <summary>
        /// Event handler for File->Divide menu item
        /// </summary>
        /// <param name="sender">The control that raised the event.</param>
        /// <param name="e">The event arguments.</param>
        private void miDivide_Click(object sender, System.EventArgs e)
        {
            // Call DivideInk to perform the ink division
            DivideInk();
        }

        /// <summary>
        /// Event Handler from Ink Overlay's Stroke event
        /// This event is fired when a new stroke is drawn. 
        /// In this case, it is necessary to update the ink divider's 
        /// strokes collection. The event is fired even when the eraser stroke is created.
        /// The event handler must filter out the eraser strokes.
        /// </summary>
        /// <param name="sender">The control that raised the event.</param>
        /// <param name="e">The event arguments.</param>
        private void myInkOverlay_Stroke(object sender, InkCollectorStrokeEventArgs e )
        {
            // Filter out the eraser stroke.
            if(InkOverlayEditingMode.Ink == myInkOverlay.EditingMode)
            {
                // Add the new stroke to the ink divider's strokes collection
                myInkDivider.Strokes.Add(e.Stroke);
                
                if(miAutomaticLayoutAnalysis.Checked)
                {
                    // Call DivideInk
                    DivideInk();

                    // Repaint the screen to reflect the change
                    DrawArea.Refresh();
                }
            }
        }

        /// <summary>
        /// Event Handler for Ink Overlay's StrokeDeleting event. 
        /// This event is fired when a set of stroke is about to be deleted.
        /// The stroke should also be removed from the ink divider's
        /// stroke collection as well
        /// </summary>
        /// <param name="sender">The control that raised the event</param>
        /// <param name="e">The event arguments</param>
        void myInkOverlay_StrokeDeleting(object sender, InkOverlayStrokesDeletingEventArgs e)
        {
            // Remove the strokes to be deleted from the ink divider's stroke collection
            myInkDivider.Strokes.Remove(e.StrokesToDelete);
        }

        /// <summary>
        /// Event handler for Ink Overlay's StrokeDeleted event.
        /// This event is fired when the set of strokes were actually deleted.
        /// DivideInk method is called to analyze the current layout.
        /// </summary>
        /// <param name="sender">The control that raised the event</param>
        /// <param name="e">The event argument</param>
        void myInkOverlay_StrokeDeleted(object sender, System.EventArgs e)
        {
            // If automatic layout analysis is turned on, call DivideInk
            if(miAutomaticLayoutAnalysis.Checked)
            {
                // Call Divide Ink
                DivideInk();

                // Repaint the screen to reflect the change
                DrawArea.Refresh();
            }
        }
        
        /// <summary>
        /// Event handler for File->Exit menu item
        /// </summary>
        /// <param name="sender">The control that raised the event.</param>
        /// <param name="e">The event arguments.</param>
        private void miExit_Click(object sender, System.EventArgs e)
        {
            this.Close();
        }

        /// <summary>
        /// Event handler for Mode->Ink menu item
        /// </summary>
        /// <param name="sender">The control that raised the event.</param>
        /// <param name="e">The event arguments.</param>
        private void miInk_Click(object sender, System.EventArgs e)
        {
            // Turn on the inking mode
            myInkOverlay.EditingMode = InkOverlayEditingMode.Ink;

            // Update the state of the Ink and Erase menu items
            miInk.Checked = true;
            miErase.Checked = false;

            // Update the UI
            this.Refresh();
        }

        /// <summary>
        /// Event handler for Mode->Erase menu item
        /// </summary>
        /// <param name="sender">The control that raised the event.</param>
        /// <param name="e">The event argument.</param>
        private void miErase_Click(object sender, System.EventArgs e)
        {
            // Turn on the ink deletion mode
            myInkOverlay.EditingMode = InkOverlayEditingMode.Delete;

            // Update the state of the Ink and Erase menu items
            miInk.Checked = false;
            miErase.Checked = true;

            // Update the UI
            this.Refresh();
        }

        /// <summary>
        /// Event handler for Mode->AutomaticLayoutAnalysis menu item.
        /// </summary>
        /// <param name="sender">The control that raised the event.</param>
        /// <param name="e">The event argument.</param>
        private void miAutomaticLayoutAnalysis_Click(object sender, System.EventArgs e)
        {
            // Toggle the check on the menu item.
            miAutomaticLayoutAnalysis.Checked = !miAutomaticLayoutAnalysis.Checked;
            // Update the window.
            this.Refresh();
        }
    }
}

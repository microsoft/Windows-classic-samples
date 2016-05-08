// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) 2006 Microsoft Corporation. All rights reserved.

using System;
using System.Windows.Forms;
using System.Drawing;

namespace Microsoft.Samples.RssPlatform.ScreenSaver.UI
{
    /// <summary>
    /// Encapsulates the rendering of the description of an item.
    /// </summary>
    /// <typeparam name="T">The type of item that this ItemDescriptionView will draw.</typeparam>
    public class ItemDescriptionView<T> : IDisposable where T : IItem
    {
        // What to draw
        private FeedList feeds;

        // Where to draw
        private Point location;
        private Size size;
        
        private static Brush textDrawingBrush = Brushes.Black;
        private Color lineColor;
        private float lineWidth;
        private Rectangle textRect;
        private Rectangle titleRect;
        private StringFormat textFormat;
        private Color backColor;
        private Color borderColor;
        private Color foreColor;
        private Font titleFont;
        private Font textFont;
        private T displayItem;

        // The initial alpha value and the amount to change the value by each time
        private int textAlpha = 0;
        private int textAlphaDelta = 8; //4;
        private int textAlphaMax = 200;
        
        // Alpha fading timer
        private Timer fadeTimer;
        private int fadeInterval = 40;
        private int fadePauseInterval = 2000;

        public T DisplayItem { get { return displayItem; } set { displayItem = value; } }
        public Point Location { get { return location; } set { location = value; } }
        public Size Size { get { return size; } set { size = value; } }
        public Color BackColor { get { return backColor; } set { backColor = value; } }
        public Color BorderColor { get { return borderColor; } set { borderColor = value; } }
        public Color ForeColor { get { return foreColor; } set { foreColor = value; } }
        public Color LineColor { get { return lineColor; } set { lineColor = value; } }
        public Font TextFont { get { return textFont; } set { textFont = value; } }
        public Font TitleFont { get { return titleFont; } set { titleFont = value; } }
        public float LineWidth { get { return lineWidth; } set { lineWidth = value; } }
        public int FadeInterval { get { return fadeInterval; } set { fadeInterval = value; } }
        public int FadePauseInterval { get { return fadePauseInterval; } set { fadePauseInterval = value; } }

        public event EventHandler FadingComplete;
        public event EventHandler FadingTick
        {
            add { fadeTimer.Tick += value; }
            remove { fadeTimer.Tick -= value; }
        }

        /// <summary>
        /// Create a new ItemDescriptionView connected to <paramref name="listView"/>.
        /// </summary>
        /// <param name="listView"></param>
        public ItemDescriptionView(FeedList feeds)
        {
            this.feeds = feeds;
            fadeTimer = new Timer();
            fadeTimer.Tick += new EventHandler(fadeTimer_Tick);
            fadeTimer.Enabled = true;
            fadeTimer.Start();
        }

        /// <summary>
        /// Restart the fade effect
        /// </summary>
        public void Reset()
        {
            textAlpha = 0;
            textAlphaDelta = Math.Abs(textAlphaDelta);
            fadeTimer_Tick(this, new EventArgs());
        }

        public void Paint(PaintEventArgs e)
        {
            // Change the graphics settings to draw clear text
            e.Graphics.SmoothingMode = System.Drawing.Drawing2D.SmoothingMode.AntiAlias;
            e.Graphics.TextRenderingHint = System.Drawing.Text.TextRenderingHint.ClearTypeGridFit;

            DrawBackground(e.Graphics); 

            int lineVerticalBuffer = Size.Height / 50;
            //DrawTwoLines(e.Graphics,lineVerticalBuffer);

            // Draw the text of the article
            textFormat = new StringFormat(StringFormatFlags.LineLimit);
            textFormat.Alignment = StringAlignment.Near;
            textFormat.LineAlignment = StringAlignment.Near;
            textFormat.Trimming = StringTrimming.EllipsisWord;

            int textVerticalBuffer = 4 * lineVerticalBuffer;
            int textHorizontalBuffer = 20;

            titleRect = new Rectangle(Location.X + textHorizontalBuffer,Location.Y + textVerticalBuffer,
                Size.Width - 2*textHorizontalBuffer, titleFont.Height);
            textRect = new Rectangle(Location.X + textHorizontalBuffer, Location.Y + textVerticalBuffer + titleFont.Height + textVerticalBuffer, 
                Size.Width - 2*textHorizontalBuffer, Size.Height - 3*textVerticalBuffer - titleFont.Height);

            //using (Brush backBrush1 = new SolidBrush(Color.Blue))
            //using (Brush backBrush2 = new SolidBrush(Color.BurlyWood))
            //{
            //    e.Graphics.FillRectangle(backBrush1, titleRect);
            //    e.Graphics.FillRectangle(backBrush2, textRect);
            //}
            
            using (Brush textBrush = new SolidBrush(Color.FromArgb(textAlpha, ForeColor)))
            {
                string title = feeds.CurrentItem.Title;
                e.Graphics.DrawString(title, TitleFont, textBrush, titleRect, textFormat);

                string text = feeds.CurrentItem.Description;
                e.Graphics.DrawString(text, TextFont, textBrush, textRect, textFormat);
            }
        }

        /// <summary>
        /// Draws a box and border ontop of which the text of the items can be drawn.
        /// </summary>
        /// <param name="g">The Graphics object to draw onto</param>
        private void DrawBackground(Graphics g)
        {
            using (Brush backBrush = new SolidBrush(BackColor))
            using (Pen borderPen = new Pen(BorderColor, 4))
            {
                //g.FillRectangle(backBrush, new Rectangle(Location.X + 4, Location.Y + 4, Size.Width - 8, Size.Height - 8));
                g.FillRectangle(backBrush, new Rectangle(Location.X, Location.Y, Size.Width, Size.Height));
                //g.DrawRectangle(borderPen, new Rectangle(Location, Size));
            }
        }

        private void DrawTwoLines(Graphics g,int lineVerticalBuffer)
        {
            // Determine the placement of the lines that will be placed 
            // above and below the text
            float lineLeftX = Size.Width / 4;
            float lineRightX = 3 * Size.Width / 4;
            float lineTopY = Location.Y + lineVerticalBuffer;
            float lineBottomY = Location.Y + Size.Height - lineVerticalBuffer;

            // Draw the two lines
            using (Pen linePen = new Pen(lineColor, lineWidth))
            {
                g.DrawLine(linePen, Location.X + lineLeftX, lineTopY, Location.X + lineRightX, lineTopY);
                g.DrawLine(linePen, Location.X + lineLeftX, lineBottomY, Location.X + lineRightX, lineBottomY);
            }
        }

        private void fadeTimer_Tick(object sender, EventArgs e)
        {
            // Change the alpha value of the text being drawn
            // Moves up until it reaches textAlphaMax and then moves down
            // Moves to the next article when it gets back to zero
            fadeTimer.Interval = fadeInterval;
            textAlpha += textAlphaDelta;
            if (textAlpha >= textAlphaMax)
            {
                fadeTimer.Interval = fadePauseInterval;
                textAlphaDelta *= -1;
            }
            else if (textAlpha <= 0)
            {
                FadingComplete(this, new EventArgs());
                textAlpha = 0;
                textAlphaDelta *= -1;
            }
        }

        /// <summary>
        /// Dispose all disposable fields
        /// </summary>
        public void Dispose()
        {
            Dispose(true);
            GC.SuppressFinalize(this);
        }

        protected virtual void Dispose(bool disposing)
        {
            if (disposing)
            {
                if (fadeTimer != null)
                {
                    fadeTimer.Dispose();
                    fadeTimer = null;
                }
                if (textFormat != null)
                {
                    textFormat.Dispose();
                    textFormat = null;
                }
            }
        }
    }
}

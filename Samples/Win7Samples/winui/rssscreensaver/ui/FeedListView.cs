// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) 2006 Microsoft Corporation. All rights reserved.

using System;
using System.Drawing;
using System.Windows.Forms;
using System.Collections.Generic;

namespace Microsoft.Samples.RssPlatform.ScreenSaver.UI
{
    /// <summary>
    /// Encapsulates the rendering of a list of items.  Each item's description is shown in a list, and a single item is selected.
    /// </summary>
    /// <typeparam name="T">The type of item that this ItemListView will draw.</typeparam>
    public class FeedListView<T> : IDisposable where T : IItem
    {
        private const float percentOfArticleDisplayBoxToFillWithText = 0.5f;
        private const float percentOfFontHeightForSelectionBox = 1.5f;
        private const int padding = 20;

        // Where to draw
        private Point location;
        private Size size;

        private FeedList feeds;

        private string title;
        private Font feedFont;
        private Font titleFont;
        private Color backColor;
        private Color borderColor;
        private Color foreColor;
        private Color titleBackColor;
        private Color titleForeColor;
        private Color selectedForeColor;
        private Color selectedBackColor;
        private float feedFontHeight;
        // The maximum number of feeds that will be displayed (defines viewport)
        private int maxFeedsToShow;
        // The current first index of the feed visible in viewport
        private int firstFeedToShow = 0;
        // The minimum number of feeds that will be displayed
        // If there are fewer than this then there will be blank space in the display
        private int minFeedsToShow;

        private int NumFeeds { get { return Math.Min(feeds.Count, maxFeedsToShow); } }
        private int NumFeedRows { get { return Math.Max(NumFeeds, minFeedsToShow); } }

        public Point Location { get { return location; } set { location = value; } }
        public Size Size { get { return size; } set { size = value; } }

        public Color ForeColor { get { return foreColor; } set { foreColor = value; } }
        public Color BackColor { get { return backColor; } set { backColor = value; } }
        public Color BorderColor { get { return borderColor; } set { borderColor = value; } }
        public Color TitleForeColor { get { return titleForeColor; } set { titleForeColor = value; } }
        public Color TitleBackColor { get { return titleBackColor; } set { titleBackColor = value; } }
        public Color SelectedForeColor { get { return selectedForeColor; } set { selectedForeColor = value; } }
        public Color SelectedBackColor { get { return selectedBackColor; } set { selectedBackColor = value; } }
        public int MaxFeedsToShow { get { return maxFeedsToShow; } set { maxFeedsToShow = value; } }
        public int MinFeedsToShow { get { return minFeedsToShow; } set { minFeedsToShow = value; } }

        public int RowHeight
        {
            get
            {
                // There is one row for each item plus 2 rows for the title.
                return size.Height / (NumFeedRows + 2);
            }
        }

        public Font ItemFont
        {
            get
            {
                // Choose a font for each of the item titles that will fit all numItems 
                // of them (plus some slack for the title) in the control 
                feedFontHeight = (float)(percentOfArticleDisplayBoxToFillWithText * RowHeight);
                if (feedFont == null || feedFont.Size != feedFontHeight)
                {
                    feedFont = new Font("Microsoft Sans Serif", feedFontHeight, GraphicsUnit.Pixel);
                }
                return feedFont;
            }
        }

        public Font TitleFont
        {
            get
            {
                // Choose a font for the title text.
                // This font will be twice as big as the ItemFont
                float titleFontHeight = (float)(percentOfArticleDisplayBoxToFillWithText * 2 * RowHeight);
                if (titleFont == null || titleFont.Size != titleFontHeight)
                {
                    titleFont = new Font("Microsoft Sans Serif", titleFontHeight, GraphicsUnit.Pixel);
                }
                return titleFont;
            }
        }

        public FeedListView(string title, FeedList feeds)
        {
            if (feeds == null)
                throw new ArgumentNullException("feeds");

            this.feeds = feeds;
            this.title = title;
        }

        public void Paint(PaintEventArgs args)
        {
            Graphics g = args.Graphics;

            // Settings to make the text drawing look nice
            g.SmoothingMode = System.Drawing.Drawing2D.SmoothingMode.AntiAlias;
            g.TextRenderingHint = System.Drawing.Text.TextRenderingHint.ClearTypeGridFit;

            DrawBackground(g);

            // Determine the viewport for display
            int index = feeds.CurrentFeedIndex + 1;
            if (firstFeedToShow < index - maxFeedsToShow)   // selection at bottom
                firstFeedToShow = Math.Max(index - maxFeedsToShow, 0);
            else if (firstFeedToShow >= index)              // selection at top
                firstFeedToShow = feeds.CurrentFeedIndex;

            index = firstFeedToShow;

            // Draw each feed's description
            int row = 0;
            while (row < maxFeedsToShow && index < feeds.Count)
            {
                DrawFeedTitle(g, row++, feeds.GetFeed(index++));
            }

            // Draw the title text
            DrawTitle(g);
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
                g.FillRectangle(backBrush, new Rectangle(Location.X + 4, Location.Y + 4, Size.Width - 8, Size.Height - 8));
                g.DrawRectangle(borderPen, new Rectangle(Location, Size));
            }
        }

        /// <summary>
        /// Draws the title of the feed with the given index.
        /// </summary>
        /// <param name="g">The Graphics object to draw onto</param>
        /// <param name="index">The index of the item in the list</param>
        private void DrawFeedTitle(Graphics g, int row, Rss.RssFeed rssFeed)
        {
            // Set formatting and layout
            StringFormat stringFormat = new StringFormat(StringFormatFlags.LineLimit);
            stringFormat.Trimming = StringTrimming.EllipsisCharacter;
            Rectangle feedTitleRect = new Rectangle(Location.X + padding, Location.Y + (int)(row * RowHeight) + padding, Size.Width - (2 * padding), (int)(percentOfFontHeightForSelectionBox * feedFontHeight));

            // Select color and draw border if current index is selected
            Color textBrushColor = ForeColor;
            if (rssFeed == feeds.CurrentFeed)
            {
                textBrushColor = SelectedForeColor;
                using (Brush backBrush = new SolidBrush(SelectedBackColor))
                {
                    g.FillRectangle(backBrush, feedTitleRect);
                }
            }

            // Draw the title of the feed
            string textToDraw = rssFeed.Title + " (" + rssFeed.LastWriteTime + ")";
            using (Brush textBrush = new SolidBrush(textBrushColor))
            {
                g.DrawString(textToDraw, ItemFont, textBrush, feedTitleRect, stringFormat);
            }
        }

        /// <summary>
        /// Draws a title bar.
        /// </summary>
        /// <param name="g">The Graphics object to draw onto</param>
        private void DrawTitle(Graphics g)
        {
            Point titleLocation = new Point(Location.X + padding, Location.Y + Size.Height - (RowHeight) - padding);
            Size titleSize = new Size(Size.Width - (2 * padding), 2 * RowHeight);
            Rectangle titleRectangle = new Rectangle(titleLocation, titleSize);

            // Draw the title box and the selected item box
            using (Brush titleBackBrush = new SolidBrush(TitleBackColor))
            {
                g.FillRectangle(titleBackBrush, titleRectangle);
            }

            // Draw the title text
            StringFormat titleFormat = new StringFormat(StringFormatFlags.LineLimit);
            titleFormat.Alignment = StringAlignment.Far;
            titleFormat.Trimming = StringTrimming.EllipsisCharacter;
            using (Brush titleBrush = new SolidBrush(TitleForeColor))
            {
                g.DrawString(title, TitleFont, titleBrush, titleRectangle, titleFormat);
            }
        }

        /// <summary>
        /// Dispose all disposable fields
        /// </summary>
        public void Dispose()
        {
            this.Dispose(true);
            GC.SuppressFinalize(this);
        }

        protected virtual void Dispose(bool disposing)
        {
            if (disposing)
            {
                if (feedFont != null)
                {
                    feedFont.Dispose();
                    feedFont = null;
                }
                if (titleFont != null)
                {
                    titleFont.Dispose();
                    titleFont = null;
                }
            }
        }
    }
}
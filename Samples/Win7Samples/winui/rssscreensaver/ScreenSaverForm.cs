// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) 2006 Microsoft Corporation. All rights reserved.

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Drawing;
using System.IO;
using System.Windows.Forms;
using Microsoft.Samples.RssPlatform.ScreenSaver.UI;
using Microsoft.Samples.RssPlatform.ScreenSaver.Rss;

namespace Microsoft.Samples.RssPlatform.ScreenSaver
{
    partial class ScreenSaverForm : Form
    {
        // The feeds to display articles and conclosures from
        private FeedList feedlist;

        // Objects for displaying RSS contents
        private ItemListView<RssItem> rssItemsView;
        private ItemDescriptionView<RssItem> rssDescriptionView;
        private FeedListView<RssFeed> rssFeedsView;

        // Keep track of whether the screensaver has become active.
        private bool isActive;

        // Keep track of the location of the mouse
        private Point mouseLocation;

        // Keep track if FeedView and ItemsView should be shown
        private bool isFeedViewShown;
        private bool isItemsViewShown;

        internal static bool IsResourcePictureUrl(string url)
        {
            return (url.Equals("res:SSaverBackground.jpg", StringComparison.OrdinalIgnoreCase) ||
                url.Equals("res:SSaverBackground2.jpg", StringComparison.OrdinalIgnoreCase));
        }

        internal static bool IsResourcePictureFilename(string filename)
        {
            return (filename.Equals("@SSaverBackground.jpg", StringComparison.OrdinalIgnoreCase) ||
                filename.Equals("@SSaverBackground2.jpg", StringComparison.OrdinalIgnoreCase));
        }

        public ScreenSaverForm()
            : this(false)
        {
        }

        public ScreenSaverForm(bool debugmode)
        {
            InitializeComponent();

            isFeedViewShown = debugmode;
            isItemsViewShown = debugmode;

            // In debug mode, allow minimize after breakpoint 
            if (debugmode) this.FormBorderStyle = FormBorderStyle.Sizable;

            LoadFeedList();     // may throw ApplicationException

            SetupScreenSaver();

            // Initialize the ItemListView to display the list of items in the 
            // current feed. It is placed on the left side of the screen.            
            rssItemsView = new ItemListView<RssItem>(feedlist);
            InitializeRssItemsView();

            // Initialize the ItemDescriptionView to display the description of the 
            // RssItem.  It is placed on the bottom of the screen.
            rssDescriptionView = new ItemDescriptionView<RssItem>(feedlist);
            InitializeRssDescriptionView();

            // Initialize the FeedListView to display the list of feeds. It is placed
            // at the top of the screen.
            rssFeedsView = new FeedListView<RssFeed>("Picture Feeds", feedlist);
            InitializeRssFeedView();

            InitializeListRefresh();
        }

        /// <summary>
        /// Set up the main form as a full screen screensaver.
        /// </summary>
        private void SetupScreenSaver()
        {
            // Use double buffering to improve drawing performance
            this.SetStyle(ControlStyles.OptimizedDoubleBuffer | ControlStyles.UserPaint | ControlStyles.AllPaintingInWmPaint, true);
            // Capture the mouse
            this.Capture = true;

            // Set the application to full screen mode and hide the mouse
            Cursor.Hide();
            Bounds = Screen.PrimaryScreen.Bounds;
            WindowState = FormWindowState.Maximized;
            TopMost = true;

            ShowInTaskbar = false;
            DoubleBuffered = true;
            BackgroundImageLayout = ImageLayout.Stretch;
        }

        private void LoadFeedList()
        {
            feedlist = new FeedList();

            try
            {
                feedlist.Load();
            }
            catch (FileNotFoundException ex)
            {
                // This error is most likely caused by a missing interop assembly
                throw new NoInteropException(ex.ToString(), ex);
            }
        }

        /// <summary>
        /// Initialize display properties of the rssView.
        /// </summary>
        private void InitializeRssItemsView()
        {
            rssItemsView.BackColor = Color.FromArgb(120, 240, 234, 232);
            rssItemsView.BorderColor = Color.White;
            rssItemsView.ForeColor = Color.FromArgb(255, 40, 40, 40);
            rssItemsView.SelectedBackColor = Color.FromArgb(200, 105, 61, 76);
            rssItemsView.SelectedForeColor = Color.FromArgb(255, 204, 184, 163);
            rssItemsView.TitleBackColor = Color.Empty;
            rssItemsView.TitleForeColor = Color.FromArgb(255, 240, 234, 232);
            rssItemsView.MaxItemsToShow = 20;
            rssItemsView.MinItemsToShow = 10;
            rssItemsView.Location = new Point(Width / 10, Height / 5);
            rssItemsView.Size = new Size(Width / 2, Height / 3);
        }

        /// <summary>
        /// Initialize display properties of the rssDescriptionView.
        /// </summary>
        private void InitializeRssDescriptionView()
        {
            // Almost white background / reddish border
            rssDescriptionView.BackColor = Color.FromArgb(220, 240, 234, 232);
            rssDescriptionView.ForeColor = Color.Black;
            rssDescriptionView.BorderColor = Color.FromArgb(255, 204, 184, 163);

            rssDescriptionView.TitleFont = rssItemsView.TitleFont;
            Font textFont = new Font(rssItemsView.TitleFont, FontStyle.Regular);
            rssDescriptionView.TextFont = textFont;
            rssDescriptionView.LineColor = Color.FromArgb(120, 240, 234, 232);
            rssDescriptionView.LineWidth = 2f;
            rssDescriptionView.FadingTick += new EventHandler(FadeTimer_Tick);
            rssDescriptionView.FadingComplete += new EventHandler(FadeTimer_Complete);
            rssDescriptionView.FadeInterval = 60;
            rssDescriptionView.FadePauseInterval = Properties.Settings.Default.FadePauseInterval * 1000 + 10;
            rssDescriptionView.Location = new Point((Width / 2) - Width / 20, (4 * Height / 5) - Height / 20);
            rssDescriptionView.Size = new Size(Width / 2, Height / 5);
        }

        private void InitializeRssFeedView()
        {
            rssFeedsView.BackColor = Color.FromArgb(120, 240, 234, 232);
            rssFeedsView.BorderColor = Color.White;
            rssFeedsView.ForeColor = Color.FromArgb(255, 40, 40, 40);
            rssFeedsView.SelectedBackColor = Color.FromArgb(200, 105, 61, 76);
            rssFeedsView.SelectedForeColor = Color.FromArgb(255, 204, 184, 163);
            rssFeedsView.TitleBackColor = Color.Empty;
            rssFeedsView.TitleForeColor = Color.FromArgb(255, 240, 234, 232);
            rssFeedsView.MaxFeedsToShow = 10;
            rssFeedsView.MinFeedsToShow = 4;
            rssFeedsView.Location = new Point(10, 10);
            rssFeedsView.Size = new Size(Width / 3, Height / 7);
        }

        private void InitializeListRefresh()
        {
            checkCflTimer.Interval = 15000; // every 15 seconds
            checkCflTimer.Tick += new EventHandler(checkCflTimer_Tick);
            checkCflTimer.Enabled = true;
        }

        private void ScreenSaverForm_MouseMove(object sender, MouseEventArgs e)
        {
            // Set IsActive and MouseLocation only the first time this event is called.
            if (!isActive)
            {
                mouseLocation = MousePosition;
                isActive = true;
            }
            else
            {
                // If the mouse has moved significantly since first call, close.
                if ((Math.Abs(MousePosition.X - mouseLocation.X) > 10) ||
                    (Math.Abs(MousePosition.Y - mouseLocation.Y) > 10))
                {
                    Close();
                }
            }
        }

        private void ScreenSaverForm_KeyDown(object sender, KeyEventArgs e)
        {
            if (e.KeyCode == (Keys.RButton | Keys.ShiftKey))
            { ;} // capture the Alt keypress
            else if (e.KeyCode == Keys.F && e.Alt)
                isFeedViewShown = !isFeedViewShown;
            else if (e.KeyCode == Keys.I && e.Alt)
                isItemsViewShown = !isItemsViewShown;
            else if (e.KeyCode == Keys.Down)
            { 
                feedlist.MoveNext();
                rssDescriptionView.Reset();
            }
            else if (e.KeyCode == Keys.Up)
            {
                feedlist.MovePrevious();
                rssDescriptionView.Reset();
            }
            else //if(e.KeyCode == Keys.Escape)
                Close();

            this.Refresh();
        }

        private void ScreenSaverForm_MouseDown(object sender, MouseEventArgs e)
        {
            Close();
        }

        protected override void OnPaintBackground(PaintEventArgs e)
        {
            // Draw the current background image stretched to fill the full screen
            Image image = feedlist.CurrentImage;
            if (image != null)
            {
                if (image.Size.Height < Size.Height && image.Size.Width < Size.Width)
                    e.Graphics.DrawImage(image, (Size.Width - image.Size.Width) / 2,
                        (Size.Height - image.Size.Height) / 2, image.Size.Width, image.Size.Height);
                else
                {
                    // Keep aspect ratio
                    double aspect = Math.Min((double)Size.Height / image.Size.Height,
                        (double)Size.Width / image.Size.Width);
                    Rectangle rect = new Rectangle();
                    rect.Width = (int)((double)image.Size.Width * aspect);
                    rect.Height = (int)((double)image.Size.Height * aspect);
                    rect.X = (Size.Width - rect.Width) / 2;
                    rect.Y = (Size.Height - rect.Height) / 2;
                    e.Graphics.DrawImage(image, rect);
                }
            }
        }

        protected override void OnPaint(PaintEventArgs e)
        {
            if (isItemsViewShown)
                if (e.ClipRectangle.IntersectsWith(new Rectangle(rssItemsView.Location, rssItemsView.Size)))
                    rssItemsView.Paint(e);

            if (isFeedViewShown)
                if (e.ClipRectangle.IntersectsWith(new Rectangle(rssFeedsView.Location, rssFeedsView.Size)))
                    rssFeedsView.Paint(e);

            rssDescriptionView.Paint(e);
        }

        void FadeTimer_Tick(object sender, EventArgs e)
        {
            //this.Refresh();
            Rectangle r = new Rectangle(rssDescriptionView.Location, rssDescriptionView.Size);
            this.Invalidate(r);
            if (isItemsViewShown)
            {
                r = new Rectangle(rssItemsView.Location, rssItemsView.Size);
                this.Invalidate(r);
            }
            if (isFeedViewShown)
            {
                r = new Rectangle(rssFeedsView.Location, rssFeedsView.Size);
                this.Invalidate(r);
            }
        }

        void FadeTimer_Complete(object sender, EventArgs e)
        {
            feedlist.MoveNext();
            this.Refresh();
            Activate();        // recapture keyboard input, in case it has moved elsewhere
        }

        void checkCflTimer_Tick(object sender, EventArgs e)
        {
            if (feedlist != null)
                feedlist.Refresh();
        }

    }

}

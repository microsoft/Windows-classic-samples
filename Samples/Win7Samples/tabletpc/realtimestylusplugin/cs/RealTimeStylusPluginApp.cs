//-------------------------------------------------------------------------
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//  File: RealTimeStylusPlugin.cs
//  Real time stylus plugin sample.
//
//  This sample application demonstrates several different types of 
//  Real time stylus plugins:
//
// -PacketFilterPlugin: demonstrates packet modification by
//  constraining all x,y packet data within a rectangular area.
// -CustomDynamicRendererPlugin:  demonstrates custom dynamic
//  rendering by drawing a small circle around each x,y.
// -GestureRecognizer:  plugin provided by the Microsoft Tablet PC 
//  API, which recognizes application gestures.
// -DynamicRenderer:  plugin provided by the Microsoft Tablet PC
//  API, which renders packet data as it is collected.
//
//  In addition, this sample provides a user interface that
//  enables the user to add/remove and change the order of each
//  plugin in the notification chain.
//  
//
//  The features used are: RealTimeStylus, GestureRecognizer, 
//  DynamicRenderer, IStylusAsyncPlugin, and IStylusSyncPlugin.
//
//--------------------------------------------------------------------------

using System;
using System.Diagnostics;
using System.Drawing;
using System.Windows.Forms;

// The Microsoft Tablet PC namespaces
using Microsoft.StylusInput;
using Microsoft.StylusInput.PluginData;
using Microsoft.Ink;

// Contains the custom real time stylus plugins used by this
// sample application
using RealTimeStylusPlugins;

namespace RealTimeStylusPluginApp
{

    /// <summary>
    /// Helper used to store information about a given plugin.
    /// </summary>
    public struct PluginListItem
    {
        /// <summary>
        /// The real time stylus plugin associated with this class.
        /// </summary>
        public IStylusSyncPlugin Plugin;

        /// <summary>
        /// A string containing the description of the plugin.
        /// </summary>
        public String Description;

        /// <summary>
        /// Public constructor
        /// </summary>
        /// <param name="Plugin">The plugin</param>
        /// <param name="description">Description of the plugin</param>
        public PluginListItem(IStylusSyncPlugin stylusPlugin, String pluginDescription)
        {
            Plugin = stylusPlugin;
            Description = pluginDescription;
        }

        /// <summary>
        /// The description of this plugin
        /// </summary>
        /// <returns>String that describes this plugin</returns>
        override public String ToString()
        {
            return Description;
        }
    }

    /// <summary>
    /// The RealTimeStylusPlugin Sample Application form class.
    /// </summary>
    public class RealTimeStylusPluginApp : System.Windows.Forms.Form, IStylusAsyncPlugin
    {

        // Declare the real time stylus.
        private RealTimeStylus myRealTimeStylus;

        // Declare the graphics object passed to the custom dynamic
        // renderer
        private Graphics customDynamicRendererGraphics;

        #region Standard Template Code
        private System.Windows.Forms.GroupBox gbTestArea;
        private System.Windows.Forms.Button btnMoveUp;
        private System.Windows.Forms.Button btnMoveDown;
        private System.Windows.Forms.CheckedListBox chklbPlugins;
        private System.Windows.Forms.StatusBar sbGesture;
        private System.Windows.Forms.MenuItem miAction;
        private System.Windows.Forms.MenuItem miClear;
        private System.Windows.Forms.MenuItem miExit;
        private System.Windows.Forms.TextBox txtPluginDescription;
        private System.Windows.Forms.Label lblPluginList;
        private System.Windows.Forms.MainMenu miMain;
        private System.Windows.Forms.MenuItem miSeparator;
        private System.Windows.Forms.Panel pnlPlugins;
        #endregion
        private System.ComponentModel.IContainer components;

        public RealTimeStylusPluginApp()
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

            // Dispose the graphics objects associated with the
            // custom dynamic renderer.
            customDynamicRendererGraphics.Dispose();
        }
        #endregion

        #region Windows Form Designer generated code
        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.components = new System.ComponentModel.Container();
            this.gbTestArea = new System.Windows.Forms.GroupBox();
            this.chklbPlugins = new System.Windows.Forms.CheckedListBox();
            this.btnMoveUp = new System.Windows.Forms.Button();
            this.btnMoveDown = new System.Windows.Forms.Button();
            this.txtPluginDescription = new System.Windows.Forms.TextBox();
            this.lblPluginList = new System.Windows.Forms.Label();
            this.sbGesture = new System.Windows.Forms.StatusBar();
            this.miMain = new System.Windows.Forms.MainMenu(this.components);
            this.miAction = new System.Windows.Forms.MenuItem();
            this.miClear = new System.Windows.Forms.MenuItem();
            this.miSeparator = new System.Windows.Forms.MenuItem();
            this.miExit = new System.Windows.Forms.MenuItem();
            this.pnlPlugins = new System.Windows.Forms.Panel();
            this.pnlPlugins.SuspendLayout();
            this.SuspendLayout();
            // 
            // gbTestArea
            // 
            this.gbTestArea.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                        | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.gbTestArea.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.gbTestArea.Location = new System.Drawing.Point(7, 7);
            this.gbTestArea.Name = "gbTestArea";
            this.gbTestArea.Size = new System.Drawing.Size(409, 345);
            this.gbTestArea.TabIndex = 0;
            this.gbTestArea.TabStop = false;
            this.gbTestArea.Text = "Test Area";
            // 
            // chklbPlugins
            // 
            this.chklbPlugins.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                        | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.chklbPlugins.CheckOnClick = true;
            this.chklbPlugins.Location = new System.Drawing.Point(9, 96);
            this.chklbPlugins.Name = "chklbPlugins";
            this.chklbPlugins.Size = new System.Drawing.Size(171, 184);
            this.chklbPlugins.TabIndex = 6;
            this.chklbPlugins.ItemCheck += new System.Windows.Forms.ItemCheckEventHandler(this.chklbPlugins_ItemCheck);
            this.chklbPlugins.MouseDown += new System.Windows.Forms.MouseEventHandler(this.chklbPlugins_MouseDown);
            // 
            // btnMoveUp
            // 
            this.btnMoveUp.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.btnMoveUp.Location = new System.Drawing.Point(9, 315);
            this.btnMoveUp.Name = "btnMoveUp";
            this.btnMoveUp.Size = new System.Drawing.Size(78, 25);
            this.btnMoveUp.TabIndex = 7;
            this.btnMoveUp.Text = "Move Up";
            this.btnMoveUp.Click += new System.EventHandler(this.btnMove_Click);
            // 
            // btnMoveDown
            // 
            this.btnMoveDown.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.btnMoveDown.Location = new System.Drawing.Point(103, 315);
            this.btnMoveDown.Name = "btnMoveDown";
            this.btnMoveDown.Size = new System.Drawing.Size(77, 25);
            this.btnMoveDown.TabIndex = 8;
            this.btnMoveDown.Text = "Move Down";
            this.btnMoveDown.Click += new System.EventHandler(this.btnMove_Click);
            // 
            // txtPluginDescription
            // 
            this.txtPluginDescription.BorderStyle = System.Windows.Forms.BorderStyle.None;
            this.txtPluginDescription.Location = new System.Drawing.Point(9, 20);
            this.txtPluginDescription.Multiline = true;
            this.txtPluginDescription.Name = "txtPluginDescription";
            this.txtPluginDescription.ReadOnly = true;
            this.txtPluginDescription.Size = new System.Drawing.Size(171, 76);
            this.txtPluginDescription.TabIndex = 9;
            this.txtPluginDescription.Text = "Packets are processed by the plugins in the order shown in the list.  Also, the p" +
                "lugins can be turned off by unchecking/checking the items.";
            // 
            // lblPluginList
            // 
            this.lblPluginList.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.lblPluginList.Location = new System.Drawing.Point(1, 3);
            this.lblPluginList.Name = "lblPluginList";
            this.lblPluginList.Size = new System.Drawing.Size(145, 17);
            this.lblPluginList.TabIndex = 10;
            this.lblPluginList.Text = "Plugin List";
            // 
            // sbGesture
            // 
            this.sbGesture.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.sbGesture.Dock = System.Windows.Forms.DockStyle.None;
            this.sbGesture.Location = new System.Drawing.Point(0, 353);
            this.sbGesture.Name = "sbGesture";
            this.sbGesture.Size = new System.Drawing.Size(614, 24);
            this.sbGesture.SizingGrip = false;
            this.sbGesture.TabIndex = 6;
            // 
            // miMain
            // 
            this.miMain.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
            this.miAction});
            // 
            // miAction
            // 
            this.miAction.Index = 0;
            this.miAction.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
            this.miClear,
            this.miSeparator,
            this.miExit});
            this.miAction.Text = "&Action";
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
            this.miExit.Text = "&Exit";
            this.miExit.Click += new System.EventHandler(this.miExit_Click);
            // 
            // pnlPlugins
            // 
            this.pnlPlugins.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.pnlPlugins.BackColor = System.Drawing.SystemColors.Control;
            this.pnlPlugins.Controls.Add(this.lblPluginList);
            this.pnlPlugins.Controls.Add(this.txtPluginDescription);
            this.pnlPlugins.Controls.Add(this.btnMoveDown);
            this.pnlPlugins.Controls.Add(this.btnMoveUp);
            this.pnlPlugins.Controls.Add(this.chklbPlugins);
            this.pnlPlugins.Location = new System.Drawing.Point(429, 7);
            this.pnlPlugins.Name = "pnlPlugins";
            this.pnlPlugins.Size = new System.Drawing.Size(180, 350);
            this.pnlPlugins.TabIndex = 7;
            // 
            // RealTimeStylusPluginApp
            // 
            this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
            this.ClientSize = new System.Drawing.Size(614, 377);
            this.Controls.Add(this.pnlPlugins);
            this.Controls.Add(this.gbTestArea);
            this.Controls.Add(this.sbGesture);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedSingle;
            this.MaximizeBox = false;
            this.Menu = this.miMain;
            this.MinimumSize = new System.Drawing.Size(367, 295);
            this.Name = "RealTimeStylusPluginApp";
            this.Text = "Real Time Stylus Plugin Sample Application";
            this.Load += new System.EventHandler(this.RealTimeStylusPlugin_Load);
            this.pnlPlugins.ResumeLayout(false);
            this.pnlPlugins.PerformLayout();
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
            Application.Run(new RealTimeStylusPluginApp());
        }
        #endregion

        /// <summary>
        /// Event Handler from Form Load Event
        /// Setup the real time stylus for collection.
        /// </summary>
        /// <param name="sender">The control that raised the event.</param>
        /// <param name="e">The event arguments.</param>
        private void RealTimeStylusPlugin_Load(object sender, System.EventArgs e)
        {
            
            // Create the real time stylus used to receive stylus notifications
            myRealTimeStylus = new RealTimeStylus(gbTestArea, true);

            // Store the Graphics object associated with the drawing area.
            customDynamicRendererGraphics = gbTestArea.CreateGraphics();

            // Create the plugins.  Note that since these are 
            // synchronous plugins, notification will occur on the 
            // pen thread.
            // 
            // -PacketFilterPlugin: demonstrates packet modification by
            //  constraining all x,y packet data within a rectangular area.
            // -CustomDynamicRendererPlugin:  demonstrates custom dynamic
            //  rendering by drawing a small circle around each x,y.
            // -GestureRecognizer:  plugin provided by the Tablet PC Platform
            //  API, which recognizes application gestures.
            // -DynamicRenderer:  plugin provided by Tablet PC Platform API,
            // which renders packet data as it is collected.

            // Create the PacketFilterPlugin 
            // Note that this custom plugin takes a rectangle in
            // ink space coordinates (.01mm = 1 ink space unit), which
            // specifies the filter region.  This sample uses a rectangle
            // that is centered over the drawing area.
            float right  = ((float)gbTestArea.Width*2540.0F)/(float)customDynamicRendererGraphics.DpiX;
            float bottom = ((float)gbTestArea.Height*2540.0F)/(float)customDynamicRendererGraphics.DpiY;
            Rectangle filterRect = new Rectangle((int)Math.Round(.15F*right), 
                                                 (int)Math.Round(.15F*bottom), 
                                                 (int)Math.Round(.7F*right), 
                                                 (int)Math.Round(.7F*bottom));
            IStylusSyncPlugin filterPlugin = new PacketFilterPlugin(filterRect);
            chklbPlugins.Items.Add(new PluginListItem(filterPlugin,"PacketFilter"));

            // Create the CustomDynamicRendererPlugin
            IStylusSyncPlugin rendererPlugin = new CustomDynamicRendererPlugin(customDynamicRendererGraphics);
            chklbPlugins.Items.Add(new PluginListItem(rendererPlugin,"CustomDynamicRenderer"));
            
            // Attempt to create the GestureRecognizer plugin.
            // An exception will occur if no recognizers are available.
            // In this case, the sample proceeds, but does not add the
            // gesture recognizer into the list of available plugins.
            GestureRecognizer gr = null;
            try
            {
                gr = new GestureRecognizer();
                ApplicationGesture [] gestures = { ApplicationGesture.AllGestures };
                gr.EnableGestures(gestures);
                chklbPlugins.Items.Add(new PluginListItem(gr,"GestureRecognizer"));
            }
            catch
            {
            }

            // Create the dynamic renderer used to render the stroke that is
            //  currently being collected
            DynamicRenderer dr = new DynamicRenderer(gbTestArea);
            chklbPlugins.Items.Add(new PluginListItem(dr,"DynamicRenderer"));

            // Enable all plugins
            for(int i = 0; i < chklbPlugins.Items.Count; i++)
            {
                chklbPlugins.SetItemChecked(i,true);
            } 
            
            // Add this form to the collection of asynchronous plugins.
            // The CustomStylusDataAdded notification will be used to 
            // update the form's UI when an application gesture occurs.
            // Since this is an asynchronous plugin, notification will
            // occur on the UI thread.
            myRealTimeStylus.AsyncPluginCollection.Add(this);

            // Enable the RealTimeStylus, GestureRecognizer, and DynamicRenderer
            myRealTimeStylus.Enabled = true; 
            if (gr != null)
            {
                gr.Enabled = true;
            }
            dr.Enabled = true;
        }

        /// <summary>
        /// Private field used to track the last MouseDown event information
        /// for use in the chklbPlugins_ItemCheck handler. 
        /// </summary>
        private MouseEventArgs lastMouseDownEventArgs = null;

        /// <summary>
        /// Event Handler for the checked list box MouseDown Event.
        /// Cache the MouseEventArgs for use in the chklbPlugins_ItemCheck handler. 
        /// </summary>
        /// <param name="sender">The control that raised the event.</param>
        /// <param name="e">The event arguments.</param>
        private void chklbPlugins_MouseDown(object sender, System.Windows.Forms.MouseEventArgs e)
        {
            lastMouseDownEventArgs = e;
        }

        /// <summary>
        /// Event Handler from the checked list box ItemCheck Event.
        /// Enabled/disable the real time stylus plugin corresponding
        /// to the item being checked/unchecked.
        /// </summary>
        /// <param name="sender">The control that raised the event.</param>
        /// <param name="e">The event arguments.</param>
        private void chklbPlugins_ItemCheck(object sender, System.Windows.Forms.ItemCheckEventArgs e)
        {
            // The CheckedListBox CheckOnClick property allows the user
            // to toggle the checkbox without first selecting the list item,
            // but since the list also allows the user to rearrange the
            // items, we only want CheckOnClick to proceed if the user clicks
            // on the checkbox itself.
            if (lastMouseDownEventArgs != null)
            {
                // If we previously cached MouseDownEventArgs, check the MouseDown location.
                bool abortItemCheck = (lastMouseDownEventArgs.X > chklbPlugins.GetItemHeight(e.Index));
                
                // Reset the cache
                lastMouseDownEventArgs = null;

                // If the MouseDown location is outside the checkbox area, abort.
                if (abortItemCheck)
                {
                    e.NewValue = e.CurrentValue;
                    return;
                }
            }

            // Enable/disable the plugin according to its new
            // checked value.  A plugin is enabled by inserting
            // it into the real time stylus notification chain and it is 
            // disabled by removing it from the plugin collection.  The dynamic
            // renderer is handled specially by setting its enabled 
            // property.
            if(e.Index < chklbPlugins.Items.Count)
            {
                if (e.NewValue == CheckState.Checked)
                {
                    InsertIntoPluginCollection(e.Index);
                }
                else if (e.NewValue == CheckState.Unchecked)
                {
                    RemoveFromPluginCollection(e.Index);
                }
            }

            // Clear the form
            Clear();
        }

        /// <summary>
        /// Event Handler from the Clear Menu Item.
        /// Clears the screen.
        /// </summary>
        /// <param name="sender">The control that raised the event.</param>
        /// <param name="e">The event arguments.</param>
        private void miClear_Click(object sender, System.EventArgs e)
        {
            Clear();
        }

        /// <summary>
        /// Event Handler from the Exit Menu Item.
        /// Exits the application.
        /// </summary>
        /// <param name="sender">The control that raised the event.</param>
        /// <param name="e">The event arguments.</param>
        private void miExit_Click(object sender, System.EventArgs e)
        {
            Application.Exit();
        }

        /// <summary>
        /// Event Handler from MoveUp and MoveDown Button Click.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void btnMove_Click(object sender, System.EventArgs e)
        {
            // Declare the current and destination indices of the 
            // real time stylus plugin being moved.
            int currentIndex = chklbPlugins.SelectedIndex;
            int destinationIndex = -1;

            if ((0 <= currentIndex) && (currentIndex <= chklbPlugins.Items.Count))
            {

                // Calculate the destination based on whether move up
                // or down was pressed.
                if (sender == btnMoveUp)
                {
                    destinationIndex = currentIndex - 1;
                }
                else
                {
                    destinationIndex = currentIndex + 1;
                }

                // The destination index of the plugin being moved must fall 
                // within the bounds of the plugin list.
                if ((0 <= destinationIndex) && (destinationIndex < chklbPlugins.Items.Count))
                {
                    CheckState checkState = chklbPlugins.GetItemCheckState(currentIndex);

                    // If the plugin being moved is currently enabled,
                    // remove it from the plugin collection.
                    if (CheckState.Checked == checkState)
                    {
                        RemoveFromPluginCollection(currentIndex);
                    }

                    // Update the checked list box to reflect the move.
                    object item = chklbPlugins.Items[currentIndex];
                    chklbPlugins.Items.RemoveAt(currentIndex);
                    chklbPlugins.Items.Insert(destinationIndex,item); 

                    // Restore the selected and checked state of the list item.
                    chklbPlugins.SetSelected(destinationIndex, true);
                    chklbPlugins.SetItemCheckState(destinationIndex, checkState);
                }   
            }
        }

        /// <summary>
        /// Helper method to clear the form.
        /// </summary>
        private void Clear()
        {
            sbGesture.Text = String.Empty;
            this.Refresh();
        }

        /// <summary>
        /// Helper method to find the index where the specified 
        /// plugin should be inserted.
        /// </summary>
        /// <param name="index">The index of the plugin.</param>
        /// <returns>The index to insert the specified plugin.</returns>
        private int FindPrecedingPlugin(int index)
        {
            int insertAtIndex = 0;

            // For each real time stylus plugin preceding
            // the specified index, check whether it is currently
            // enabled.  If so, update the insert at index.
            for (int i = 0; i < index; i++)
            {
                if (chklbPlugins.GetItemChecked(i))
                {
                    insertAtIndex++;
                }
            }

            // Return the preceding plugin or null if none found.
            return insertAtIndex;
        }

        /// <summary>
        /// Helper method to insert the real time stylus plugin
        /// at the specified index into the plugin collection.
        /// </summary>
        /// <param name="index">The index of the plugin to insert.</param>
        private void InsertIntoPluginCollection(int index)
        {
            // Obtain the index where the plugin should be inserted
            int insertAtIndex = FindPrecedingPlugin(index);
            IStylusSyncPlugin plugin = ((PluginListItem)chklbPlugins.Items[index]).Plugin;

            bool rtsEnabled = myRealTimeStylus.Enabled;
            myRealTimeStylus.Enabled = false;
            myRealTimeStylus.SyncPluginCollection.Insert(insertAtIndex,plugin);
            myRealTimeStylus.Enabled = rtsEnabled;

        }

        /// <summary>
        /// Helper method to remove the real time stylus plugin
        /// at the specified index from the plugin collection.
        /// </summary>
        /// <param name="index">The index of the plugin to remove.</param>
        private void RemoveFromPluginCollection(int index)
        {
            IStylusSyncPlugin plugin = ((PluginListItem)chklbPlugins.Items[index]).Plugin;

            bool rtsEnabled = myRealTimeStylus.Enabled;
            myRealTimeStylus.Enabled = false;
            myRealTimeStylus.SyncPluginCollection.Remove(plugin);
            myRealTimeStylus.Enabled = rtsEnabled;
        }

        #region IStylusAsyncPlugin interface implementation

        /// <summary>
        /// Informs the implementing object that user data is available.
        /// </summary>
        /// <param name="sender">The real time stylus associated with the notification</param>
        /// <param name="data">The notification data</param>
        public void CustomStylusDataAdded(RealTimeStylus sender, CustomStylusData data)
        {
            // We can identify the kind of custom data via either the Guid or Type.
            // For the purpose of this demonstration we will validate both just to be safe.
            // For other scenarios either approach is valid.
            if (data.CustomDataId == GestureRecognizer.GestureRecognitionDataGuid)
            {
                GestureRecognitionData grd = data.Data as GestureRecognitionData;
                if (grd != null)
                {
                    if (grd.Count > 0)
                    {
                        GestureAlternate ga = grd[0];
                        sbGesture.Text = "Gesture=" + ga.Id + ", Confidence=" + ga.Confidence;
                    }
                }
            }
        }

        /// <summary>
        /// Called when the current plugin or the ones previous in the list
        /// threw an exception.
        /// </summary>
        /// <param name="sender">The real time stylus associated with the notification</param>
        /// <param name="data">The notification data</param>
	public void Error(RealTimeStylus sender, ErrorData data)
        {
            Debug.Assert(false, null, "An error occurred.  DataId=" + data.DataId + ", " + "Exception=" + data.InnerException);
        }

        /// <summary>
        /// Defines the types of notifications the plugin is interested in.
        /// </summary>
        public DataInterestMask DataInterest
        {
            get
            {
                return DataInterestMask.CustomStylusDataAdded | DataInterestMask.Error;
            }
        }

        // The remaining interface methods are not used in this sample application.
        public void RealTimeStylusDisabled(RealTimeStylus sender, RealTimeStylusDisabledData data) {}
        public void RealTimeStylusEnabled(RealTimeStylus sender, RealTimeStylusEnabledData data){}
        public void StylusOutOfRange(RealTimeStylus sender, StylusOutOfRangeData data) {}
        public void StylusInRange(RealTimeStylus sender, StylusInRangeData data) {}
        public void StylusDown(RealTimeStylus sender, StylusDownData data) {}
        public void StylusUp(RealTimeStylus sender, StylusUpData data) {}
        public void StylusButtonDown(RealTimeStylus sender, StylusButtonDownData data) {}
        public void StylusButtonUp(RealTimeStylus sender, StylusButtonUpData data) {}
        public void SystemGesture(RealTimeStylus sender, SystemGestureData data){}
        public void Packets(RealTimeStylus sender,  PacketsData data) {}
        public void InAirPackets(RealTimeStylus sender, InAirPacketsData data){}
        public void TabletAdded(RealTimeStylus sender, TabletAddedData data){}
        public void TabletRemoved(RealTimeStylus sender, TabletRemovedData data) {}
        #endregion

    }
}

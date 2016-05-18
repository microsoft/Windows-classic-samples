// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) 2006 Microsoft Corporation. All rights reserved.

using System;
using System.Configuration;
using System.Drawing;
using System.Windows.Forms;
using Microsoft.Feeds.Interop;

namespace Microsoft.Samples.RssPlatform.ScreenSaver
{
    partial class OptionsForm : Form
    {
        public OptionsForm()
        {
            InitializeComponent();

            // Load the text boxes from the current settings
            try
            {
                rootFolderTextBox.Text = Properties.Settings.Default.ImagePathOverride;
                rssFadePauseLength.Value = Properties.Settings.Default.FadePauseInterval;
            }
            catch(System.Configuration.SettingsPropertyNotFoundException)
            {
                MessageBox.Show("The default settings could not be located.",
                    "Failed to Load Settings", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }

        // Determine whether changes have been made
        private Boolean IsDirty()
        {
            return (
                Properties.Settings.Default.ImagePathOverride != rootFolderTextBox.Text.Trim() ||
                Properties.Settings.Default.FadePauseInterval != rssFadePauseLength.Value );
        }

        // Update the apply button to be active only if changes 
        // have been made since apply was last pressed
        private void UpdateApply()
        {
            applyButton.Enabled = IsDirty();
        }

        // Verify that folder exists
        private bool ExistsFolder(string name)
        {
            // This code is encapsulated here because it will cause an exception
            // _as the stack is constructed_ if the Interop.Feeds.dll is not installed properly.
            IFeedsManager fm = new FeedsManager();
            return fm.ExistsFolder(name);
        }

        // Verify that the folder name is valid, and it exists
        private bool ValidateFolderName(string name)
        {
            try
            {
                return (name == "" || ExistsFolder(name));
            }
            catch (System.IO.FileNotFoundException)
            {
                // This error is most likely caused by a missing interop assembly
                string msg = "Please copy Interop.Feeds.dll into the same directory as the screen saver binary.";
                MessageBox.Show(msg, "Missing Interop Assembly", MessageBoxButtons.OK, MessageBoxIcon.Stop);
            }
            return false;
        }

        // Apply all the changes since apply button was last pressed
        private bool ApplyChanges()
        {
            if (IsDirty())
            {
                string folderName = rootFolderTextBox.Text.Trim();
                // Empty folder name (root) is always valid
                if (ValidateFolderName(folderName))
                {
                    Properties.Settings.Default.ImagePathOverride = folderName;
                    Properties.Settings.Default.FadePauseInterval = rssFadePauseLength.Value;
                    Properties.Settings.Default.Save();
                }
                else
                {
                    string msg = String.Format("Feed folder '{0}' does not exist.", folderName);
                    MessageBox.Show(msg, "Choose Another Feed Folder", MessageBoxButtons.OK, MessageBoxIcon.Error);
                    rootFolderTextBox.Focus();
                    rootFolderTextBox.SelectAll();
                    return false;
                }
            }
            return true;
        }

        private void btnOK_Click(object sender, EventArgs e)
        {
            try
            {
                if (ApplyChanges())
                    Close();
            }
            catch (ConfigurationException)
            {
                string msg = "Your settings couldn't be saved.  Make sure that you have a .config file in the same directory as your screensaver.";
                MessageBox.Show(msg, "Failed to Save Settings", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }

        private void btnCancel_Click(object sender, EventArgs e)
        {
            Close();
        }

        private void btnApply_Click(object sender, EventArgs e)
        {
            ApplyChanges();
            UpdateApply();
        }

        private void rootFolderTextBox_TextChanged(object sender, EventArgs e)
        {
            UpdateApply();
        }

        private void rssFadePauseLength_Scroll(object sender, EventArgs e)
        {
            UpdateApply();
        }

    }
}
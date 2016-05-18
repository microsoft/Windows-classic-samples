//=======================================================================================;
//
//  This source code is only intended as a supplement to existing Microsoft documentation. 
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
//  KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
//  IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
//  PURPOSE.
//
//  Copyright (C) Microsoft Corporation.  All Rights Reserved.
//
//=======================================================================================;

using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;

namespace Microsoft.ManagementConsole.Samples
{
    /// <summary>
    /// Form that gets a name for the snapin
    /// </summary>
    public partial class InitializationWizard : Form
    {
        /// <summary>
        /// Constructor
        /// </summary>
        public InitializationWizard()
        {
            InitializeComponent();
            this.SelectedSnapInName = "Unknown";
        }

        /// <summary>
        /// Handles Continue button click
        /// </summary>
        /// <param name="sender">sender</param>
        /// <param name="e">e</param>
        private void Continue_Click(object sender, EventArgs e)
        {
            this.DialogResult = DialogResult.OK;
        }

        /// <summary>
        /// Name entered
        /// </summary>
        public string SelectedSnapInName
        {
            get 
            {
                return SnapInName.Text;
            }
            set 
            {
                SnapInName.Text = value;
            }
        }

    } // form
} // namespace

//=======================================================================================
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
//=======================================================================================

using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Text;
using System.Windows.Forms;

namespace Microsoft.ManagementConsole.Samples
{
    /// <summary>
    /// Gets Name and Birthday
    /// </summary>
    public partial class MachinePropertiesControl : UserControl
    {
        /// <summary>
        /// Parent property page to expose data and state of property sheet
        /// </summary>
        private MachinePropertyPage machinePropertyPage;

        /// <summary>
        /// Constructor
        /// </summary>
        /// <param name="parentPropertyPage">Container property page for the control</param>
        public MachinePropertiesControl(MachinePropertyPage parentPropertyPage)
        {
            // This call is required by the Windows.Forms Form Designer.
            InitializeComponent();

            // keep reference to parent
            machinePropertyPage = parentPropertyPage;
        }

        /// <summary>
        /// Populate control values from the SelectionObject (set in UserListView.SelectionOnChanged)
        /// </summary>
        /// <param name="sharedDataItem"></param>
        public void RefreshData(SharedDataItem sharedDataItem)
        {
            this.MachineName.Text = GetMachineName(sharedDataItem);
            machinePropertyPage.Dirty = false;
        }

        /// <summary>
        /// Update the node with the controls values
        /// </summary>
        /// <param name="sharedDataItem">Node being updated by property page</param>
        public void UpdateData(SharedDataItem sharedDataItem)
        {
            // In this sample the primary does not allow its data to be updated.
            // However, you can create primary nodes with updatable shared data 
            // that would be accessed with a line such as the one below.
            
            // sharedDataItem.SetData(Encoding.Unicode.GetBytes(this.MachineName.Text));
            machinePropertyPage.Dirty = false;
        }

        /// <summary>
        /// Check during MachineProptertyPage.OnApply to ensure that changes can be Applied
        /// </summary>
        /// <returns>returns true if changes are valid</returns>
        public bool CanApplyChanges()
        {
            bool result = false;

            if (MachineName.Text.Trim().Length == 0)
            {
                MessageBox.Show("Machine Name cannot be blank");
            }
            else
            {
                result = true;
            }
            return result;
        }

        /// <summary>
        /// Notifies the PropertyPage that info has changed and that the PropertySheet can change the 
        /// buttons
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void MachineName_TextChanged(object sender, System.EventArgs e)
        {
            machinePropertyPage.Dirty = true;
        }

        /// <summary>
        /// The computer name is published by the primary.  
        /// Note: this defaults to an empty string for localhost.
        /// </summary>
        /// <param name="sharedDataItem"></param>
        private string GetMachineName(SharedDataItem sharedDataItem)
        {
            // get buffer containing the machine name
            string machineName = Encoding.Unicode.GetString(sharedDataItem.GetData());

            // find first null terminated string in buffer. 
            if (machineName.IndexOf('\0') <= 0)
            {
                // either not found in buffer or first entry in buffer
                machineName = String.Empty;
            }
            else
            {
                machineName = machineName.Substring(0, machineName.IndexOf('\0'));
            }

            // if empty then localhost
            if (machineName == string.Empty)
            {
                machineName = "localhost";
            }
            
            return (machineName);
        }

    } // class
} // namespace

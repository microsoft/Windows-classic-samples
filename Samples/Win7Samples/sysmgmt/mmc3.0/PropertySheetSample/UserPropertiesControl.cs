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

using Microsoft.ManagementConsole.Advanced;

namespace Microsoft.ManagementConsole.Samples
{
    /// <summary>
    /// Gets Name and Birthday
    /// </summary>
    public partial class UserPropertiesControl : UserControl
    {
        /// <summary>
        /// Parent property page to expose data and state of property sheet
        /// </summary>
        private UserPropertyPage userPropertyPage;

        /// <summary>
        /// Constructor
        /// </summary>
        /// <param name="parentPropertyPage">Container property page for the control</param>
        public UserPropertiesControl(UserPropertyPage parentPropertyPage)
        {
            // This call is required by the Windows.Forms Form Designer.
            InitializeComponent();

            // keep reference to parent
            userPropertyPage = parentPropertyPage;
        }

        /// <summary>
        /// Populate control values from the SelectionObject (set in UserListView.SelectionOnChanged)
        /// </summary>
        public void RefreshData(ResultNode userNode)
        {
            this.UserName.Text = userNode.DisplayName;
            this.Birthday.Text = userNode.SubItemDisplayNames[0];    // first subitem
            userPropertyPage.Dirty = false;
        }

        /// <summary>
        /// Update the node with the controls values
        /// </summary>
        /// <param name="userNode">Node being updated by property page</param>
        public void UpdateData(ResultNode userNode)
        {
            userNode.DisplayName = this.UserName.Text;
            userNode.SubItemDisplayNames[0] = this.Birthday.Text;    // first subitem
            userPropertyPage.Dirty = false;
        }


        /// <summary>
        /// Check during UserProptertyPage.OnApply to ensure that changes can be Applied
        /// </summary>
        /// <returns>returns true if changes are valid</returns>
        public bool CanApplyChanges()
        {
            bool result = false;

            if (UserName.Text.Trim().Length == 0)
            {
                MessageBoxParameters messageBoxParameters = new MessageBoxParameters();
                messageBoxParameters.Text = "Name cannot be blank";
                userPropertyPage.ParentSheet.ShowDialog(messageBoxParameters);
            }
            else if (Birthday.Text.Trim().Length == 0)
            {
                MessageBoxParameters messageBoxParameters = new MessageBoxParameters();
                messageBoxParameters.Text = "Birthday cannot be blank";
                userPropertyPage.ParentSheet.ShowDialog(messageBoxParameters);
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
        private void UserName_TextChanged(object sender, System.EventArgs e)
        {
            userPropertyPage.Dirty = true;
        }

        /// <summary>
        /// Notifies the PropertyPage that info has changed and that the PropertySheet can change the 
        /// buttons
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void Birthday_TextChanged(object sender, System.EventArgs e)
        {
            userPropertyPage.Dirty = true;
        }

    }
}

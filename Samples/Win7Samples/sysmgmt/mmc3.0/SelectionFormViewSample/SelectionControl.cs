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
    public partial class SelectionControl : UserControl, IFormViewControl
    {
        SelectionFormView selectionFormView = null;

        /// <summary>
        /// Constructor
        /// </summary>
        public SelectionControl()
        {
            // initialize the controls
            InitializeComponent();
            this.Dock = DockStyle.Fill;

            //// setup the list
            UserListView.View = System.Windows.Forms.View.Details;

            ColumnHeader userColumnHeader = new ColumnHeader();
            userColumnHeader.Text = "User";
            userColumnHeader.Width = 200;
            UserListView.Columns.Add(userColumnHeader);

            ColumnHeader birthdayColumnHeader = new ColumnHeader();
            birthdayColumnHeader.Text = "BirthDay";
            birthdayColumnHeader.Width = 200;
            UserListView.Columns.Add(birthdayColumnHeader);
        }

        /// <summary>
        /// Cache the associated Form View and add the actions
        /// </summary>
        /// <param name="parentSelectionFormView">Containing form</param>
        ///         
        void IFormViewControl.Initialize(FormView parentSelectionFormView)
        {
            selectionFormView = (SelectionFormView)parentSelectionFormView;

            // Add the actions
            selectionFormView.SelectionData.ActionsPaneItems.Clear();
            selectionFormView.SelectionData.ActionsPaneItems.Add(new Action("Show Selection", "Shows the Names of the selected Items in the FormView's ListView.", -1, "ShowSelection"));
        }

        /// <summary>
        /// Populate the list with sample data
        /// </summary>
        /// <param name="users">array of user data to add to the list</param>
        public void RefreshData(string[][] users)
        {
            // empty the list
            UserListView.Items.Clear();

            // populate the list using the sample data.
            foreach (string[] user in users)
            {
                ListViewItem listViewItem = new ListViewItem();
                listViewItem.Text = user[0];
                listViewItem.SubItems.Add(user[1]);
                UserListView.Items.Add(listViewItem);
            }
        }

        /// <summary>
        /// Shows the selected items that were stored in the FormView's selected data context
        /// during the UserListView_SelectedIndexChanged 
        /// </summary>
        public void ShowSelection()
        {
            if (UserListView.SelectedItems == null)
            {
                MessageBox.Show("There are no items selected");
            }
            else
            {
                MessageBox.Show("Selected Users: \n" + GetSelectedUsers());
            }
        }

        /// <summary>
        /// Updates the FormView's selected data context
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void UserListView_SelectedIndexChanged(object sender, EventArgs e)
        {
            if (UserListView.SelectedItems.Count == 0)
            {
                selectionFormView.SelectionData.Clear();
            }
            else
            {
                // update MMC with the current selection information
                selectionFormView.SelectionData.Update(GetSelectedUsers(), UserListView.SelectedItems.Count > 1, null, null);

                // update action pane selected data menu's title
                selectionFormView.SelectionData.DisplayName = ((UserListView.SelectedItems.Count == 1) ? UserListView.SelectedItems[0].Text : "Selected Objects");
            }
        }

        /// <summary>
        /// Build string of selected users
        /// </summary>
        /// <returns></returns>
        private string GetSelectedUsers()
        {
            StringBuilder selectedUsers = new StringBuilder();
            
            foreach (ListViewItem listViewItem in UserListView.SelectedItems)
            {
                selectedUsers.Append(listViewItem.Text + "\n");
            }

            return selectedUsers.ToString();
        }

        /// <summary>
        /// Handle mouseclick and use MMC to show context menu if necessary 
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void UserListView_MouseClick(object sender, MouseEventArgs e)
        {
            if (e.Button == MouseButtons.Right)
            {
                // did they right click on selected items?
                bool rightClickedOnSelection = false;

                ListViewItem rightClickedItem = UserListView.GetItemAt(e.X, e.Y);
                if (rightClickedItem == null || rightClickedItem.Selected == false)
                {
                    rightClickedOnSelection = false;
                }
                else
                {
                    rightClickedOnSelection = true;
                }

                // show context menu
                selectionFormView.ShowContextMenu(PointToScreen(e.Location), rightClickedOnSelection);
            }
        }
    
    } // class
} // namespace

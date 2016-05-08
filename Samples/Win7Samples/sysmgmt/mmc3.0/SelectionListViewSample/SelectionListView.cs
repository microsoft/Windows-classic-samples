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
using System.Windows.Forms;
using System.Text;

namespace Microsoft.ManagementConsole.Samples
{
    /// <summary>
    /// MmcListView class - Basic list of icons and names in the result pane.
    /// SelectionListView class - Loads and allows selection of users list.
    /// </summary>
    public class SelectionListView : MmcListView
    {
        /// <summary>
        /// Constructor
        /// </summary>
        public SelectionListView()
        {
        }

        /// <summary>
        /// Define the ListView's structure 
        /// </summary>
        /// <param name="status">status for updating the console</param>
        protected override void OnInitialize(AsyncStatus status)
        {
            // do default handling
            base.OnInitialize(status);

            // Create a set of columns for use in the list view
            // Define the default column title
            this.Columns[0].Title = "User";
            this.Columns[0].SetWidth(300);

            // Add detail column
            this.Columns.Add(new MmcListViewColumn("Birthday", 200));

            // Set to show all columns
            this.Mode = MmcListViewMode.Report;  // default (set for clarity)

            // set to show refresh as an option
            this.SelectionData.EnabledStandardVerbs = StandardVerbs.Refresh;

            // Load the list with values
            Refresh();
        }

        /// <summary>
        /// Define actions for selection  
        /// </summary>
        /// <param name="status">status for updating the console</param>
        protected override void OnSelectionChanged(SyncStatus status)
        {
            if (this.SelectedNodes.Count == 0)
            {
                this.SelectionData.Clear();
            }
            else
            {
                this.SelectionData.Update(GetSelectedUsers(), this.SelectedNodes.Count > 1, null, null);
                this.SelectionData.ActionsPaneItems.Clear();
                this.SelectionData.ActionsPaneItems.Add(new Action("Show Selected", "Shows list of selected Users.", -1, "ShowSelected"));
            }
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="status"></param>
        protected override void OnRefresh(AsyncStatus status)
        {
            MessageBox.Show("The method or operation is not implemented.");
        }

        /// <summary>
        /// Handle short cut style menu actions for selection
        /// </summary>
        /// <param name="action">triggered action</param>
        /// <param name="status">asynchronous status used to update the console</param>
        protected override void  OnSelectionAction(Action action, AsyncStatus status)
        {
            switch ((string)action.Tag)
            {
                case "ShowSelected":
                    {
                        ShowSelected();
                        break;
                    }
            }
        }

        /// <summary>
        /// Shows selected 
        /// </summary>
        private void ShowSelected()
        {
            MessageBox.Show("Selected Users: \n" + GetSelectedUsers());
        }

        /// <summary>
        /// Build string of selected users
        /// </summary>
        /// <returns></returns>
        private string GetSelectedUsers()
        {
            StringBuilder selectedUsers = new StringBuilder();

            foreach (ResultNode resultNode in this.SelectedNodes)
            {

                selectedUsers.Append(resultNode.DisplayName + "\n");
            }

            return selectedUsers.ToString();
        }

        /// <summary>
        /// Loads the ListView with data
        /// </summary>
        public void Refresh()
        {
            // Clear existing information
            this.ResultNodes.Clear();

            // Get some fictitious data to populate the lists with
            string[][] users = { new string[] {"Karen", "February 14th"},
                                        new string[] {"Sue", "May 5th"},
                                        new string[] {"Tina", "April 15th"},
                                        new string[] {"Lisa", "March 27th"},
                                        new string[] {"Tom", "December 25th"},
                                        new string[] {"John", "January 1st"},
                                        new string[] {"Harry", "October 31st"},
                                        new string[] {"Bob", "July 4th"}
                                    };

            // Populate the list.
            foreach (string[] user in users)
            {
                ResultNode node = new ResultNode();
                node.DisplayName = user[0];
                node.SubItemDisplayNames.Add(user[1]);

                this.ResultNodes.Add(node);
            }
        }
    } // class
} //namespace

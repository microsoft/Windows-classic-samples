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
using System.Windows.Forms;
using System.Text;

namespace Microsoft.ManagementConsole.Samples
{
    /// <summary>
    /// MmcListView - A basic result pane that lists ResultNodes
    /// ActionListView - An MmcListView that can handle some custom Actions  
    /// </summary>
    public class ActionListView : MmcListView
    {
        /// <summary>
        /// Constructor
        /// </summary>
        public ActionListView()
        {
        }

        /// <summary>
        /// Define the ListView's structure and actions
        /// </summary>
        /// <param name="status">asynchronous status for updating the console</param>
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

            // define mode action
            this.ModeActionsPaneItems.Clear();
            this.ModeActionsPaneItems.Add(new Action("Sort by Name", "Sorts list in Name order.", 0 , "SortByName"));
            this.ModeActionsPaneItems.Add(new Action("Sort by Birthday", "Sorts list in Birthday order.", 0, "SortByBirthday"));

            // define some view level actions
            this.ActionsPaneItems.Add(new Action("Refresh", "Reload the list", 0, "Refresh"));

            // Load the list with values
            Refresh();
        }

        /// <summary>
        /// Handles the view level actions
        /// </summary>
        /// <param name="action">action that was triggered</param>
        /// <param name="status">asynchronous status for updating the console</param>
        protected override void OnAction(Action action, AsyncStatus status)
        {
            switch ((string)action.Tag)
            {
                case "Refresh":
                    {
                        this.Refresh();
                        break;
                    }
            }
        }

        /// <summary>
        /// Handle all the mode actions
        /// </summary>
        /// <param name="action">action that was triggered</param>
        /// <param name="status">asynchronous status for updating the console</param>
        protected override void OnModeAction(Action action, AsyncStatus status)
        {
            // handle triggered action 
            switch ((string)action.Tag)
            {
                case "SortByName":
                    {
                        this.Sort(0);
                        break;
                    }

                case "SortByBirthday":
                    {
                        this.Sort(1);
                        break;
                    }
            }

            // set selection to bullet 
            action.Bulleted = true;
        }

        /// <summary>
        /// Manage the shortcut style menu for selection  
        /// </summary>
        /// <param name="status">asynchronous status for updating the console</param>
        protected override void OnSelectionChanged(SyncStatus status)
        {
            if (this.SelectedNodes.Count == 0)
            {
                this.SelectionData.Clear();
            }
            else
            {
                this.SelectionData.Update(null, this.SelectedNodes.Count > 1, null, null);
                this.SelectionData.ActionsPaneItems.Clear();
                this.SelectionData.ActionsPaneItems.Add(new Action("Show Selected", "Shows list of selected Users.", 0, "ShowSelected"));
            }
        }

        /// <summary>
        /// Handle short cut menu actions for selection
        /// </summary>
        /// <param name="action">action that was triggered</param>
        /// <param name="status">asynchronous status for updating the console</param>
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
        /// Shows names from selected result nodes in a messagebox
        /// </summary>
        private void ShowSelected()
        {
            StringBuilder selectedUserNames = new StringBuilder();

            foreach (ResultNode selectedNode in this.SelectedNodes)
            {
                selectedUserNames.Append(selectedNode.DisplayName + "\n");
            }

            MessageBox.Show("Selected Users: \n" + selectedUserNames.ToString());
        }

        /// <summary>
        /// Loads the list with fictitious data
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
                node.ImageIndex = 0;

                this.ResultNodes.Add(node);
            }
        }

    } // class
} //namespace

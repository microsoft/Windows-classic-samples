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
    /// MmcListView class - Basic listing of icon and name in the result pane.
    /// StandardVerbsListView class - Listing with some StandardVerbs enabled.
    /// </summary>
    public class StandardVerbsListView : MmcListView
    {
        /// <summary>
        /// Constructor.
        /// </summary>
        public StandardVerbsListView()
        {
        }

        /// <summary>
        /// Define the ListView's structure and actions
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
            this.Mode = MmcListViewMode.Report;

            // Load the list with values
            Refresh();
        }

        /// <summary>
        /// Manage shortcut menu for selection  
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
                this.SelectionData.Update(null, this.SelectedNodes.Count > 1, null, null);
                this.SelectionData.EnabledStandardVerbs = StandardVerbs.Delete | StandardVerbs.Refresh;

                // rename is only for single node
                if (this.SelectedNodes.Count == 1)
                {
                    this.SelectionData.EnabledStandardVerbs = this.SelectionData.EnabledStandardVerbs | StandardVerbs.Rename;
                }
            }
        }

        /// <summary>
        /// Handles Delete standard verb
        /// </summary>
        /// <param name="status">synchronous status used to update console</param>
        protected override void OnDelete(SyncStatus status)
        {
            foreach (ResultNode cutNode in this.SelectedNodes)
            {
                this.ResultNodes.Remove(cutNode);
            }
        }

        /// <summary>
        /// Handles Refresh standard verb
        /// </summary>
        /// <param name="status">asynchronous status used to update console</param>
        protected override void OnRefresh(AsyncStatus status)
        {
            this.Refresh();
        }

        /// <summary>
        /// Handles rename standard verb
        /// </summary>
        /// <param name="newText">name being changed to</param>
        /// <param name="status">asynchronous status used to update console</param>
        protected override void OnRename(string newText, SyncStatus status)
        {
            ResultNode resultNode = (ResultNode)this.SelectedNodes[0];
            resultNode.DisplayName = newText;
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

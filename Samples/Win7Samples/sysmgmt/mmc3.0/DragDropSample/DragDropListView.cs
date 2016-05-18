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
    /// MmcListView - Result Pane view to list Result Nodes
    /// DragDropListView - Adds some drag drop handling behavior to MmcListView
    /// </summary>
    public class DragDropListView : MmcListView
    {
        /// <summary>
        /// Constructor
        /// </summary>
        public DragDropListView()
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

            // Set to show all columns
            this.Mode = MmcListViewMode.Report;

            // Load the list with values
            Refresh();
        }

        /// <summary>
        /// Manage shortcut menu for selection  
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
                //// store dragged name
                WritableSharedData writableSharedData = new WritableSharedData();
                WritableSharedDataItem writableSharedDataItem = new WritableSharedDataItem("DisplayName", false);
                writableSharedDataItem.SetData(Encoding.Unicode.GetBytes(this.SelectedNodes[0].Tag + "\0"));
                writableSharedData.Add(writableSharedDataItem);

                // update context
                this.SelectionData.Update(null, this.SelectedNodes.Count > 1, null, writableSharedData);
                this.SelectionData.EnabledStandardVerbs = StandardVerbs.Copy | StandardVerbs.Paste | StandardVerbs.Refresh;

                //Set SetAllowedClipboardFormatIdsForPaste so that ListView only accepts clipboard objects that support "DisplayName" clipboard format
                this.SelectionData.SetAllowedClipboardFormatIdsForPaste(new string[] { "DisplayName" });
            }
        }

        /// <summary>
        /// Drop handler 
        /// </summary>
        /// <param name="data">shared data</param>
        /// <param name="pasteType">verbs one of {Copy | Move}</param>
        /// <param name="status">synchronous status for updating the console</param>
        /// <returns>true for success</returns>
        protected override bool OnPaste(SharedData data, DragAndDropVerb pasteType, SyncStatus status)
        {
            string displayName = "";

            // get pasted name
            data.Add(new SharedDataItem("DisplayName"));
            try
            {
               displayName = Encoding.Unicode.GetString(data.GetItem("DisplayName").GetData());
            }
            catch (Microsoft.ManagementConsole.Advanced.PrimarySnapInDataException)
            {
                return false;
            }

            // find string in buffer
            displayName = displayName.Substring(0, displayName.IndexOf("\0"));

            // update pasted on node to show the drop
            this.SelectedNodes[0].DisplayName += " ( " +  displayName + " Dropped)";

            return true;
        }

        /// <summary>
        /// Refresh handler
        /// </summary>
        /// <param name="status">asynchronous status for updating the console</param>
        protected override void OnRefresh(AsyncStatus status)
        {
            this.Refresh();
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
                node.Tag = node.DisplayName;

                this.ResultNodes.Add(node);
            }
        }

    } // class
} //namespace

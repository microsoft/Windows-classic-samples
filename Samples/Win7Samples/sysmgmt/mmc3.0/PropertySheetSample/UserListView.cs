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
using System.ComponentModel;
using Microsoft.ManagementConsole;

namespace Microsoft.ManagementConsole.Samples
{
	/// <summary>
	/// MmcListView class - Provides the base functionality required to present a list in MMC's Result Pane
	/// </summary>
	public class UserListView : MmcListView
	{
		/// <summary>
		/// Contstructor
		/// </summary>
		public UserListView()
		{
		}
		
		/// <summary>
		/// 
		/// </summary>
        /// 

        protected override void OnInitialize(AsyncStatus status)
        {
			// handle any basic setup
			base.OnInitialize(status);

			// define view columns. 
			// 1st column already exists
			this.Columns[0].Title = "User";
			this.Columns[0].SetWidth(300);
			// add second+ columns
			this.Columns.Add(new MmcListViewColumn("Birthday", 200));

			// populate the list
			Refresh();

			// define actions
			this.ActionsPaneItems.Add(new Action("Refresh", "refresh", -1, "Refresh"));
		}

		/// <summary>
		/// Handle view global action execution
		/// </summary>
        protected override void OnAction(Action action, AsyncStatus status)
        {
			switch ((string)action.Tag)
			{
				case "Refresh":
				{
					Refresh();
					break;
				}
			}
		}
	
		/// <summary>
		/// Load the list with data. (In this case fictional data.)
		/// </summary>
		protected void Refresh()
		{
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

			// remove existing data
			this.ResultNodes.Clear();

			// repopulate list
			foreach (string[] user in users)
			{
				ResultNode userNode = new ResultNode();
				userNode.DisplayName = user[0];
				userNode.SubItemDisplayNames.Add(user[1]);
				this.ResultNodes.Add(userNode);
			}
		}

		/// <summary>
		/// Handles changes in ListView selection. Only acts on first selected row.
		/// </summary>
		/// <param name="status">synchronous status to update the console</param>
        protected override void OnSelectionChanged(SyncStatus status)
        {
			int count = SelectedNodes.Count;

			// update selection context
			if (count == 0)
			{
				this.SelectionData.Clear();
				this.SelectionData.ActionsPaneItems.Clear();
			}
			else
			{
                // update MMC with the selection information. 
                // MMC will find an already open property sheet based on the SelectionObject (first parameter below)
				this.SelectionData.Update((ResultNode)this.SelectedNodes[0], count > 1, null, null);
				this.SelectionData.ActionsPaneItems.Clear();
				this.SelectionData.ActionsPaneItems.Add(new Action("Properties", "Properties", -1, "Properties"));
			}
		}

		/// <summary>
		/// Handle action for selected resultnode 
		/// </summary>
        /// 
        protected override void OnSelectionAction(Action action, AsyncStatus status)
        {
			switch ((string)action.Tag)
			{
				case "Properties":
				{
					this.SelectionData.ShowPropertySheet("User Properties");   // triggers OnAddPropertyPages
					break;
				}
			}
		}

		/// <summary>
		/// OnAddPropertyPages is used to get the property pages to show. 
		/// (triggered by SelectionData.ShowPropertySheet)
		/// </summary>
		/// <param name="propertyPageCollection">property pages</param>
		protected override void OnAddPropertyPages(PropertyPageCollection propertyPageCollection)
		{
			if(this.SelectedNodes.Count == 0)
			{
				throw new Exception("there should be at least one selection");
			}
			else
			{
				// add at least one property page relevant to the selection
				propertyPageCollection.Add(new UserPropertyPage());
			}
		}

	} // class
} // namespace

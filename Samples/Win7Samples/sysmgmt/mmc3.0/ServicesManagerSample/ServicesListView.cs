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
using System.Text;
using System.Windows.Forms;
using System.Drawing;
using System.Configuration.Install;
using System.ComponentModel;
using System.Diagnostics;
using System.Collections;
using System.Management;
using Microsoft.ManagementConsole;

namespace Microsoft.ManagementConsole.Samples
{
    /// <summary>
    /// The ListView class provides a standard MMC view that allows the result pane to be populated with a list of items
    /// </summary>
    public class ServicesListView : MmcListView
	{
        private MmcListViewColumn DescriptionColumn;
        private MmcListViewColumn StatusColumn;
        private MmcListViewColumn StartupType;
        private MmcListViewColumn LogOnAs;
		
        /// <summary>
        /// 
        /// </summary>
        /// <param name="status">asynchronous status to update the console</param>
		protected override void OnInitialize(AsyncStatus status)
		{
            // Create a set of columns for use in the list view
            DescriptionColumn = new MmcListViewColumn("Description ", 300);
            StatusColumn = new MmcListViewColumn("Status", -1);
            StartupType = new MmcListViewColumn("Startup Type", -1);
            LogOnAs = new MmcListViewColumn("Log On As", -1);

            Columns.AddRange(new MmcListViewColumn[] { DescriptionColumn, StatusColumn, StartupType, LogOnAs });
			Columns[0].Title = "Service ";
			Columns[0].SetWidth(200);

            // load the list with values
            Refresh();

		    // Add a view Action.
            Action ViewAction = new Action("Refresh", "refresh", -1, "Refresh");
            ViewAction.Checked = true;
			ActionsPaneItems.Add(ViewAction);

            Mode = MmcListViewMode.LargeIcon;

            base.OnInitialize(status);
		}

		/// <summary>
		/// Handle view global action execution.
		/// </summary>
		protected override void OnAction(Action action, AsyncStatus status)
		{
            string actionString = action.Tag as string;
            if (actionString != null)
            {
                switch (actionString)
                {
                    case "Refresh":
                        {
                            Refresh();
                            break;
                        }
                }
            }
        }

        /// <summary>
        /// The OnSelectionChanged method is called when the selection changes. 
        /// The snap-in must override this method to read the updated selected nodes 
        /// property and update the selection data accordingly. 
        /// NOTE: MmcListViewOptions is already set to SingleSelect
        /// </summary>
        /// <param name="status">The object that holds the status information.</param>
        protected override void OnSelectionChanged(SyncStatus status)
		{
			int count = SelectedNodes.Count;

			if (count == 0)
			{
				// No items are selected; clear selection data and associated actions.
				SelectionData.Clear();
				SelectionData.ActionsPaneItems.Clear();
			}
			else
			{
				SelectionData.Update(SelectedNodes[0], count > 1, null, null);
				SelectionData.ActionsPaneItems.Clear();

				ManagementObjectSearcher ServiceQuery = new ManagementObjectSearcher
				("Select * from Win32_Service Where DisplayName = '" + SelectedNodes[0].DisplayName + "'");

				foreach (ManagementObject ServiceObject in ServiceQuery.Get())
				{
					SelectionData.ActionsPaneItems.AddRange(new ActionsPaneItem[]
                                                                  {
                                                                     new Action("Start", "Start", -1, "Start"),
                                                                     new Action("Stop", "Stop", -1, "Stop"),
																	 new Action("Pause", "Pause", -1, "Pause"),
																	 new Action("Resume", "Resume", -1, "Resume"),
																	 new Action("Properties", "Properties", -1, "Properties")
                                                                   }
														  );

					string serviceState = ServiceObject.GetPropertyValue("State").ToString();
		
					switch(serviceState)
					{
						case "Running":
                            ((Action)SelectionData.ActionsPaneItems[0]).Enabled = false;
                            ((Action)SelectionData.ActionsPaneItems[3]).Enabled = false;
							break;
						
						case "Stopped":
                            ((Action)SelectionData.ActionsPaneItems[1]).Enabled = false;
                            ((Action)SelectionData.ActionsPaneItems[2]).Enabled = false;
                            ((Action)SelectionData.ActionsPaneItems[3]).Enabled = false;
							break;

						case "Paused":
                            ((Action)SelectionData.ActionsPaneItems[0]).Enabled = false;
                            ((Action)SelectionData.ActionsPaneItems[2]).Enabled = false;
							break;
					}
				}				
				
			}
		}

        /// <summary>
        /// OnSelectionAction method handles the execution of a selection-dependent action.
        /// </summary>
        /// <param name="action">The executed action.</param>
        /// <param name="status">The object that holds the status information.</param>
		protected override void OnSelectionAction(Action action, AsyncStatus status)
		{
			ManagementObjectSearcher ServiceQuery = new ManagementObjectSearcher
			("Select * from Win32_Service Where DisplayName = '" + SelectedNodes[0].DisplayName + "'");

            string actionString = action.Tag as string;
            if (actionString != null)
            {
                switch (actionString)
                {
                    case "Start":
                        {
                            foreach (ManagementObject ServiceObject in ServiceQuery.Get())
                            {
                                ServiceObject.InvokeMethod("StartService", null);
                            }
                            SelectedNodes[0].SubItemDisplayNames[1] = "Started";
                            ((Action)SelectionData.ActionsPaneItems[0]).Enabled = false;
                            ((Action)SelectionData.ActionsPaneItems[3]).Enabled = false;
                        }
                        break;

                    case "Stop":
                        {
                            foreach (ManagementObject ServiceObject in ServiceQuery.Get())
                            {
                                ServiceObject.InvokeMethod("StopService", null);
                            }
                            SelectedNodes[0].SubItemDisplayNames[1] = "Stopped";
                            ((Action)SelectionData.ActionsPaneItems[1]).Enabled = false;
                            ((Action)SelectionData.ActionsPaneItems[2]).Enabled = false;
                            ((Action)SelectionData.ActionsPaneItems[3]).Enabled = false;
                        } 
                        break;

                    case "Pause":
                        {
                            foreach (ManagementObject ServiceObject in ServiceQuery.Get())
                            {
                                ServiceObject.InvokeMethod("PauseService", null);
                            }
                            SelectedNodes[0].SubItemDisplayNames[1] = "Paused";
                            ((Action)SelectionData.ActionsPaneItems[0]).Enabled = false;
                            ((Action)SelectionData.ActionsPaneItems[2]).Enabled = false;
                        }
                        break;

                    case "Resume":
                        {
                            foreach (ManagementObject ServiceObject in ServiceQuery.Get())
                            {
                                ServiceObject.InvokeMethod("ResumeService", null);
                            }
                            SelectedNodes[0].SubItemDisplayNames[1] = "Started";
                            ((Action)SelectionData.ActionsPaneItems[0]).Enabled = false;
                            ((Action)SelectionData.ActionsPaneItems[3]).Enabled = false;
                        }
                        break;


                    case "Properties":
                        {
                            SelectionData.ShowPropertySheet("Service Properties");
                        }
                        break;
                }
            }
        }

		void Refresh()
		{
			this.ResultNodes.Clear();

            // Retrieve data to populate the list with.
            ManagementObjectSearcher searcher = new ManagementObjectSearcher("select * from Win32_service");
            ManagementObjectCollection results = searcher.Get();

            // Populate the list.
            foreach (ManagementObject result in results)
            {
                ResultNode node = new ResultNode();
                node.DisplayName = (string)result.GetPropertyValue("DisplayName");

                string description = (string)result.GetPropertyValue("Description");
                string state = (string)result.GetPropertyValue("State");
                string startMode = (string)result.GetPropertyValue("StartMode");
                string startName = (string)result.GetPropertyValue("StartName");

                description = (description == null) ? String.Empty : description;
                state = (state == null) ? String.Empty : state;
                startMode = (startMode == null) ? String.Empty : startMode;
                startName = (startName == null) ? String.Empty : startName;

                node.SubItemDisplayNames.AddRange(new string[]  
                                        { description,
                                            state,
                                            startMode,
                                            startName     						        
                                        });

                ResultNodes.Add(node);
            }
        }

        /// <summary>
        /// OnAddPropertyPages virtual method is called to add property pages to a property sheet.
        /// </summary>
        /// <param name="propertyPageCollection">The property page collection.</param>
        protected override void OnAddPropertyPages(PropertyPageCollection propertyPageCollection)
		{
			if( SelectedNodes.Count == 0)
			{
				throw new Exception("there should be one selection");
			}
			else
			{
                GeneralPropertyPage generalPropertyPage = new GeneralPropertyPage();
                generalPropertyPage.Title = "General";

                GeneralPropertiesControl generalPropertiesControl = new GeneralPropertiesControl(generalPropertyPage);
                generalPropertyPage.Control = generalPropertiesControl;

                propertyPageCollection.Add(generalPropertyPage);

                StartupPropertyPage startupPropertyPage = new StartupPropertyPage();
                startupPropertyPage.Title = "Startup";

                StartupPropertiesControl startupPropertiesControl = new StartupPropertiesControl(startupPropertyPage);
                startupPropertyPage.Control = startupPropertiesControl;

                propertyPageCollection.Add(startupPropertyPage);
			}
		}
	}
}

//
//  <copyright file="PageAdorner.cs" company="Microsoft">
//    Copyright (C) Microsoft. All rights reserved.
//  </copyright>
//

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Windows.Forms;
using Microsoft.WindowsServerSolutions.Administration.ObjectModel;
using Microsoft.WindowsServerSolutions.Administration.ObjectModel.Adorners;

namespace WSSTabExtenderSample
{
    public class PageAdorner : PageContentAdorner
    {
        public PageAdorner()
            : base(new Guid("9A6CAD3F-28F1-4C1C-BD78-8DFCBCC1225D"), // Put your fixed, static GUID here
            "Extend Computers Sample",
            "Sample that demonstrates how to extend the existing Computers tab including adding a new column.")
        {
        }

        public override ListColumnCollection<ListObject> CreateColumns()
        {
            ListColumn<ListObject> listColumn = new ListColumn<ListObject>("Antivirus", new Converter<ListObject, string>(
                delegate(ListObject listObject)
                {
                    Debug.WriteLine("Getting listObject from ListColumn " + listObject.Id);
                    return GetDataFromYourCache(listObject.Id.ToString());
                }));

            ListColumnCollection<ListObject> columns = new ListColumnCollection<ListObject>();
            columns.Add(listColumn);
            return columns;
        }

        private string GetDataFromYourCache(string ComputerId)
        {
            // This is where you would want to use the id to perform a quick lookup in your cached data
            // DO NOT do network based lookups here as it will have a negative impact on the performance
            // of the dashboard. Instead, use a proxy service or other mechanism for collecting data and
            // caching it locally, then grab that info from here. For this example, we'll just return
            // the string "yes" if the name has the lettter 'a' in it.
            return ComputerId.Contains("a") ? "yes" : "no";
        }

        public override ListProviderAdorner CreateRefreshContext()
        {
            return (new ListAdorner());
        }

        public override TaskCollection CreateTasks()
        {
            TaskCollection tasks = new TaskCollection();

            // Global Tasks
            tasks.Add(new SyncUiTask("Demo custom global task",
                delegate()
                {
                    MessageBox.Show("Your custom code goes here", "", MessageBoxButtons.OK, MessageBoxIcon.Information, MessageBoxDefaultButton.Button1, MessageBoxOptions.DefaultDesktopOnly);
                    return null;
                }));

            // Object Specific
            SyncUiTask<ListObject> taskObjectSpecific = new SyncUiTask<ListObject>("Demo Object Specific Task",
                delegate(ListObject computer)
                {
                    // Show computer ID
                    MessageBox.Show(computer.Id, "", MessageBoxButtons.OK, MessageBoxIcon.Information, MessageBoxDefaultButton.Button1, MessageBoxOptions.DefaultDesktopOnly);
                    return null;
                });
            tasks.Add(taskObjectSpecific);

            return (tasks);
        }

        public override DetailGroup GetDetails(ListObject computer)
        {
            // You can cast computer to the specific object and get
            // more detailed information

            if (computer != null)
            {
                DetailGroup group = new DetailGroup("Computer detailed information");
                group.Add("Computer Id:", computer.Id);

                return group;
            }
            else
            {
                return null;
            }
        }
    }
}

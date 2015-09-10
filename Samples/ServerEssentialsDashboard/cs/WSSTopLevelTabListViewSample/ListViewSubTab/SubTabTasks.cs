//
//  <copyright file="SubTabTasks.cs" company="Microsoft">
//    Copyright (C) Microsoft. All rights reserved.
//  </copyright>
//

using System;
using Microsoft.WindowsServerSolutions.Administration.ObjectModel;
using System.Windows.Forms;

namespace ListViewSubTab
{
    static class SubTabTasks
    {
        public static TaskCollection CreateTasks()
        {
            TaskCollection tasks = new TaskCollection();

            tasks.Add(CreateCustomAction());
            tasks.Add(new ProcessTask<MyBusinessObject>("Selection Task", "notepad.exe"));

            return tasks;
        }

        private static AsyncUiTask CreateCustomAction()
        {
            var task = new AsyncUiTask("Custom action",
                delegate(object sender)
                {
                    MessageBox.Show("Custom action", "", MessageBoxButtons.OK, MessageBoxIcon.Information, MessageBoxDefaultButton.Button1, MessageBoxOptions.DefaultDesktopOnly);
                    return (null);
                });

            return (task);
        }
    }
}

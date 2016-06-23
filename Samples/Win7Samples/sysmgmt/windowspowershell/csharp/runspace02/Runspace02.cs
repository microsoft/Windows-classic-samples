//
// Copyright (c) 2006 Microsoft Corporation. All rights reserved.
// 
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
//
using System;
using System.Collections;
using System.Collections.ObjectModel;
using System.Windows.Forms;
using System.Management.Automation.Runspaces;
using System.Management.Automation;

namespace Microsoft.Samples.PowerShell.Runspaces
{
    using PowerShell = System.Management.Automation.PowerShell;

    class Runspace02
    {
        static void CreateForm()
        {
            Form form = new Form();
            DataGridView grid = new DataGridView();
            form.Controls.Add(grid);
            grid.Dock = DockStyle.Fill;

            // Create an instance of the PowerShell class.
            // This takes care of all building all of the other
            // data structures needed...
            PowerShell powershell = PowerShell.Create().AddCommand("get-process").AddCommand("sort-object").AddArgument("ID");
            if (Runspace.DefaultRunspace == null)
            {
                Runspace.DefaultRunspace = powershell.Runspace;
            }

            Collection<PSObject> results = powershell.Invoke();

            // The generic collection needs to be re-wrapped in an ArrayList
            // for data-binding to work...
            ArrayList objects = new ArrayList();
            objects.AddRange(results);

            // The DataGridView will use the PSObjectTypeDescriptor type
            // to retrieve the properties.
            grid.DataSource = objects;

            form.ShowDialog();
        }

        /// <summary>
        /// This sample uses the PowerShell class to execute
        /// the get-process cmdlet synchronously. Windows Forms and data
        /// binding are then used to display the results in a
        /// DataGridView control.
        /// </summary>
        /// <param name="args">Unused</param>
        /// <remarks>
        /// This sample demonstrates the following:
        /// 1. Creating an instance of the PowerShell class.
        /// 2. Using this instance to invoke a PowerShell command.
        /// 3. Using the output of PowerShell in a DataGridView
        ///    in a Windows Forms application 
        /// </remarks
        static void Main(string[] args)
        {
            Runspace02.CreateForm();
        }
    }
}





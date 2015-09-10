//
//  <copyright file="MyCustomWinformControl.cs" company="Microsoft">
//    Copyright (C) Microsoft. All rights reserved.
//  </copyright>
//

using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.Diagnostics;

namespace TopLevelTab
{
    public partial class MyCustomWinformControl : UserControl
    {
        private class ProcessInfo
        {
            private int _Id;
            public int Id
            {
                get
                {
                    return (_Id);
                }
            }

            private string _Name = "Unknown";
            public string Name
            {
                get
                {
                    return (_Name);
                }
            }

            public ProcessInfo(int id, string name)
            {
                _Id = id;
                _Name = name;
            }

            public override string ToString()
            {
                return (Name + " (" + Id + ")");
            }
        }

        private Random rand = new Random();

        public MyCustomWinformControl()
        {
            InitializeComponent();
            GetProcesses();
        }

        private void listBox1_SelectedIndexChanged(object sender, EventArgs e)
        {
            if (listBox1.SelectedIndex >= 0)
            {
                var details = "";
                try
                {
                    var processInfo = listBox1.SelectedItem as ProcessInfo;
                    var process = Process.GetProcessById(processInfo.Id);

                    details += "Process name: " + process.ProcessName;
                    details += Environment.NewLine + "Working set: " + process.WorkingSet64 + " bytes";
                    details += Environment.NewLine + "Main module: " + process.MainModule.FileName;
                }
                catch
                {
                    details = "Error retrieving informations";
                }
                textBox1.Text = details;
            }
        }

        private void button1_Click_1(object sender, EventArgs e)
        {
            GetProcesses();
        }

        private void GetProcesses()
        {
            listBox1.Items.Clear();
            var processes = Process.GetProcesses();

            foreach (var process in processes)
            {
                var processInfo = new ProcessInfo(process.Id, process.ProcessName);
                listBox1.Items.Add(processInfo);
            }
        }
    }
}

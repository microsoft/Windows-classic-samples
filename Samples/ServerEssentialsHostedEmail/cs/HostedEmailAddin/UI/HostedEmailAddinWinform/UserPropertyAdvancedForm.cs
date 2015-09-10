//
//  <copyright file="UserPropertyAdvancedForm.cs" company="Microsoft">
//    Copyright (C) Microsoft. All rights reserved.
//  </copyright>
//

using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.Threading;

using Microsoft.WindowsServerSolutions.HostedEmail;
using Contoso.EmailService;

namespace Contoso.HostedEmail.DashboardAddin
{
    public partial class UserPropertyAdvancedForm : Form
    {
        private const string KeyForwardEmail = "KeyForwardEmail";

        private const string KeyActiveSync = "KeyActiveSync";

        private string UserName { get; set; }

        private EmailAccountInfo EmailAccountInfo { get; set; }

        private HostedEmailManager manager = new HostedEmailManager(Constants.AdaptorId);

        public UserPropertyAdvancedForm(string user)
        {
            InitializeComponent();

            AcceptButton = this.buttonOK;
            CancelButton = this.buttonCancel;

            UserName = user;
        }

        public bool loadUserData()
        {
            bool ret = false;

            string forwardEmail = string.Empty;
            string activeSync = "False";

            using (ManualResetEvent done = new ManualResetEvent(false))
            {
                ThreadPool.QueueUserWorkItem((state) =>
                    {
                        if (!manager.Connected)
                        {
                            if (!manager.Connect()) ret = false;
                        }

                        try
                        {
                            EmailAccountInfo = manager.GetAccount(UserName);
                            if (null != EmailAccountInfo.ExtendedProperties)
                            {
                                EmailAccountInfo.ExtendedProperties.TryGetValue(KeyForwardEmail, out forwardEmail);
                                EmailAccountInfo.ExtendedProperties.TryGetValue(KeyActiveSync, out activeSync);
                            }
                        }
                        catch (Exception)
                        {
                            ret = false;
                        }

                        ret = true;
                        done.Set();
                    });
                done.WaitOne();
            }

            this.textBoxEmail.Text = forwardEmail;
            this.checkBoxActiveSync.Checked = bool.Parse(activeSync);
            return ret;
        }

        private void buttonOK_Click(object sender, EventArgs e)
        {
            if (null != EmailAccountInfo.ExtendedProperties)
            {
                EmailAccountInfo.ExtendedProperties[KeyForwardEmail] = this.textBoxEmail.Text;
                EmailAccountInfo.ExtendedProperties[KeyActiveSync] = this.checkBoxActiveSync.Checked.ToString();
            }

            bool err = false;

            using (ManualResetEvent done = new ManualResetEvent(false))
            {
                ThreadPool.QueueUserWorkItem((state) =>
                {
                    if (!manager.Connected)
                    {
                        if (!manager.Connect())
                        {
                            err = true;
                            done.Set();
                            return;
                        }
                    }

                    try
                    {
                        manager.UpdateAccount(UserName, EmailAccountInfo);
                    }
                    catch (Exception)
                    {
                        err = true;
                    }

                    done.Set();
                });
                done.WaitOne();
            }

            if (err) popupErrorMsg();
        }

        private void popupErrorMsg()
        {
            MessageBox.Show(Resources.ErrorMsg_UpdateFailed, "", MessageBoxButtons.OK, MessageBoxIcon.Error, MessageBoxDefaultButton.Button1, MessageBoxOptions.DefaultDesktopOnly);
        }
    }
}

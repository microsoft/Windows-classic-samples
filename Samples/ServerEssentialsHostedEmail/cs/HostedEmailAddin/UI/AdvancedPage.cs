//
//  <copyright file="AdvancedPage.cs" company="Microsoft">
//    Copyright (C) Microsoft. All rights reserved.
//  </copyright>
//

using System;
using System.Collections.Generic;
using System.Windows.Forms;
using System.Text;
using Microsoft.WindowsServerSolutions.Administration.ObjectModel.Adorners;
using Contoso.EmailService;

namespace Contoso.HostedEmail.DashboardAddin
{
    public class AdvancedPage : FormExtensionAdorner, IHostedEmailExtension
    {
        public AdvancedPage()
            : base(UIConstants.ContosoServicesSubTab,
                   Resources.ContosoServicesSubTab_DisplayName,
                   Resources.ContosoServicesSubTab_Description)
        { 
        }

        public override Form CreateForm(FormPropertyBag bag)
        {
            UserPropertyAdvancedForm ret = new UserPropertyAdvancedForm(null == bag ? string.Empty : bag.UserName);
            if ((null == bag) || string.IsNullOrEmpty(bag.UserName) || !ret.loadUserData())
            {
                MessageBox.Show(Resources.ContosoServicesSubTab_ErrorDescription, "", MessageBoxButtons.OK, MessageBoxIcon.Error, MessageBoxDefaultButton.Button1, MessageBoxOptions.DefaultDesktopOnly);
                return null;
            }
            else
            {
                return ret;
            }
        }

        public Guid GetAddinId()
        {
            return Constants.AdaptorId;
        }
    }
}

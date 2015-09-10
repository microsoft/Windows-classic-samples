//
//  <copyright file="TopLevelTab.cs" company="Microsoft">
//    Copyright (C) Microsoft. All rights reserved.
//  </copyright>
//

using System.Drawing;
using Microsoft.WindowsServerSolutions.Administration.ObjectModel;

namespace Contoso.HostedEmail.DashboardAddin
{
    [ContainsCustomControl] // Required attribute for custom tabs
    public class TopLevelTabPageProvider : PageProvider
    {
        public TopLevelTabPageProvider()
            : base(UIConstants.ContosoServicesTopLevelTab,
                   Resources.ContosoServicesTopLevelTab_DisplayName,
                   Resources.ContosoServicesTopLevelTab_Description)
        {
        }

        protected override Icon CreateImage()
        {
            return SystemIcons.Information;
        }

        protected override object CreatePages()
        {
            return (new object[] { new SubTabPage() });
        }
    }
}

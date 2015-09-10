//
//  <copyright file="SubTabPage.cs" company="Microsoft">
//    Copyright (C) Microsoft. All rights reserved.
//  </copyright>
//

using Microsoft.WindowsServerSolutions.Administration.ObjectModel;

namespace Contoso.HostedEmail.DashboardAddin
{
    public class SubTabPage : Page
    {
        public SubTabPage()
            : base(UIConstants.ContosoServicesSubTab,
                   Resources.ContosoServicesSubTab_DisplayName,
                   Resources.ContosoServicesSubTab_Description)
        { 
        }

        protected override PageContent CreateContent()
        {
            return PageContent.Create((content, owner) => new SampleAddinForm(), GlobalTasks.CreateGlobalTasks(), null);
        }

    }
}

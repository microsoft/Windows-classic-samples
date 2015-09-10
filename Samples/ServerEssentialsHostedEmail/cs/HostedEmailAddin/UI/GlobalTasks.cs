//
//  <copyright file="GlobalTasks.cs" company="Microsoft">
//    Copyright (C) Microsoft. All rights reserved.
//  </copyright>
//

using System;
using System.IO;
using System.Runtime.CompilerServices;
using Microsoft.WindowsServerSolutions.Administration.ObjectModel;
using Microsoft.WindowsServerSolutions.HostedEmail;
using Microsoft.WindowsServerSolutions.Common;
using Contoso.EmailService;

namespace Contoso.HostedEmail.DashboardAddin
{
    internal static class GlobalTasks
    {
        private static TaskCollection globalTasks = null;

        private static bool ContosoEmailEnabled
        {
            get
            {
                return (HostedEmailIntegrationManager.IsEnabled() && HostedEmailIntegrationManager.EnabledAddinId.Equals(Constants.AdaptorId));
            }
        }

        [MethodImpl(MethodImplOptions.Synchronized)]
        internal static TaskCollection CreateGlobalTasks()
        {
            if (globalTasks == null)
            {
                globalTasks = new TaskCollection();
                globalTasks.Add(CreateChangeAdminAccountTask());
                globalTasks.Add(CreateDisableServicesTask());
                globalTasks.Add(CreateVisitMyAccountTask());
            }
            return globalTasks;
        }

        private static AsyncUiTask CreateVisitMyAccountTask()
        {
            AsyncUiTask task = new AsyncUiTask(
                UIConstants.VisitMyAccountTask,
                Resources.ContosoServicesVisitMyAccountTask_DisplayName,
                delegate(object obj)
                {
                    Uri userPortal = null; 
                    if (ContosoEmailEnabled)
                    {
                        userPortal = HostedEmailIntegrationManager.Configuration.Service.ServiceUserPortal;
                    }                 
                    if(userPortal == null)
                    {

                        userPortal = new Uri(Resources.ContosoServices_DefaultUserPortal);
                    }
                    System.Diagnostics.Process.Start(userPortal.ToString());

                    return null;
                },
                true //global task
            );
            return task;
        }

        private static AsyncUiTask CreateChangeAdminAccountTask()
        {
            AsyncUiTask task = new AsyncUiTask(
                UIConstants.ChangeAdminAccountTask,
                Resources.ContosoServicesChangeAdminAccountTask_DisplayName,
                delegate(object obj)
                {
                    //don't need to check if addin Id is valid since it has been checked when tab loaded
                    string configureWizardResetRelativePath = "Wssg.HostedEmailConfigureWizard.Reset.vbs";
                    System.Diagnostics.Process.Start(configureWizardResetRelativePath);
                    return null;
                },
                true //global task
            );
            task.ConditionProvider = ChangeAdminAccountCondition;
            return task;
        }

        private static AsyncUiTask CreateDisableServicesTask()
        {
            AsyncUiTask task = new AsyncUiTask(
                UIConstants.DisableServicesTask,
                Resources.ContosoServicesDisableServicesTask_DisplayName,
                delegate(object obj)
                {
                    string configureWizardRelativePath = "Wssg.HostedEmailConfigureWizard.vbs";
                    System.Diagnostics.Process.Start(configureWizardRelativePath);
                    return null;
                },
                true //global task
            );
            task.ConditionProvider = DisableServicesCondition;
            return task;
        }

        private static TaskCondition ChangeAdminAccountCondition(object obj)
        {
            return ContosoEmailEnabled ? TaskCondition.Normal : TaskCondition.NotApplicable;
        }

        private static TaskCondition DisableServicesCondition(object obj)
        {
            return ContosoEmailEnabled ? TaskCondition.Normal : TaskCondition.NotApplicable;
        }
    }
}
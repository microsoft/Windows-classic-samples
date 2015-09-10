//
//  <copyright file="SyncQuickStatus.cs" company="Microsoft">
//    Copyright (C) Microsoft. All rights reserved.
//  </copyright>
//

using System;
using System.Globalization;
using Microsoft.WindowsServerSolutions.Dashboard.Addins.Home;

namespace WSSHomepageQuickStatus
{
    /// <summary>
    /// The Sample class demonstrates how to implement ITaskStatusQuery
    /// synchronously. Simply derive from helper class SyncTaskStatusQuery
    /// and implement QueryTaskStatus and everything is done.
    /// </summary>
    public class SyncQuickStatus : SyncTaskStatusQuery
    {
        public SyncQuickStatus()
        {
        }

        /// <summary>
        /// All query logic goes here, and synchronously.
        /// </summary>
        /// <remarks>
        /// It is possible this routine will be aborted if the query logic takes
        /// too long to complete.
        /// </remarks>
        /// <returns>Queried result</returns>
        protected override TaskQuickStatus QueryTaskStatus()
        {
            return new TaskQuickStatus()
            {
                Title = Resources.SyncStatusTitle,
                Details = Resources.SyncStatusDetails,
                Mark = DateTime.Now.ToString(CultureInfo.CurrentCulture),
                StatusTips = String.Empty
            };
        }
    }
}

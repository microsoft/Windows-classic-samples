//
//  <copyright file="CancellableAsyncQuickStatus.cs" company="Microsoft">
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
    /// synchronously with a cancellation token.
    /// </summary>
    public class CancellableAsyncQuickStatus : CancellableTaskStatusQuery
    {
        private const int COUNT = 10;

        private const int WAIT_TIMEOUT = 100;

        public CancellableAsyncQuickStatus()
        {
        }

        /// <summary>
        /// Implement all the query logic in this method.
        /// </summary>
        /// <remarks>
        /// The cancellationToken must be checked when it is safe to cancel the
        /// operation. The blocking logic between two check points for cancellationToken
        /// should run as quick as possible. The logic will still be aborted if it takes too
        /// long after cancellationToken is canceled but no cancellation work is actually performed.
        /// The cancellation work can be simply return from the method or throw an OperationCanceledException.
        /// </remarks>
        /// <param name="cancellationToken">The cancellationToken to be checked.</param>
        /// <returns>Queried result</returns>
        protected override TaskQuickStatus QueryStatus(System.Threading.CancellationToken cancellationToken)
        {
            int count;
            for (count = 0; count < COUNT; count++)
            {
                // Check cancellationToken for the canceled state.
                cancellationToken.ThrowIfCancellationRequested();

                // TODO: do real work, keep it as quick as possible.
                // Here, simply uses a timed wait to make it look like working on something.
                cancellationToken.WaitHandle.WaitOne(WAIT_TIMEOUT);
            }

            // Check the cancellationToken again
            cancellationToken.ThrowIfCancellationRequested();

            return new TaskQuickStatus()
            {
                Mark = count.ToString(CultureInfo.CurrentCulture),
                StatusTips = String.Empty
            };
        }
    }
}

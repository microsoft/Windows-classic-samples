// <copyright file="ClearTransactedCommentCommand.cs" company="Microsoft Corporation">
// Copyright (c) 2009 Microsoft Corporation. All rights reserved.
// </copyright>
// DISCLAIMER OF WARRANTY: The software is licensed “as-is.” You 
// bear the risk of using it. Microsoft gives no express warranties, 
// guarantees or conditions. You may have additional consumer rights 
// under your local laws which this agreement cannot change. To the extent 
// permitted under your local laws, Microsoft excludes the implied warranties 
// of merchantability, fitness for a particular purpose and non-infringement.

namespace TransactedComment
{
  using System.Management.Automation;
  
  /// <summary>
  /// Contains the implementation of the Clear-TransactedComment cmdlet.
  /// </summary>
  /// <remarks>
  /// The SupportsTransactions parameter of the Cmdlet attribute allows 
  /// a user to specify the UseTransaction parameter, which causes the 
  /// cmdlet to participate in the active transaction.
  /// </remarks>
  [Cmdlet("Clear", "TransactedComment", SupportsTransactions = true)]
  public class ClearTransactedCommentCommand : PSCmdlet
  {
    /// <summary>
    /// Implementation of the cmdlet.
    /// </summary>
    protected override void ProcessRecord()
    {
      // The comments are stored in the PrivateData property of the
      // module. This provides an isolated storage area associated 
      // with this module and runspace, protecting it from interferance 
      // from other aspects of the user's session.
      TransactedComment current = this.MyInvocation.MyCommand.Module.PrivateData as TransactedComment;
      if (current == null)
      {
        return;
      }

      // Check to see if the UseTransaction parameter was specified. If it 
      // was not, this.CurrentPSTransaction is null so it can not be in a 
      // using statement. Checking this first allows the cmdlet to be used 
      // within and outside of a transaction. In principle, a cmdlet could  
      // do different things depending on the transactional context, but
      // this cmdlet just does pretty much the same thing in either
      // situation because TransactedComment already handles it differently.
      if (this.TransactionAvailable())
      {
        // Within this using statement, PowerShell exposes the active
        // transaction as Transaction.Current, allowing standard .NET
        // transactional resource managers (such as TransactedComment)
        // to participate.
        using (this.CurrentPSTransaction)
        {
          current.Clear();
        }
       }
       else
       {
         // Otherwise, act on the live version of the TransactedComment.
         current.Clear();
       }
     }
   }
}

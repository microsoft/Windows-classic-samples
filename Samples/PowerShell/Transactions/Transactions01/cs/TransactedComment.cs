// <copyright file="TransactedComment.cs" company="Microsoft Corporation">
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
  using System;
  using System.Collections.Generic;
  using System.Linq;
  using System.Text;
  using System.Transactions;

  /// <summary>
  /// A transaction-aware store for building strings. It supports appending
  /// and inspecting. 
  /// </summary>
  /// <remarks>
  /// The <see cref="System.Transactions.IEnlistmentNotification"/> interface
  /// provides callbacks for the transaction system. This is a simple 
  /// example of a transactional resource manager. If your current resource
  /// manager already implements <see cref="System.Transactions.IEnlistmentNotification"/>,
  /// you probably do not need to write one yourself.
  /// </remarks>
  public class TransactedComment : IEnlistmentNotification
  {
    /// <summary>
    /// Stores the official value of the comment.
    /// </summary>
    private StringBuilder commentValue;

    /// <summary>
    /// Stores the temprary value of the comment inside the transaction.
    /// </summary>
    private StringBuilder temporaryValue;
      
    /// <summary>
    /// enlistedTransaction is the one that has the right to affect
    /// this TransactedComment, and to see the temporary value. If 
    /// it is set, then attempting to alter the TransactedComment from
    /// outside a transaction or inside a different transaction is an error.
    /// </summary>
    private Transaction enlistedTransaction = null;

    /// <summary>
    /// Initializes a new instance of the TransactedComment class. This is 
    /// the default constructor.
    /// </summary>
    public TransactedComment()
        : this(string.Empty)
    {
    }

    /// <summary>
    /// Initializes a new instance of the TransactedComment class.
    /// </summary>
    /// <param name="value">The initial value of the comment.</param>
    public TransactedComment(string value)
    {
        this.commentValue = new StringBuilder(value);
        this.temporaryValue = null;
    }

    /// <summary>
    /// Gets the length of the transacted string. If this is
    /// called within the transaction, it returns the length of
    /// the transacted value. Otherwise, it returns the length of
    /// the original value.
    /// </summary>
    public int Length
    {
      get
      {
        // If not in a transaction, or in a different transaction 
        // than the one we are enlisted in, return the publicly
        // visible state.
        if (
             (Transaction.Current == null) ||
             (this.enlistedTransaction != Transaction.Current))
        {
          return this.commentValue.Length;
        }
        else
        {
          return this.temporaryValue.Length;
        }
      }
    }
 
    /// <summary>
    /// Make the transacted changes permanent.
    /// </summary>
    /// <param name="enlistment">The enlistment for the open tranacstion.</param>
    void IEnlistmentNotification.Commit(Enlistment enlistment)
    {
      // This copies the value to the permanent version and wipes out
      // any temporary information, then informs the enlistment that we are
      // all set.
      this.commentValue = new StringBuilder(this.temporaryValue.ToString());
      this.temporaryValue = null;
      this.enlistedTransaction = null;
      enlistment.Done();
    }

    /// <summary>
    /// Discard the transacted changes.
    /// </summary>
    /// <param name="enlistment">The enlistment for the open tranacstion.</param>
    void IEnlistmentNotification.Rollback(Enlistment enlistment)
    {
      this.temporaryValue = null;
      this.enlistedTransaction = null;
      enlistment.Done();
    }

    /// <summary>
    /// Discard the transacted changes.
    /// </summary>
    /// <param name="enlistment">The enlistment for the open tranacstion.</param>
    void IEnlistmentNotification.InDoubt(Enlistment enlistment)
    {
      enlistment.Done();
    }

    /// <summary>
    /// Determine if the transaction can be committed.
    /// </summary>
    /// <param name="preparingEnlistment">A PreparingEnlistment object used 
    /// to send a response to the transaction manager.</param>
    void IEnlistmentNotification.Prepare(PreparingEnlistment preparingEnlistment)
    {
      preparingEnlistment.Prepared();
    }

    /// <summary>
    /// Append text to the transacted string.
    /// </summary>
    /// <param name="text">The text to append.</param>
    public void Append(string text)
    {
      // Make sure that we are in a sensible transaction context.
      this.ValidateTransactionOrEnlist();

      if (this.enlistedTransaction != null)
      {
        // If ValidateTransactionOrEnlist did not throw an error and
        // left a value in enlistedTransaction, then we are in the right
        // transaction and should act on the temporary value.
        this.temporaryValue.Append(text);
      }
      else
      {
        // If we are not in a transaction and this TransactedComment has 
        // never been manipulated inside a transaction since its last Commit, 
        // just act on the permanent value.
        this.commentValue.Append(text);
      }
    }

    /// <summary>
    /// Remove all text from the string.
    /// </summary>
    public void Clear()
    {
      // Make sure that we are in a sensible transaction context.
      this.ValidateTransactionOrEnlist();

      if (this.enlistedTransaction != null)
      {
        // If ValidateTransactionOrEnlist did not throw an error and left 
        // a value in enlistedTransaction, then we are in the right
        // transaction and should act on the temporary value.
        this.temporaryValue = new StringBuilder();
      }
      else
      {
        // If we are not in a transaction and this TransactedComment has 
        // never been manipulated inside a transaction since its last Commit, 
        // act on the permanent value.
        this.commentValue = new StringBuilder();
      }
    }

    /// <summary>
    /// Gets the System.String that represents the transacted
    /// transacted string. If this is called within the
    /// transaction, it returns the transacted value.
    /// Otherwise, it returns the original value.
    /// </summary>
    /// <returns>A System.String that represents the transacted 
    /// transacted string</returns>
    public override string ToString()
    {
      // If we are not in a transaction, or we are in a different transaction 
      // than the one we enlisted in, return the publicly visible state.
      if (
          (Transaction.Current == null) ||
          (this.enlistedTransaction != Transaction.Current))
      {
        return this.commentValue.ToString();
      }
      else
      {
        return this.temporaryValue.ToString();
      }
    }
    
    /// <summary>
    /// Test if we are in a transaction and enlist if needed. 
    /// </summary>
    private void ValidateTransactionOrEnlist()
    {
      // We are in a transaction. The using (CurrentPSTransaction) block 
      // in the cmdlet caused Transaction.Current to be set correctly, so 
      // we can test against it here.
      if (Transaction.Current != null)
      {
        // We have not yet been called inside of a transaction. So enlist
        // in the transaction, and store our save point.
        if (this.enlistedTransaction == null)
        {
          Transaction.Current.EnlistVolatile(this, EnlistmentOptions.None);
          this.enlistedTransaction = Transaction.Current;
          this.temporaryValue = new StringBuilder(this.commentValue.ToString());
        }
        else
        {
          // We are already enlisted in a transaction.
          if (Transaction.Current != this.enlistedTransaction)
          {
            throw new InvalidOperationException("Cannot modify string. It has been modified by another transaction.");
          }
        }
      }
      else
      {
        // We are not in a transaction.
        // If we are not subscribed to a transaction, modify the underlying value.
        if (this.enlistedTransaction != null)
        {
          throw new InvalidOperationException("Cannot modify string. It has been modified by another transaction.");
        }
      }
    }
  }
}

// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
//  Copyright (c) Microsoft Corporation. All rights reserved.
//
//	This file contains global methods and properties of the 
//	WebExpense application
//
//	Carolyn Van Slyck 06/2003 - Created
//  DaveMM - Updates 06/2005 - Tweaks, Updates, Fixes for SDK
//	Revision: v1.0

using System;
using System.Collections;
using System.Collections.Specialized;
using System.Runtime.InteropServices;
using System.Web;
using System.Web.UI;
using System.Web.UI.WebControls;
using System.Web.UI.HtmlControls;
using Microsoft.Interop.Security.AzRoles;

//
//	Application Namespace
//
namespace WebExpense 
{
    /// <summary>
    /// ExpenseCommon Class - contains public methods and properties that
    /// are used throughout the application to use Authorization Manager
    /// and to manipulate the expense reports
    /// </summary>
    public class ExpenseCommon 
    {
        private ExpenseCommon()
        {
        }

        #region Constants
        /// <summary>
        /// Submit Operation ID Number
        ///	The number defined in the authorization policy store
        ///	which is assoicated with the submit expense operation
        /// </summary>
        public const int AzopSubmit = 1;


        /// <summary>
        /// Approve Opertaion ID Number
        /// The number defined in the authorization policy store
        /// which is associated with the approve expense operation
        /// </summary>
        public const int AzopApprove = 2;


        /// <summary>
        /// Read Opertaion ID Number
        /// The number defined in the authorization policy store
        /// which is associated with the read expense operation
        /// </summary>
        public const int AzopRead = 3;


        /// <summary>
        /// List Opertaion ID Number
        /// The number defined in the authorization policy store
        /// which is associated with the list expenses operation
        /// </summary>
        public const int AzopList = 4;


        /// <summary>
        /// Administer Operation ID Number
        ///	The number defined in the authorization policy store
        ///	which is assoicated with the administer application 
        ///	settings operation
        /// </summary>
        public const int AzopAdministrater = 5;


        /// <summary>
        /// Self Approval Parameter
        /// The string which represents the business rule parameter
        /// for Self Approval
        /// </summary>
        public const string ParamSelfApproval = "SelfApproval";


        /// <summary>
        /// Maximum Number of Transactions Parameter
        /// The string which represents the business rule parameter
        /// for Maximum Number of Transactions
        /// </summary>
        public const string ParamMaxTrans = "MaxTrans";


        /// <summary>
        /// User Name Parameter - 
        /// The string which represents the business rule parameter
        /// for a User Name
        /// </summary>
        public const string ParamUserName = "UserName";


        /// <summary>
        /// Approver Name Parameter - 
        /// The string which represents the business rule parameter
        /// for an Approver User Name
        /// </summary>
        public const string ParamApproverName = "ApproverName";


        /// <summary>
        /// Date Parameter - 
        /// The string which represents the business rule parameter
        /// for a Date
        /// </summary>
        public const string ParamDate = "Date";


        /// <summary>
        /// Amount Parameter - 
        /// The string which represents the business rule parameter
        /// for an Expense Amount
        /// </summary>
        public const string ParamAmount = "Amount";

        /// <summary>
        /// No Error - HRESULT, returned by AccessCheck when
        /// an access check succeeds
        /// </summary>
        public const int NoError = 0;


        /// <summary>
        /// Access Denied - HRESULT, returned by AccessCheck
        /// when an access check fails due to user does not have
        /// sufficient permissions
        /// </summary>
        public const int AccessDenied = 5;


        /// <summary>
        /// Transaction Prefix for all transactions
        /// </summary>
        private const string DataSourceTransPrefix = "Expense.Trans.";


        /// <summary>
        /// Transaction Status Suffix for all transactions
        /// </summary>
        private const string DataSourceTransPrefixStatus = ".status";


        /// <summary>
        /// Transaction Date Suffix for all transactions
        /// </summary>
        private const string DataSourceTransPrefixDate = ".date";

        #endregion

        /// <summary>
        /// Updates the cache of the policy store
        /// See SDK documentation for UpdateCache.
        /// </summary>
        public static void UpdateCache()
        {
            //	Update the cache
            ((AzAuthorizationStoreClass)HttpContext.Current.Application["AZMAN_STORE"]).UpdateCache(null);
            

            //
            //	Get a new client context
            //
            IAzClientContext AzManClientContext;

            //
            //	Create the client context from the user's token
            //
            //	ClientContext can be initialized from either a user's account name (InitializeClientContextFromName), 
            //	a user's token (InitializeClientContextFromToken) or from a string SID (InitializeClientContextFromStringSid)
            //
            HandleRef token = new HandleRef(new object(), ((HttpWorkerRequest)((IServiceProvider)HttpContext.Current).GetService(typeof(HttpWorkerRequest))).GetUserToken());
            AzManClientContext = ((IAzApplication)HttpContext.Current.Application["AZMAN_APP"]).InitializeClientContextFromToken((UInt64)token.Handle, 0);

            //
            //	Save the client context in a session variable
            //
            HttpContext.Current.Session["AZMAN_CLIENT"] = AzManClientContext;
        }

        /// <summary>
        /// Initializes the WebExpense application to use the 
        /// authorization policy and settings defined in Global.asax.cs
        /// </summary>
        /// <returns>Returns any errors encountered in the initialization process
        /// or null if no errors occured</returns>
        public static bool Initialize()
        {
            //
            //	WebExpense Application object
            //
            IAzApplication AzManApp;
            
            //
            //	WebExpense application store object
            //
            AzAuthorizationStoreClass AzManStore;

            string RtnMsg;



            //
            //	Create a new Authorization Manager Store object
            //
            AzManStore = new AzAuthorizationStoreClass();

            try
            {
                //
                //	Open the Authorization Manager policy store from the path specified the in
                //	the application variable AZMAN_STORE.  This variable
                //	is initially specified in Global.asax.cs
                //
                AzManStore.Initialize(0, (string)HttpContext.Current.Application["STORE_PATH"], null);

            }
            catch (System.IO.FileNotFoundException)
            {
                // IAzAuthorizationStore.Initialize failed w/ File Not Found. This can happen if the
                // store URL is bad.
                throw; // Since we can't proceed we'll throw to the default handler
            }
            catch (System.UnauthorizedAccessException)
            {
                // IAzAuthorizationStore.Initialize failed w/ AccessDenied. Make sure that the calling
                // context is in the Reader (or Admin if the app needs to write) role on the AzMan store 
                throw; // Since we can't proceed we'll throw to the default handler
            }

            //
            //	Save the policy store in a session variable	
            //
            HttpContext.Current.Application["AZMAN_STORE"] = AzManStore;

            try
            {
                //
                //	Open the application specified in the application variable
                //	AZMAN_APP_NAME, This variable is initially specified in Global.asax.cs
                //
                AzManApp = AzManStore.OpenApplication ((string)HttpContext.Current.Application["AZMAN_APP_NAME"], null);
            }
            catch (System.IO.FileNotFoundException)
            {
                // IAzAuthorizationStore.OpenApplication failed w/ File Not Found. This can happen if the
                // application name is not correct (no corresponding application in the store).
                throw; // Since we can't proceed we'll throw to the default handler
            }

            //
            //	Save the application to a session variable
            //
            HttpContext.Current.Application["AZMAN_APP"] = AzManApp;


            //
            //	Return initialization suceeded
            //
            return true;
        }

        /// <summary>
        /// Get Client SAM Name
        /// </summary>
        /// <returns>Returns the the client SAM name (\\domain\user)</returns>
        public static string GetClientSamName() 
        {
            return((string)HttpContext.Current.Session["CLIENT_SAM_NAME"]);
        }

        /// <summary>
        /// Get Client Context
        /// </summary>
        /// <returns>Returns the current user's client context</returns>
        public static IAzClientContext3 GetAzClientContext()
        {
            //	Update any cached data
            UpdateCache();

            return((IAzClientContext3)HttpContext.Current.Session["AZMAN_CLIENT"]);
        }

        
        /// <summary>
        /// Gets the Self Approval setting - whether or not approvers can approve
        /// their own expenses
        /// </summary>
        /// <returns>True - Self approval allowed, False - Self approval prohibited</returns>
        public static bool GetSelfApproval()
        {
            return((bool)HttpContext.Current.Application["SELF_APPROVAL"]);
        }

        /// <summary>
        /// Sets Self Approval setting - whether or not approvers can approve
        /// their own expenses
        /// </summary>
        /// <param name="setting">True - Self approval allowed, False - Self approval prohibited</param>
        public static void SetApproval (bool setting )
        {
            HttpContext.Current.Application["SELF_APPROVAL"] = setting;
        }

        #region Data Store Methods
        //	Data persistence is outside the scope of this sample
        //	For simplicity this sample uses application variables 
        //	to store the demo data.  This is volitile memory and should not
        //	be used for live applications.


        /// <summary>
        /// Gets the status of a transaction, i.e. APPROVED, REJECTED, PENDING...
        /// </summary>
        /// <param name="transID">ID of the transaction to query</param>
        /// <returns>Returns the transaction status</returns>
        public static string GetTransactionStatus (int transID )
        { 
            return ((string)HttpContext.Current.Application[string.Concat(DataSourceTransPrefix, transID.ToString(), DataSourceTransPrefixStatus)]);
        }

        /// <summary>
        /// Approves the specified transaction
        /// </summary>
        /// <param name="transID">ID of the transaction to approve</param>
        public static void ApproveTransaction (int transID )
        {
            //
            //	Set the approval date
            //
            HttpContext.Current.Application[string.Concat(DataSourceTransPrefix, transID.ToString(),DataSourceTransPrefixDate)] = DateTime.Now.ToShortDateString();
            
            //
            //	Set the transaction as APPROVED
            //
            HttpContext.Current.Application[string.Concat(DataSourceTransPrefix, transID.ToString(),DataSourceTransPrefixStatus)] = "APPROVED";
        }

        /// <summary>
        /// Rejects the specified transaction
        /// </summary>
        /// <param name="transID">ID of the transaction to reject</param>
        public static void RejectTransaction (int transID)
        {
            //
            //	Set the rejection date
            //
            HttpContext.Current.Application[string.Concat(DataSourceTransPrefix, transID.ToString(),DataSourceTransPrefixDate)] = DateTime.Now.ToShortDateString();
            
            //
            //	Set the transaction as REJECTED
            //
            HttpContext.Current.Application[string.Concat(DataSourceTransPrefix, transID.ToString(),DataSourceTransPrefixStatus)] = "REJECTED";
        }

        /// <summary>
        /// Get the time that a transaction was approved or rejected 
        /// </summary>
        /// <param name="transID">ID of the transaction to query</param>
        /// <returns></returns>
        public static string GetTransactionDecisionTime(int transID )
        {
            return((string)HttpContext.Current.Application[string.Concat(DataSourceTransPrefix, transID.ToString(), DataSourceTransPrefixDate)]);
        }

        /// <summary>
        /// Increments the next available transaction number for the application
        /// </summary>
        /// <returns>Next available transaction number</returns>
        public static int AssignNextTransaction() 
        {
            //
            //	Checks if the number of transactions has reached the maximum allowed for the sample
            //	if max_trans = 0, then there is no preset max number of transactions
            //
            int max_trans = GetMaxTransaction();
            if( (max_trans!=0) && ((int)HttpContext.Current.Application[ "DATASTORE_LASTTRANS" ]  >= max_trans)) 
            {
                ClearTrans();
            }

            //
            //	Lock the application
            //
            HttpContext.Current.Application.Lock();

            //
            //	Sets the next available transaction number
            //
            HttpContext.Current.Application[ "DATASTORE_LASTTRANS" ] = (int)HttpContext.Current.Application[ "DATASTORE_LASTTRANS" ] + 1;
            
            //
            //	Unlock the application
            //
            HttpContext.Current.Application.UnLock();

            //
            //	Return the next available transaction number
            //
            return((int)HttpContext.Current.Application[ "DATASTORE_LASTTRANS" ]);
        }


        /// <summary>
        ///	Gets total number of transactions in the application
        /// </summary>
        /// <returns>Total number of transactions</returns>
        public static int GetNextTransaction()
        {
            return((int)HttpContext.Current.Application[ "DATASTORE_LASTTRANS" ]);
        }

        /// <summary>
        /// Saves a transaction in an application variable
        /// </summary>
        /// <param name="intNextIdNumber">ID number to assign to the transaction</param>
        /// <param name="TransData">Transaction Data</param>
        public static void SaveTransaction (int intNextIdNumber, StringDictionary TransData )
        {
            HttpContext.Current.Application[string.Concat(DataSourceTransPrefix, intNextIdNumber.ToString())] =  TransData;
        }

        /// <summary>
        /// Gets the data from a transaction
        /// </summary>
        /// <param name="transID">ID of the transaction to query</param>
        /// <returns>Transaction data</returns>
        public static StringDictionary GetTransData (int transID )
        {
            return((StringDictionary)HttpContext.Current.Application[string.Concat(DataSourceTransPrefix, transID.ToString())]);
        }

        /// <summary>
        /// Clears all application variables which contain application transactions
        /// </summary>
        public static void ClearTrans()
        {
            //
            //	Total number of transactions
            //
            int count = GetNextTransaction(); 

            for( int i = 1; i <= count; i++) 
            {
                //
                //	Remove all application variables associted with the current transaction
                //
                HttpContext.Current.Application[ string.Concat(DataSourceTransPrefix, i.ToString(), DataSourceTransPrefixDate) ] = "";
                HttpContext.Current.Application[ string.Concat(DataSourceTransPrefix, i.ToString(), DataSourceTransPrefixStatus) ] = "";
                HttpContext.Current.Application[ string.Concat(DataSourceTransPrefix, i.ToString()) ] = "";
                HttpContext.Current.Application[ "DATASTORE_LASTTRANS" ] = 0;
            } 
        }

        /// <summary>
        /// Gets the maximum number of transactions that can be created before the 
        /// demo resets itself and clears it's data store
        /// </summary>
        /// <returns>Max transactions</returns>
        public static int GetMaxTransaction()
        {
            return((int)HttpContext.Current.Application["DATASTORE_MAXTRANS"]);
        }

        /// <summary>
        /// Sets the maximum number of transactions that can be created before the 
        /// demo resets itself and clears it's data store
        /// </summary>
        /// <param name="max">Max transactions</param>
        public static void SetMaxTransaction (int max )
        {
            HttpContext.Current.Application["DATASTORE_MAXTRANS"] = max;
        }

        #endregion
    }
}

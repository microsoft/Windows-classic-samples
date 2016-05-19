// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
//  Copyright (c) Microsoft Corporation. All rights reserved.
//
//	This page can list all expenses in the application
//	data store and sort them by their status, i.e. approved, rejected, etc.
//
//
//	Carolyn Van Slyck 06/2003 - Created
//  DaveMM - Updates 06/2005 - Tweaks, Updates, Fixes for SDK



using System;
using System.Collections;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Web;
using System.Web.SessionState;
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
    /// List Page Class
    ///	Contains all methods, events and properties of the List page
    /// </summary>
    public partial class List : System.Web.UI.Page
    {
        #region Page Variables
        /// <summary>
        /// Message Label - Displays any messages for the user
        /// </summary>

        /// <summary>
        /// Transaction List - Displays the transactions
        /// filtered by the ModeSelect drop down listbox
        /// </summary>

        /// <summary>
        /// Mode Select Drop Down ListBox - Filters which
        /// expense transactions are displayed in the transaction list
        /// </summary>

        /// <summary>
        /// Mode Label - Displays the filter on the transaction list
        /// </summary>

        /// <summary>
        /// Logo Link - Displays the company logo and links
        /// back to the main page of the application
        /// </summary>

        /// <summary>
        /// Title Label - Displays the page title
        /// </summary>

        /// <summary>
        /// Return Link - Displays a link back to the main
        /// page of the application
        /// </summary>

        #endregion

        /// <summary>
        /// Page Load - This is executed when the page is first requested
        /// by the user and additionally when the user clicks a button on
        /// the form
        /// </summary>
        protected void Page_Load(object sender, System.EventArgs e)
        {
            //
            //	To prevent users from by-passing the portal page (index.aspx)
            //	and going directly to this page, use URL Authorization
            //	See <url> for details.
            //

            //		
            //	Check for this is the first time the page is being loaded
            //	only fill in the form if this is the first time otherwise
            //	any user changes will be lost
            //
            if(!Page.IsPostBack)
            {


                //
                //	Check if the user has permission to list expenses
                //

                //
                //
                //	Get the client context from the session variables
                //
                IAzClientContext3 AzClient = ExpenseCommon.GetAzClientContext();

                //
                // Set BizRule Parameters
                //
                IAzBizRuleParameters BizRuleParams = AzClient.BizRuleParameters;
                BizRuleParams.AddParameter("Amount", 0);
                BizRuleParams.AddParameter("Date", DateTime.Now.ToShortDateString());
                BizRuleParams.AddParameter("SubmitterName", "");
                BizRuleParams.AddParameter("UserName", ExpenseCommon.GetClientSamName());

                //
                //	Run the access check on the submit operation
                //	Passing the audit text, scope, operations and business rule parameters
                //
                uint result = AzClient.AccessCheck2("List Expense Reports", "", ExpenseCommon.AzopList);

                //
                //	Check for success of the access check
                //
                bool bAuthorized = false;

                if (result == ExpenseCommon.NoError)
                {
                    bAuthorized = true;
                }

                else if (result == ExpenseCommon.AccessDenied)
                {
                    string errorMessage = AzClient.GetBusinessRuleString();
                    if (errorMessage != "")
                    {
                        MSG.Text = "<font color=\"FF0000\">Access Denied." + errorMessage + "</font>";
                    }
                    else
                    {
                        MSG.Text = "<font color=\"FF0000\">Access Denied.  You do not have sufficient permissions to perform this operation.</font>";
                    }
                    bAuthorized = false;
                }
                else
                {
                    //
                    //	Check for other error
                    //
                    if (result != ExpenseCommon.NoError)
                    {
                        Win32Exception ex = new Win32Exception();
                        MSG.Text = "<font color=\"FF0000\">There was an error performing the AccessCheck: " + ex.Message + "</font>";
                    }
                }

                if(bAuthorized)
                {
                    //
                    //	List the expense reports
                    //
                    ListTransactions();
                }
                else
                {
                    //
                    //	Access Check failed so display an error message to the user
                    //
                    MSG.Text = "Error Access Denied: " + AzClient.GetBusinessRuleString();
                    return;
                }
            }
        }

        /// <summary>
        /// Lists all transactions in the application data store
        /// according to the filter set by the ModeSelect drop down listbox
        /// </summary>
        private void ListTransactions()
        {
            //
            //	remove previous transactions from list
            //
            TransList.Rows.Clear();

            //
            //	Create the header row of the table
            //
            TableRow trow = new TableRow();
            trow.BackColor = System.Drawing.Color.LightSteelBlue;
            trow.Font.Bold = true;
            TableCell tcell = new TableCell();
            tcell.Text = "Select an expense";
            trow.Cells.Add(tcell);
            tcell = new TableCell();
            tcell.Text = "Status";
            trow.Cells.Add(tcell);
            TransList.Rows.Add(trow);

            //
            //	Get the number of transactions in the 
            //	application data store
            //
            int numTrans = ExpenseCommon.GetNextTransaction();
            
            //
            //	Check for a valid number of transactions
            //
            if(numTrans > 0)
            {
                //
                //	Check the transaction status filter	
                //
                if(mode.Text == "ALL")
                {
                    //
                    // Show all transactions
                    //
                    for(int i = 1; i <= numTrans; i++)
                    {
                        //
                        //	Create a new transaction entry
                        //
                        TableRow row = new TableRow();
                        TableCell cell = new TableCell();

                        //
                        //	Display a link to the transaction data
                        //
                        cell.Text=string.Concat("<a href='display.aspx?transactionId=",
                            i.ToString(), "'>Expense ", i.ToString());
                        row.Cells.Add(cell);
                        cell = new TableCell();

                        //
                        //	Display the transaction status
                        //
                        cell.Text=string.Concat(ExpenseCommon.GetTransactionStatus( i ), " ", ExpenseCommon.GetTransactionDecisionTime( i ));
                        row.Cells.Add(cell);
                        TransList.Rows.Add(row);
                    }
                }
                else 
                {
                    //
                    //	Only show transactions that match the status filter
                    //
                    for(int i = 1; i <= numTrans; i++)
                    {
                        //
                        //	only show transactions of the specified type 
                        //	(ie approved, denied, pending)
                        if(string.Concat(ExpenseCommon.GetTransactionStatus(i),"") == mode.Text)
                        {
                            //
                            //	Create a new transaction entry
                            //
                            TableRow row = new TableRow();
                            TableCell cell = new TableCell();
                            
                            //
                            //	Display a link to the transaction data
                            //
                            cell.Text=string.Concat("<a href='display.aspx?transactionId=",
                                i.ToString(), "'>Expense ", i.ToString());
                            row.Cells.Add(cell);
                            cell = new TableCell();

                            //
                            //	Display the transaction status
                            //
                            cell.Text=string.Concat(ExpenseCommon.GetTransactionStatus( i ), " ", ExpenseCommon.GetTransactionDecisionTime( i ));
                            row.Cells.Add(cell);
                            TransList.Rows.Add(row);
                        }
                    }
                }
            }
        }

        /// <summary>
        /// Mode Select Selected Index Changed
        /// When the user selects a new transaction status filter
        /// refresh the list of transactions according to the new filter
        /// </summary>
        protected void ModeSelect_SelectedIndexChanged(object sender, System.EventArgs e)
        {
            //
            //	Change the filter label to the new filter (ie approved, denied, pending, all)
            //
            mode.Text = ModeSelect.SelectedValue;
            
            //
            //	Relist the transactions according to the 
            //	new filter
            //
            ListTransactions();
        }

        #region Web Form Designer generated code
        override protected void OnInit(EventArgs e)
        {
            //
            // CODEGEN: This call is required by the ASP.NET Web Form Designer.
            //
            InitializeComponent();
            base.OnInit(e);
        }
        
        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {    

        }
        #endregion
    }
}

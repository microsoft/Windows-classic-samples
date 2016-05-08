// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
//  Copyright (c) Microsoft Corporation. All rights reserved.
//
//
//	This is the main portal for the WebExpense application.
//	The user is presented with a custom UI based upon the user's
//	role memberships.
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
	/// Index Page Class
	///	Contains all methods, events and properties of the main / portal page
	/// </summary>
	public partial class MainPage : System.Web.UI.Page
	{
		#region Page Variables
		/// <summary>
		/// Logo Image - Displays the logo of the company
		/// </summary>


		/// <summary>
		/// Message Label - Displays messages to the user
		/// </summary>


		/// <summary>
		/// Approve Link - Link to a page which lists all expense
		/// reports which need to be approved
		/// </summary>


		/// <summary>
		/// Submit Link - Link to a page which allows a user
		/// to create a new expense report to submit
		/// </summary>


		/// <summary>
		/// Administration Link - Link to a page which allows an
		/// administrator to change application settings
		/// </summary>


		/// <summary>
		/// Approve Image - Display an icon that links to a page 
		/// which lists all expense reports which need to be approved
		/// </summary>


		/// <summary>
		/// Administration Image - Displays an icon that links 
		/// to a page which allows an administrator to change 
		/// application settings
		/// </summary>


		/// <summary>
		/// Submit Image - Displays an icon that links
		/// to a page which allows a user to create a new expense 
		/// report to submit
		/// </summary>


		/// <summary>
		/// Title Label - Displays the application title
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
            //	Check for this is the first time the page is being loaded
            //	only fill in the form if this is the first time otherwise
            //	any user changes will be lost
            //
            if (!Page.IsPostBack)
            {
                ExpenseCommon.Initialize();

                //
                //	Get the client context
                //                
                IAzClientContext3 AzClient = ExpenseCommon.GetAzClientContext();

                IAzBizRuleParameters BizRuleParams = AzClient.BizRuleParameters;
                BizRuleParams.AddParameter("Amount", 0);
                BizRuleParams.AddParameter("Date", "NA");
                BizRuleParams.AddParameter("SubmitterName", "NA");
                BizRuleParams.AddParameter("UserName", ExpenseCommon.GetClientSamName());

                //
                //	Use the client SAM name (\\domain\username)
                //	to display the username
                //
                string AccountName = ExpenseCommon.GetClientSamName();
                AccountName = AccountName.Substring((AccountName.IndexOf(@"\") + 1));
                MSG.Text = string.Concat("Welcome ", AccountName, ":");


                //
                //	Get the user's task assigments from the 
                //	client context
                //

                IAzTasks Tasks = AzClient.GetTasks(null);

                //
                //	Check for the user has no roles
                //
                if (Tasks.Count == 0)
                {
                    MSG.Text = string.Concat(MSG.Text, "<P>Sorry ", AccountName, " you do not have permission to use this application. <Br> Please contact your manager <Br></P>");
                }
                else
                {
                    //
                    //	Display links to the various actions the user
                    //	can perform depending on the user's role memberships and the tasks
                    //  assigned to those roles.
                    //
                    string Task;
                    foreach (IAzTask AzTask in Tasks)
                    {
                        Task = (string)AzTask.Name;

                        switch (Task)
                        {
                            case "View Pending Expenses":
                                //
                                //	User is an approver
                                //	Show link to the approval page
                                //
                                ApproveLink.Visible = true;
                                break;

                            case "Administer Settings":
                                //
                                //	User is an administrator
                                //	Show link to the administration page
                                //
                                AdminLink.Visible = true;
                                break;

                            case "Submit Expense":
                                //
                                //	User is an submitter
                                //	Show link to the expense submission page
                                //
                                SubmitLink.Visible = true;
                                break;
                        }
                    }
                }
            }
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
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
//  Copyright (c) Microsoft Corporation. All rights reserved.
//
//
//	This page allows a user to submit an expense report.
//
//
//	Carolyn Van Slyck 06/2003 - Created
//  DaveMM - Updates 06/2005 - Tweaks, Updates for SDK



using System;
using System.Collections;
using System.Collections.Specialized;
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
	/// Submit Page Class
	///	Contains all methods, events and properties of the Submit page
	/// </summary>
	public partial class Submit : System.Web.UI.Page
	{
		#region Page Variables
		/// <summary>
		/// Comment TextBox - Allows a user to attach a comment
		/// to the expense report
		/// </summary>

		/// <summary>
		/// Date TextBox - Date the expense was incurred
		/// </summary>

		/// <summary>
		/// Amount TextBox - Amount of the expense
		/// </summary>

		/// <summary>
		/// Description TextBox - Descriptiong of the expense
		/// </summary>

		/// <summary>
		/// Message Label - Displays any messages to the user
		/// </summary>

		/// <summary>
		/// Submit Group Panel - Contains the UI controls to submit
		/// an expense
		/// </summary>

		/// <summary>
		/// Logo Link - Displays the company logo and links back to
		/// the main page of the application
		/// </summary>

		/// <summary>
		/// Title Label - Displays the title of the page
		/// </summary>

		/// <summary>
		/// Description Label - Identifies the description textbox
		/// </summary>

		/// <summary>
		/// Amount Label - Identifies the amount textbox
		/// </summary>

		/// <summary>
		/// Date Label - Identifies the date textbox
		/// </summary>

		/// <summary>
		/// Comment Label - Identifies the comment textbox
		/// </summary>

		/// <summary>
		/// Submit Button - The user clicks this button to submit the expense
		/// </summary>

		/// <summary>
		/// Return Link - Link back to the main page of the application
		/// </summary>

		/// <summary>
		/// Description Validator - Requires that a description is entered
		/// A description cannot be longer than 50 characters
		/// </summary>

		/// <summary>
		/// Date Validator - Checks that the user entered a valid date between
		/// the range of 01/01/1900 and 12/31/2999
		/// </summary>

		/// <summary>
		/// Amount Validator - Checks that the user entered an amount
		/// </summary>

		/// <summary>
		/// Date Validator - Checks that the user entered a date
		/// </summary>

		/// <summary>
		/// Amount Validator - Checks that the user entered a valid amount
		/// The amount must be between 1 and 99999999999
		/// </summary>

		#endregion

		/// <summary>
		/// Submit Button Click - When the user clicks the submit button
		/// this saves the expense report in the application data store
		/// </summary>
		protected void SubmitBtn_Click(object sender, System.EventArgs e)
		{
			//
			//	Check if the user has access to the submit
			//	operation and then save the expense report
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
            BizRuleParams.AddParameter("Amount", (object)Amount.Text);
            BizRuleParams.AddParameter("Date", (object)Date.Text);
            BizRuleParams.AddParameter("SubmitterName", ExpenseCommon.GetClientSamName());
            BizRuleParams.AddParameter("UserName", ExpenseCommon.GetClientSamName());

			//
			//	Run the access check on the submit operation
			//	Passing the audit text, scope, operations and business rule parameters
			//
            uint result = AzClient.AccessCheck2("Submit Expense Report", "", ExpenseCommon.AzopSubmit);

			//
			//	Check for success of the access check
			//
            bool bAuthorized = false;

            if (result == ExpenseCommon.NoError)
            {
                bAuthorized = true;
            }

			else if(result == ExpenseCommon.AccessDenied)
			{
				string errorMessage = AzClient.GetBusinessRuleString();
				if(errorMessage != "")
				{
                    MSG.Text = "<font color=\"FF0000\">Submission Denied." + errorMessage + "</font>";
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
				 if(result != ExpenseCommon.NoError)
				{
					Win32Exception ex = new Win32Exception();
                    MSG.Text = "<font color=\"FF0000\">There was an error performing the AccessCheck: " + ex.Message + "</font>";
				}
            }

		
			if(bAuthorized)
			{
				//
				//	AccessCheck passed so submit the expense report
				//

				//
				//	Store the expense report in a name-value collection
				//
				StringDictionary ExpenseData = new StringDictionary();
				
				//
				//	Save the user SAM name (\\domain\username)
				//
				string name = ExpenseCommon.GetClientSamName();
				ExpenseData.Add("SamName", name);
				
				//
				//	Save the user Friendly Name
				//
				name = name.Substring((name.IndexOf(@"\")+1));
				ExpenseData.Add("User",name);

				//
				//	Save the transaction date
				//
				ExpenseData.Add("Date",Date.Text);

				//
				//	Save the expense description
				//
				ExpenseData.Add("Description",Description.Text);

				//
				//	Save the expense amount
				//
				ExpenseData.Add("Amount",Amount.Text);

				//
				// Attach any comments to the expense report
				//
				ExpenseData.Add("Comment",Comment.Text);

				//
				//	Save the transaction
				//
				ExpenseCommon.SaveTransaction(ExpenseCommon.AssignNextTransaction(), ExpenseData);
				
				//
				//	Show link to submit a new expense or
				//	to return to the main page
				//
				MSG.Text="Submission Sucessful.<p><a href='Submit.aspx'>Submit new expense</a> | <a href='../index.aspx'>Return to Main Menu</a></p>";

				//
				//	Clear form for new entry
				//
				Description.Text="";
				Amount.Text="";
				Date.Text="";
				Comment.Text="";

				SubmitGroup.Visible=false;
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
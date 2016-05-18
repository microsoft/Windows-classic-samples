//=======================================================================================
//
//  This source code is only intended as a supplement to existing Microsoft documentation. 
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
//  KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
//  IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
//  PURPOSE.
//
//  Copyright (C) Microsoft Corporation.  All Rights Reserved.
//
//=======================================================================================
using System;
using Microsoft.ManagementConsole;

namespace Microsoft.ManagementConsole.Samples
{
	/// <summary>
	/// User property page.
	/// </summary>
	public class UserPropertyPage : PropertyPage
	{
		private UserPropertiesControl userPropertiesControl = null;

		/// <summary>
		/// Constructor for the page.
		/// </summary>
		public UserPropertyPage()
		{
            // setup property page container stuff
            this.Title = "User Property Page";

            // setup contained control and hand it a reference to its parent (This propertypage)
            userPropertiesControl = new UserPropertiesControl(this);
            this.Control = userPropertiesControl;
        }

		/// <summary>
		/// Initialize notification for the page. Default implementation is empty.
		/// </summary>
		protected override void OnInitialize()
		{
            base.OnInitialize();

			// populate the contained control 
            userPropertiesControl.RefreshData((ResultNode)this.ParentSheet.SelectionObject);
		}

		/// <summary>
		/// Sent to every page in the property sheet to indicate that the user has clicked 
		/// the Apply button and wants all changes to take effect.
		/// </summary>
		protected override bool OnApply()
		{
			// does the control say the values are valid?
			if (this.Dirty)
			{
				if (userPropertiesControl.CanApplyChanges())
				{
					// save changes
                    userPropertiesControl.UpdateData((ResultNode)this.ParentSheet.SelectionObject);
				}
				else
				{
					// something invalid was entered 
					return false;
				}
			}
			return true;
		}

		/// <summary>
		/// Sent to every page in the property sheet to indicate that the user has clicked the OK 
		/// or Close button and wants all changes to take effect.
		/// </summary>
		protected override bool OnOK()
		{
			return this.OnApply();
		}

		/// <summary>
		/// Indicates that the user wants to cancel the property sheet.
		/// Default implementation allows cancel operation.
		/// </summary>
		protected override bool QueryCancel()
		{
			return true;
		}

		/// <summary>
		/// Indicates that the user has canceled and the property sheet is about to be destroyed.
		/// All changes made since the last PSN_APPLY notification are canceled
		/// </summary>
		protected override void OnCancel()
		{
            userPropertiesControl.RefreshData((ResultNode)this.ParentSheet.SelectionObject);
		}
		
		/// <summary>
		/// Notifies a page that the property sheet is getting destoyed. 
		/// Use this notification message as an opportunity to perform cleanup operations.
		/// </summary>
		protected override void OnDestroy()
		{
		}

	} // class
} // namespace
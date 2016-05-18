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
using System.Collections.Generic;
using System.Text;

namespace Microsoft.ManagementConsole.Samples
{
    /// <summary>
    /// FormView to display Winforms controls
    /// </summary>
    public class SelectionFormView : FormView
    {
        private SelectionControl selectionControl = null;

        /// <summary>
        /// Constructor
        /// </summary>
        public SelectionFormView()
        {
        }

        /// <summary>
        /// Handle any setup necessary
        /// </summary>
        /// <param name="status">asynchronous status for updating the console</param>
        protected override void OnInitialize(AsyncStatus status)
        {
            // handle any basic stuff
            base.OnInitialize(status);

            // get typed reference to the hosted control 
            // setup by the FormViewDescription
            selectionControl = (SelectionControl)this.Control;

            //// load data in 
            Refresh();
        }

        /// <summary>
        /// Loads in fictional data by handing it to the control to process
        /// </summary>
        protected void Refresh()
        {
            // Get some fictitious data to populate the lists with
            string[][] users = { new string[] {"Karen", "February 14th"},
                                        new string[] {"Sue", "May 5th"},
                                        new string[] {"Tina", "April 15th"},
                                        new string[] {"Lisa", "March 27th"},
                                        new string[] {"Tom", "December 25th"},
                                        new string[] {"John", "January 1st"},
                                        new string[] {"Harry", "October 31st"},
                                        new string[] {"Bob", "July 4th"}
                                    };

            selectionControl.RefreshData(users);            
        }

        /// <summary>
        /// Handle triggered action
        /// </summary>
        /// <param name="action">triggered action</param>
        /// <param name="status">asynchronous status to update console</param>
        protected override void OnSelectionAction(Action action, AsyncStatus status)
        {
            switch ((string)action.Tag)
            {
                case "ShowSelection":
                    {
                        selectionControl.ShowSelection();
                        break;
                    }
            }
        }

    } // class
} // namespace

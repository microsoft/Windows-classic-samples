using System;
using System.Collections.Generic;
using System.Text;

namespace Microsoft.ManagementConsole.Samples
{
    /// <summary>
    /// Service 'startup' properties page.
    /// </summary>
    public class StartupPropertyPage : PropertyPage
    {
        #region Overridden Methods

        /// <summary>
        /// Initialize notification for the page. Default implementation is empty.
        /// </summary>
        protected override void OnInitialize()
        {
            startupPropertiesControl = (StartupPropertiesControl)Control;
            startupPropertiesControl.InitializePageControl();
        }

        /// <summary>
        /// Sent to every page in the property sheet to indicate that the user has clicked 
        /// the Apply button and wants all changes to take effect.
        /// </summary>
        protected override bool OnApply()
        {
            return startupPropertiesControl.CanApplyChanges();
        }

        /// <summary>
        /// Sent to every page in the property sheet to indicate that the user has clicked the OK 
        /// or Close button and wants all changes to take effect.
        /// </summary>
        protected override bool OnOK()
        {
            return startupPropertiesControl.CanOKChanges();
        }

        /// <summary>
        /// Notifies a page that it is about to lose activation either because another page is 
        /// being activated or the user has clicked the OK button.
        /// Default implementation allows page to be de-activated.
        /// </summary>
        protected override bool OnKillActive()
        {
            return startupPropertiesControl.CanKillActive();
        }


        /// <summary>
        /// Indicates that the wants to cancel the property sheet.
        /// Default implementation allows cancel operation.
        /// </summary>
        protected override bool QueryCancel()
        {
            return startupPropertiesControl.CanCancelChanges();
        }

        #endregion Overridden Methods

        #region Fields

        private StartupPropertiesControl startupPropertiesControl = null;

        #endregion Fields
    } // class
} // namespace

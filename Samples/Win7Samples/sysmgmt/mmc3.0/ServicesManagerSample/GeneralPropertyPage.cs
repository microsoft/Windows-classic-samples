using System;

namespace Microsoft.ManagementConsole.Samples
{
    /// <summary>
    /// Service property page.
    /// </summary>
    public class GeneralPropertyPage : PropertyPage
    {
        #region Overridden Methods

        /// <summary>
        /// Initialize notification for the page. Default implementation is empty.
        /// </summary>
        protected override void OnInitialize()
        {
            generalPropertiesControl = (GeneralPropertiesControl)Control;
            generalPropertiesControl.InitializePageControl();
        }

        /// <summary>
        /// Sent to every page in the property sheet to indicate that the user has clicked 
        /// the Apply button and wants all changes to take effect.
        /// </summary>
        protected override bool OnApply()
        {
            return generalPropertiesControl.CanApplyChanges();
        }

        /// <summary>
        /// Sent to every page in the property sheet to indicate that the user has clicked the OK 
        /// or Close button and wants all changes to take effect.
        /// </summary>
        protected override bool OnOK()
        {
            return generalPropertiesControl.CanOKChanges();
        }

        /// <summary>
        /// Notifies a page that it is about to lose activation either because another page is 
        /// being activated or the user has clicked the OK button.
        /// Default implementation allows page to be de-activated.
        /// </summary>
        protected override bool OnKillActive()
        {
            return generalPropertiesControl.CanKillActive();
        }

        /// <summary>
        /// Indicates that the wants to cancel the property sheet.
        /// Default implementation allows cancel operation.
        /// </summary>
        protected override bool QueryCancel()
        {
            return generalPropertiesControl.CanCancelChanges();
        }

        #endregion Overridden Methods

        #region Fields

        private GeneralPropertiesControl generalPropertiesControl = null;

        #endregion Fields
    } // class
} // namespace

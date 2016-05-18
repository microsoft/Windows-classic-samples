using System;
using System.Collections.Generic;
using System.Text;

namespace Microsoft.ManagementConsole.Samples
{
    /// <summary>
    /// MessageView - Shows a message in the View Pane 
    /// NormalMessageView - Can switch to showing a different view (a sample error) 
    /// in the view pane.
    /// </summary>
    public class NormalMessageView : MessageView
    {
        /// <summary>
        /// Constructor
        /// </summary>
        public NormalMessageView()
        {
            // set message
            this.Title = "Normal Message View";
            this.BodyText = "This is a MessageView. You can attach it to any scope node. Click the 'Show Error' action to show a different view.";
            this.IconId = MessageViewIcon.Information;

            // add an action to manual trigger an 'error' handling scenario
            this.ActionsPaneItems.Add(new Action("Show Error", "Switches the view to an error handling view", -1, "ShowError"));
        }

        /// <summary>
        /// Handle when the action is clicked in context menu or action pane
        /// </summary>
        /// <param name="action"></param>
        /// <param name="status"></param>
        protected override void OnAction(Action action, AsyncStatus status)
        {
            if ((string)action.Tag == "ShowError")
            {
                // trigger MMC to show the new view
                ((ViewSwitchingScopeNode)this.ScopeNode).SwapView(this, "Simulated Error Message", typeof(ErrorMessageView));
            }
        }
    }

    /// <summary>
    /// MessageView - Shows a message in the View Pane 
    /// ErrorMessageView - Can switch back to showing a normal view 
    /// in the view pane.
    /// </summary>
    public class ErrorMessageView : MessageView
    {
        /// <summary>
        /// Constructor
        /// </summary>
        public ErrorMessageView()
        {
            // set message
            this.Title = "Simulated *Error* Message View";
            this.BodyText = "You can show a MessageView when you detect an error. Click the 'Back to Normal' action to see it change back.";
            this.IconId = MessageViewIcon.Information;

            // add an action to manual trigger that the error was handled scenario
            this.ActionsPaneItems.Add(new Action("Show Normal", "Switches the view to the normal view", -1, "ShowNormal"));
        }

        /// <summary>
        /// Handle when the action is clicked in context menu or action pane
        /// </summary>
        /// <param name="action"></param>
        /// <param name="status"></param>
        protected override void OnAction(Action action, AsyncStatus status)
        {
            if ((string)action.Tag == "ShowNormal")
            {
                // trigger MMC to show the new view
                ((ViewSwitchingScopeNode)this.ScopeNode).SwapView(this, "Simulated Error Message", typeof(NormalMessageView));
            }
        }
    }

} // namespace

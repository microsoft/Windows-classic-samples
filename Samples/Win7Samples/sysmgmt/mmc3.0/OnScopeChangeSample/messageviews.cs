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
            this.BodyText = "This is a MessageView. You can attach it to any scope node. \nSelect 'Simulate an Error' to show a different view.";
            this.IconId = MessageViewIcon.Information;
        }

        /// <summary>
        /// Do any initialization. In this case Start listening for scope node events
        /// </summary>
        /// <param name="status"></param>
        protected override void OnInitialize(AsyncStatus status)
        {
            ((NotifyingScopeNode)this.ScopeNode).Changed += new NotifyingScopeNode.ChangedDelegate(OnScopeNodeChange);
        }

        /// <summary>
        /// Do any cleanup. In this case Stop listening for scope node events
        /// </summary>
        /// <param name="status"></param>
        protected override void OnShutdown(SyncStatus status)
        {
            ((NotifyingScopeNode)this.ScopeNode).Changed -= new NotifyingScopeNode.ChangedDelegate(OnScopeNodeChange);
        }

        /// <summary>
        /// Handle any change to the scope node. In this case jsut refresh views
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        protected void OnScopeNodeChange(object sender, NotifyingScopeNode.ChangedEventArgs e) 
        {
            if (e.Status != "ShowNormal")
            {
                // make MMC refresh the view status
                this.SelectScopeNode(this.ScopeNode);
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
            this.BodyText = "You can show a MessageView when you detect an error. \nSelect the 'Show Normal' action to see it change back.";
            this.IconId = MessageViewIcon.Information;
        }

        /// <summary>
        /// Do any initialization. In this case Start listening for scope node events
        /// </summary>
        /// <param name="status"></param>
        protected override void OnInitialize(AsyncStatus status)
        {
            ((NotifyingScopeNode)this.ScopeNode).Changed += new NotifyingScopeNode.ChangedDelegate(OnScopeNodeChange);
        }

        /// <summary>
        /// Do any cleanup. In this case Stop listening for scope node events
        /// </summary>
        /// <param name="status"></param>
        protected override void OnShutdown(SyncStatus status)
        {
            ((NotifyingScopeNode)this.ScopeNode).Changed -= new NotifyingScopeNode.ChangedDelegate(OnScopeNodeChange);
        }

        /// <summary>
        /// Handle any change to the scope node. In this case jsut refresh views
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        protected void OnScopeNodeChange(object sender, NotifyingScopeNode.ChangedEventArgs e)
        {
            if (e.Status != "ShowError")
            {
                // make MMC refresh the view status
                this.SelectScopeNode(this.ScopeNode);
            }
        }
    }

} // namespace

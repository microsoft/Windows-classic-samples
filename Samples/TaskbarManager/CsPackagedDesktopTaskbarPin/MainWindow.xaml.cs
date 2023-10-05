// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

using System;
using Microsoft.UI.Xaml;
using Windows.ApplicationModel;
using Windows.UI.Shell;
using WinRT;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace CsPackagedDesktopTaskbarPin
{
    /// <summary>
    /// An empty window that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed partial class MainWindow : Window
    {
        public MainWindow()
        {
            InitializeComponent();

            // Usage of this sample requires that you have an assigned LAF token for this functionality and have filled it in in LafData.cs.
            if (LafData.Token.Length > 0)
            {
                samplePanel.Visibility = Visibility.Visible;
                lafErrorText.Visibility = Visibility.Collapsed;
            }
        }

        // Tries to unlock the necessary taskbar pinning LAF and returns true if successful.
        private bool TryUnlockPinningLaf()
        {
            // Ensure the appropriate LAF is unlocked.
            // Appropriate LAF values must be specified in "LafData.h" based on your LAF token.
            var unlockStatus = LimitedAccessFeatures.TryUnlockFeature(
                LafData.Feature, LafData.Token, LafData.Attestation).Status;
            return ((unlockStatus == LimitedAccessFeatureStatus.Available) || (unlockStatus == LimitedAccessFeatureStatus.AvailableWithoutToken));
        }

        // Returns true if the TaskbarManager supports desktop clients.
        private static bool IsDesktopAppPinningSupported()
        {
            // Check whether the TaskbarManager factory supports the ITaskbarManagerDesktopAppSupportStatics marker interface.
            return TaskbarManager.As<IWinRTObject>().NativeObject.TryAs<IInspectable.Vftbl>(
                new Guid("cdfefd63-e879-4134-b9a7-8283f05f9480"), out _) >= 0;
        }

        // Checks whether pinning the current app is possible and updates the pin button and status text accordingly.
        // NOTE: Your app should not give the user an option to pin if requesting the pin won't do anything, so the ability to pin should be checked first.
        //       There is no guarantee that an app will be allowed to pin at any given time.
        async void PinCurrentScenarioButton_Click(object sender, RoutedEventArgs args)
        {
            // First, check whether pinning from desktop apps is supported at all.
            if (TryUnlockPinningLaf() && IsDesktopAppPinningSupported())
            {
                // Then, check whether the current app is already pinned and, if not, whether pinning is currently allowed from this app.
                var taskbarManager = TaskbarManager.GetDefault();
                if (!await taskbarManager.IsCurrentAppPinnedAsync())
                {
                    if (taskbarManager.IsPinningAllowed)
                    {
                        // Pinning is supported and allowed, and the app is not already pinned. The app can offer the user the option to pin it.
                        pinCurrentScenarioPinButton.IsEnabled = true;
                        pinCurrentScenarioStatusText.Text = "";
                    }
                    else
                    {
                        // Pinning is supported, but pinning is not currently allowed (e.g., the system may not be able to show the prompt
                        // at this time). The app should not offer the usre an option to pin now, but it may check again at a later time
                        // (such as when showing the UI with the pin option on it again).
                        pinCurrentScenarioPinButton.IsEnabled = false;
                        pinCurrentScenarioStatusText.Text = "Pinning is not allowed";
                    }
                }
                else
                {
                    // Pinning is supported, but the app is already pinned. The app should not offer ther user an option to pin again.
                    pinCurrentScenarioPinButton.IsEnabled = false;
                    pinCurrentScenarioStatusText.Text = "App is already pinned";
                }

                pinCurrentScenarioPinButton.Visibility = Visibility.Visible;
            }
            else
            {
                // Pinning is not supported on this system, either because the limited access feature couldn't be unlocked
                // or the API indicated desktop callers aren't supported. The app should not offer the user an option to pin.
                pinCurrentScenarioStatusText.Text = "Pinning is not supported";
                pinCurrentScenarioPinButton.Visibility = Visibility.Collapsed;
            }
        }

        // Attempts to pin the current app and updates the status text with the result.
        async void PinCurrentScenarioPinButton_Click(object sender, RoutedEventArgs args)
        {
            // Make the pin request. Additional pin requests for the current app may not be made
            // while one is still outstanding, so disable the pin request UI until the operation is complete.
            pinCurrentScenarioButton.IsEnabled = false;
            pinCurrentScenarioPinButton.IsEnabled = false;
            var isPinned = await TaskbarManager.GetDefault().RequestPinCurrentAppAsync();

            // Update the UI to the appropriate state based on the results of the pin request.
            pinCurrentScenarioPinButton.IsEnabled = !isPinned;
            pinCurrentScenarioStatusText.Text = isPinned ? "Pinned" : "Not pinned";
            pinCurrentScenarioButton.IsEnabled = true;
        }
    }
}

// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright © Microsoft Corporation. All rights reserved

using System;
using System.ComponentModel;
using System.Windows;
using System.Windows.Controls;

namespace Microsoft.Samples.Speech.VoiceAccess
{
    /// <summary>
    /// Interaction logic for VoiceAccessUserControl.xaml
    /// Voice Access control that displays current mode (Translator/Recognizer)
    /// based on the m_useTranslatorConfig flag
    /// </summary>
    public partial class VoiceAccessUserControl : UserControl, INotifyPropertyChanged
    {
        #region Private Fields
        
        /// <summary>
        /// Flag to determine if translator mode is active
        /// true = Translator Mode, false = Recognizer Mode
        /// </summary>
        private bool m_useTranslatorConfig = false;
        
        /// <summary>
        /// Property for the mode text displayed in the UI
        /// </summary>
        private string m_modeText = "";
        
        #endregion

        #region Public Properties
        
        /// <summary>
        /// Gets or sets the current mode text
        /// </summary>
        public string ModeText
        {
            get { return m_modeText; }
            private set
            {
                if (m_modeText != value)
                {
                    m_modeText = value;
                    OnPropertyChanged(nameof(ModeText));
                }
            }
        }
        
        /// <summary>
        /// Gets or sets the translator configuration flag
        /// </summary>
        public bool UseTranslatorConfig
        {
            get { return m_useTranslatorConfig; }
            set
            {
                if (m_useTranslatorConfig != value)
                {
                    m_useTranslatorConfig = value;
                    UpdateModeText();
                    OnPropertyChanged(nameof(UseTranslatorConfig));
                }
            }
        }
        
        #endregion

        #region Constructor
        
        /// <summary>
        /// Initializes a new instance of the VoiceAccessUserControl
        /// </summary>
        public VoiceAccessUserControl()
        {
            InitializeComponent();
            DataContext = this;
            
            // Initialize with default mode
            UpdateModeText();
            UpdateStatus("Voice Access Control initialized.");
        }
        
        #endregion

        #region Private Methods
        
        /// <summary>
        /// Updates the mode text based on the current configuration
        /// </summary>
        private void UpdateModeText()
        {
            ModeText = m_useTranslatorConfig ? "Translator Mode" : "Recognizer Mode";
        }
        
        /// <summary>
        /// Updates the status text display
        /// </summary>
        /// <param name="message">Status message to display</param>
        private void UpdateStatus(string message)
        {
            if (StatusTextBlock != null)
            {
                StatusTextBlock.Text = $"[{DateTime.Now:HH:mm:ss}] {message}";
            }
        }
        
        #endregion

        #region Event Handlers
        
        /// <summary>
        /// Handles the toggle mode button click
        /// </summary>
        private void ToggleModeButton_Click(object sender, RoutedEventArgs e)
        {
            UseTranslatorConfig = !UseTranslatorConfig;
            UpdateStatus($"Mode changed to: {ModeText}");
        }
        
        /// <summary>
        /// Handles the start button click
        /// </summary>
        private void StartButton_Click(object sender, RoutedEventArgs e)
        {
            UpdateStatus($"Starting {ModeText}...");
            
            // Simulate starting the appropriate service
            if (m_useTranslatorConfig)
            {
                UpdateStatus("Translator service started. Ready to translate speech.");
            }
            else
            {
                UpdateStatus("Speech recognizer started. Ready to recognize speech.");
            }
        }
        
        /// <summary>
        /// Handles the stop button click
        /// </summary>
        private void StopButton_Click(object sender, RoutedEventArgs e)
        {
            UpdateStatus($"Stopping {ModeText}...");
            
            // Simulate stopping the service
            if (m_useTranslatorConfig)
            {
                UpdateStatus("Translator service stopped.");
            }
            else
            {
                UpdateStatus("Speech recognizer stopped.");
            }
        }
        
        #endregion

        #region INotifyPropertyChanged Implementation
        
        /// <summary>
        /// Event raised when a property value changes
        /// </summary>
        public event PropertyChangedEventHandler PropertyChanged;
        
        /// <summary>
        /// Raises the PropertyChanged event
        /// </summary>
        /// <param name="propertyName">Name of the property that changed</param>
        protected virtual void OnPropertyChanged(string propertyName)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }
        
        #endregion
    }
}
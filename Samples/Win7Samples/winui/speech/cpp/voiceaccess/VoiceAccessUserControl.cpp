// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright © Microsoft Corporation. All rights reserved

#pragma once

#using <mscorlib.dll>
#using <System.dll>
#using <PresentationCore.dll>
#using <PresentationFramework.dll>
#using <WindowsBase.dll>

using namespace System;
using namespace System::ComponentModel;
using namespace System::Windows;
using namespace System::Windows::Controls;

namespace Microsoft { namespace Samples { namespace Speech { namespace VoiceAccess {

    /// <summary>
    /// Voice Access User Control implementation in C++/CLI
    /// Displays current mode (Translator/Recognizer) based on m_useTranslatorConfig flag
    /// </summary>
    public ref class VoiceAccessUserControl : UserControl, INotifyPropertyChanged
    {
    private:
        /// <summary>
        /// Flag to determine if translator mode is active
        /// true = Translator Mode, false = Recognizer Mode
        /// </summary>
        bool m_useTranslatorConfig;
        
        /// <summary>
        /// Property for the mode text displayed in the UI
        /// </summary>
        String^ m_modeText;

        /// <summary>
        /// UI Elements
        /// </summary>
        TextBlock^ ModeTextBlock;
        TextBlock^ StatusTextBlock;
        Button^ ToggleModeButton;
        Button^ StartButton;
        Button^ StopButton;

    public:
        /// <summary>
        /// Default constructor
        /// </summary>
        VoiceAccessUserControl()
        {
            m_useTranslatorConfig = false;
            m_modeText = "";
            
            InitializeComponent();
            DataContext = this;
            
            // Initialize with default mode
            UpdateModeText();
            UpdateStatus("Voice Access Control initialized.");
        }

        /// <summary>
        /// Gets the current mode text
        /// </summary>
        property String^ ModeText
        {
            String^ get() { return m_modeText; }
            private: void set(String^ value)
            {
                if (m_modeText != value)
                {
                    m_modeText = value;
                    OnPropertyChanged("ModeText");
                }
            }
        }

        /// <summary>
        /// Gets or sets the translator configuration flag
        /// </summary>
        property bool UseTranslatorConfig
        {
            bool get() { return m_useTranslatorConfig; }
            void set(bool value)
            {
                if (m_useTranslatorConfig != value)
                {
                    m_useTranslatorConfig = value;
                    UpdateModeText();
                    OnPropertyChanged("UseTranslatorConfig");
                }
            }
        }

        /// <summary>
        /// Property changed event for data binding
        /// </summary>
        virtual event PropertyChangedEventHandler^ PropertyChanged;

    private:
        /// <summary>
        /// Initialize the user control components
        /// </summary>
        void InitializeComponent()
        {
            // Load XAML content
            String^ xamlUri = "pack://application:,,,/VoiceAccessUserControl.xaml";
            System::Uri^ uri = gcnew System::Uri(xamlUri, System::UriKind::Absolute);
            System::Windows::Application::LoadComponent(this, uri);
            
            // Get references to named elements
            ModeTextBlock = safe_cast<TextBlock^>(FindName("ModeTextBlock"));
            StatusTextBlock = safe_cast<TextBlock^>(FindName("StatusTextBlock"));
            ToggleModeButton = safe_cast<Button^>(FindName("ToggleModeButton"));
            StartButton = safe_cast<Button^>(FindName("StartButton"));
            StopButton = safe_cast<Button^>(FindName("StopButton"));
            
            // Wire up event handlers
            if (ToggleModeButton != nullptr)
                ToggleModeButton->Click += gcnew RoutedEventHandler(this, &VoiceAccessUserControl::ToggleModeButton_Click);
            if (StartButton != nullptr)
                StartButton->Click += gcnew RoutedEventHandler(this, &VoiceAccessUserControl::StartButton_Click);
            if (StopButton != nullptr)
                StopButton->Click += gcnew RoutedEventHandler(this, &VoiceAccessUserControl::StopButton_Click);
        }

        /// <summary>
        /// Updates the mode text based on the current configuration
        /// </summary>
        void UpdateModeText()
        {
            ModeText = m_useTranslatorConfig ? "Translator Mode" : "Recognizer Mode";
        }

        /// <summary>
        /// Updates the status text display
        /// </summary>
        /// <param name="message">Status message to display</param>
        void UpdateStatus(String^ message)
        {
            if (StatusTextBlock != nullptr)
            {
                DateTime now = DateTime::Now;
                StatusTextBlock->Text = String::Format("[{0:HH:mm:ss}] {1}", now, message);
            }
        }

        /// <summary>
        /// Raises the PropertyChanged event
        /// </summary>
        /// <param name="propertyName">Name of the property that changed</param>
        void OnPropertyChanged(String^ propertyName)
        {
            PropertyChangedEventArgs^ args = gcnew PropertyChangedEventArgs(propertyName);
            PropertyChanged(this, args);
        }

        /// <summary>
        /// Toggle mode button click handler
        /// </summary>
        void ToggleModeButton_Click(Object^ sender, RoutedEventArgs^ e)
        {
            UseTranslatorConfig = !UseTranslatorConfig;
            UpdateStatus(String::Format("Mode changed to: {0}", ModeText));
        }

        /// <summary>
        /// Start button click handler
        /// </summary>
        void StartButton_Click(Object^ sender, RoutedEventArgs^ e)
        {
            UpdateStatus(String::Format("Starting {0}...", ModeText));
            
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
        /// Stop button click handler
        /// </summary>
        void StopButton_Click(Object^ sender, RoutedEventArgs^ e)
        {
            UpdateStatus(String::Format("Stopping {0}...", ModeText));
            
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
    };

}}}} // namespace Microsoft::Samples::Speech::VoiceAccess
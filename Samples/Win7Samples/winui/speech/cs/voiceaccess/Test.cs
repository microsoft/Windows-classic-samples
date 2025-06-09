// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright © Microsoft Corporation. All rights reserved

using System;
using System.ComponentModel;

namespace Microsoft.Samples.Speech.VoiceAccess.Test
{
    /// <summary>
    /// Simple test class to demonstrate the core functionality
    /// without requiring the full WPF infrastructure
    /// </summary>
    public class VoiceAccessCore : INotifyPropertyChanged
    {
        private bool m_useTranslatorConfig = false;
        private string m_modeText = "";

        public event PropertyChangedEventHandler PropertyChanged;

        /// <summary>
        /// Gets the current mode text
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

        /// <summary>
        /// Constructor
        /// </summary>
        public VoiceAccessCore()
        {
            UpdateModeText();
        }

        /// <summary>
        /// Updates the mode text based on the current configuration
        /// </summary>
        private void UpdateModeText()
        {
            ModeText = m_useTranslatorConfig ? "Translator Mode" : "Recognizer Mode";
        }

        /// <summary>
        /// Raises the PropertyChanged event
        /// </summary>
        protected virtual void OnPropertyChanged(string propertyName)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }
    }

    /// <summary>
    /// Test program to verify the core functionality
    /// </summary>
    class TestProgram
    {
        static void Main(string[] args)
        {
            Console.WriteLine("Voice Access User Control - Core Functionality Test");
            Console.WriteLine("===================================================");
            Console.WriteLine();

            var voiceAccess = new VoiceAccessCore();
            
            // Subscribe to property changes
            voiceAccess.PropertyChanged += (sender, e) =>
            {
                Console.WriteLine($"Property '{e.PropertyName}' changed");
            };

            // Test initial state
            Console.WriteLine($"Initial Mode: {voiceAccess.ModeText}");
            Console.WriteLine($"Initial UseTranslatorConfig: {voiceAccess.UseTranslatorConfig}");
            Console.WriteLine();

            // Test switching to translator mode
            Console.WriteLine("Switching to Translator Mode...");
            voiceAccess.UseTranslatorConfig = true;
            Console.WriteLine($"Current Mode: {voiceAccess.ModeText}");
            Console.WriteLine($"UseTranslatorConfig: {voiceAccess.UseTranslatorConfig}");
            Console.WriteLine();

            // Test switching to recognizer mode
            Console.WriteLine("Switching to Recognizer Mode...");
            voiceAccess.UseTranslatorConfig = false;
            Console.WriteLine($"Current Mode: {voiceAccess.ModeText}");
            Console.WriteLine($"UseTranslatorConfig: {voiceAccess.UseTranslatorConfig}");
            Console.WriteLine();

            // Test that setting the same value doesn't trigger events
            Console.WriteLine("Setting same value (should not trigger events)...");
            voiceAccess.UseTranslatorConfig = false;
            Console.WriteLine("No property change events should have been triggered.");
            Console.WriteLine();

            Console.WriteLine("Test completed successfully!");
            Console.WriteLine("Press any key to exit...");
            Console.ReadKey();
        }
    }
}
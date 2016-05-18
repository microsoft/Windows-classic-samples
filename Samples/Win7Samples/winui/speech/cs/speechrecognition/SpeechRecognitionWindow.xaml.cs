// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright © Microsoft Corporation. All rights reserved

using System;
using System.Collections.Generic;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Media.Animation;
using System.Windows.Shapes;

using System.Speech.Recognition;
using Microsoft.Win32;
using System.Diagnostics;
using System.IO;

namespace Microsoft.Samples.Speech.Recognition.SpeechRecognition
{
    public sealed partial class SpeechRecognitionWindow : System.Windows.Window , IDisposable
    {

        #region Internal Types

        internal enum EngineType
        {
            SpeechRecognizer,           //Shared recognizer
            SpeechRecognitionEngine     //In process recognizer
        }

        internal enum AudioInput
        {
            DefaultAudioDevice,
            AudioFile
        }

        internal enum RecognitionMode
        {
            Asynchronous,
            Synchronous
        }

        #endregion Internal Types


        #region Private Fields

        // Whether the speech recognition engine is created or not.
        bool _engineCreated;

        // Type of the selected engine
        EngineType _engineType;
        // Recognition Engines
        SpeechRecognizer _speechRecognizer;
        SpeechRecognitionEngine _speechRecognitionEngine;

        // Input type to the engine (SpeechRecognitionEngine only)
        AudioInput _audioInput;

        // Sync or Async recognition (SpeechRecognitionEngine only)
        RecognitionMode _recognitionMode;
        // (SpeechRecognitionEngine only)
        bool _isRecognizing;

        // Three grammar categories in the grammar display tree
        TreeViewItem _dictationGrammars;
        TreeViewItem _grammarBuilders;
        TreeViewItem _SRGSGrammars;

        // Background brush of the grammar tree view (used for animation)
        SolidColorBrush _treeViewGrammarsBackground;
        // Background brushes of each grammar (used for animation)
        Dictionary<Grammar, SolidColorBrush> _dictionaryBrushes = new Dictionary<Grammar, SolidColorBrush>();

        // Colors (used for animation)
        static Color _colorDefault = Colors.Black;
        static Color _colorDefaultBackground = Colors.Transparent;
        static Color _colorHypothesized = Colors.Blue;
        static Color _colorRecognized = Colors.Green;
        static Color _colorRejected = Colors.Red;
        // Brushes derived from the colors above (used for animation)
        static SolidColorBrush _brushDefault = new SolidColorBrush(_colorDefault);
        static SolidColorBrush _brushHypothesized = new SolidColorBrush(_colorHypothesized);
        static SolidColorBrush _brushRecognized = new SolidColorBrush(_colorRecognized);
        static SolidColorBrush _brushRejected = new SolidColorBrush(_colorRejected);

        // FlowDocument elements for displaying the status and result of the recognition
        Paragraph _paragraphStatus;
        Paragraph _paragraphResult;
        Run _hypothesis;

        // Stream and player for the last recognized audio
        MemoryStream _recognizedAudioStream;
        System.Media.SoundPlayer _recognizedAudioPlayer;

        // Track whether Dispose has been called.
        bool disposed;

        #endregion Private Fields


        #region Constructor

        public SpeechRecognitionWindow()
        {
            // Required by WPF to initialize the window
            InitializeComponent();
            
            // Initialize the Status Group Box
            InitializeStatusControls();

            // Update the status of tab items
            UpdateTabItemSpeechRecognizer();
            UpdateTabItemSpeechRecognitionEngine();

            // Initialize the Grammar Tree View and create the dictation grammars.
            // Dictation grammars will be used by all recognizers, no need to recreate them
            InitializeGroupBoxGrammars();
            CreateDictationGrammars();
        }

        #endregion Constructor


        #region Methods and Event Handlers for SpeechRecognizer Tab

        // This method enables or disables the controls in the SpeechRecognizer Tab
        // according to the current state of the engine.
        private void UpdateTabItemSpeechRecognizer()
        {
            bool isCreated = _engineCreated && _engineType == EngineType.SpeechRecognizer;

            _buttonCreateSR.IsEnabled = !isCreated;
            _buttonDisposeSR.IsEnabled = isCreated;

            _labelEngineStateLabel.IsEnabled = isCreated;
            _labelEngineState.IsEnabled = isCreated;

            // Update the recognition group box in this tab.
            UpdateGroupBoxRecognitionSR();
        }

        // This method enables or disables the controls in the Recognition Group Box
        // according to the current state of the engine.
        private void UpdateGroupBoxRecognitionSR()
        {
            // Controls are active only if the engine is created
            bool isCreated = _engineCreated && _engineType == EngineType.SpeechRecognizer;

            _groupBoxRecognitionSR.IsEnabled = isCreated;

            if (isCreated)
            {
                // Emulate button and text box are only enabled when SpeechRecognizer
                // is enabled.
                bool isEnabled = _speechRecognizer.Enabled;

                _textBoxEmulateSR.IsEnabled = isEnabled;
                _buttonEmulateSR.IsEnabled = isEnabled;

                _buttonEnable.IsEnabled = !isEnabled;
                _buttonDisable.IsEnabled = isEnabled;
            }
        }

        void _buttonCreateSR_Click(object sender, RoutedEventArgs e)
        {
            if (_engineCreated)
            {
                if (AskForNewEngine() == false)
                {
                    return;
                }
            }

            CreateRecognitionEngine(EngineType.SpeechRecognizer);

            // We create the GrammarBuilders after creating the engine becuase
            // the culture of each GrammarBuilder must match the culture of
            // the engine
            CreateGrammarBuilders();

            LoadGrammars();

            // Enable the SpeechRecognizer
            EnableSpeechRecognizer();

            UpdateTabItemSpeechRecognizer();
        }

        void _buttonDispose_Click(object sender, RoutedEventArgs e)
        {
            // First unload all the grammars then dispose grammars created by GrammarBuilders.
            // Other grammars will not be disposed since they will be used by other
            // recognition engines later.
            UnloadGrammars();
            DisposeGrammarBuilders();

            // Dispose the engine and update the controls
            DisposeRecognitionEngine();
            UpdateTabItemSpeechRecognizer();
            UpdateTabItemSpeechRecognitionEngine();
        }

        void _buttonEnable_Click(object sender, RoutedEventArgs e)
        {
            EnableSpeechRecognizer();

            UpdateGroupBoxRecognitionSR();
        }

        void _buttonDisable_Click(object sender, RoutedEventArgs e)
        {
            DisableSpeechRecognizer();

            UpdateGroupBoxRecognitionSR();
        }

        private void _textBoxEmulate_KeyDown(object sender, KeyEventArgs e)
        {
            if (e.Key == Key.Return)
            {
                EmulateRecognize();
            }
        }

        void _buttonEmulate_Click(object sender, RoutedEventArgs e)
        {
            EmulateRecognize();
        }

        #endregion Methods and Event Handlers for SpeechRecognizer Tab


        #region Methods and Event Handlers for SpeechRecognitionEngine Tab

        // This method enables or disables the controls in the SpeechRecognitionEngine Tab
        // according to the current state of the engine. It also creates the list of installed
        // recognizers in the system.
        private void UpdateTabItemSpeechRecognitionEngine()
        {
            bool isCreated = _engineCreated && _engineType == EngineType.SpeechRecognitionEngine;

            _buttonCreateSRE.IsEnabled = !isCreated;
            _buttonDisposeSRE.IsEnabled = isCreated;

            _comboBoxInstalledRecognizers.IsEnabled = !isCreated;

            if (!isCreated)
            {
                // Create and display the list of installed engines for SpeechRecognitionEngine
                _comboBoxInstalledRecognizers.Items.Clear();

                foreach (RecognizerInfo recognizerInfo in SpeechRecognitionEngine.InstalledRecognizers())
                {
                    ComboBoxItem itemRecognizerInfo = new ComboBoxItem();
                    itemRecognizerInfo.Content = recognizerInfo.Description;
                    itemRecognizerInfo.Tag = recognizerInfo;
                    _comboBoxInstalledRecognizers.Items.Add(itemRecognizerInfo);
                }

                // Select the first one
                if (_comboBoxInstalledRecognizers.Items.Count > 0)
                {
                    _comboBoxInstalledRecognizers.SelectedIndex = 0;
                }
                else
                {
                    throw new InvalidOperationException("No speech recognition engine is installed");
                }
            }

            // Update the Audio Input and Recognition group boxes in this tab.
            UpdateGroupBoxAudioInput();
            UpdateGroupBoxRecognitionSRE();
        }

        // This method enables or disables the controls in the Audio Input Group Box
        // according to the current state.
        private void UpdateGroupBoxAudioInput()
        {
            // Controls are active only if the engine is created
            bool isCreated = _engineCreated && _engineType == EngineType.SpeechRecognitionEngine;

            _groupBoxAudioInput.IsEnabled = isCreated;

            if (isCreated)
            {
                // Input can only be set while not recognizing
                _radioButtonDefaultAudioDevice.IsEnabled = !_isRecognizing;
                _radioButtonWaveFile.IsEnabled = !_isRecognizing;

                _textBoxWaveFile.IsEnabled = !_isRecognizing && _audioInput == AudioInput.AudioFile;
                _buttonWaveFile.IsEnabled = !_isRecognizing && _audioInput == AudioInput.AudioFile;

                if (!_isRecognizing && _audioInput != AudioInput.AudioFile)
                {
                    _textBoxWaveFile.Clear();
                }
            }
            else
            {
                _radioButtonDefaultAudioDevice.IsChecked = false;
                _radioButtonWaveFile.IsChecked = false;

                _textBoxWaveFile.Clear();
            }
        }

        // This method enables or disables the controls in the Recognition Group Box
        // according to the current state of the engine.
        private void UpdateGroupBoxRecognitionSRE()
        {
            // Controls are active only if the engine is created
            bool isCreated = _engineCreated && _engineType == EngineType.SpeechRecognitionEngine;

            _groupBoxRecognitionSRE.IsEnabled = isCreated;

            if (isCreated)
            {
                // Recognition mode can only be changed while not recognizing
                _radioButtonAsynchronous.IsEnabled = !_isRecognizing;
                _radioButtonSynchronous.IsEnabled = !_isRecognizing;

                // Emulate button and text box are only enabled while not recognizing because
                // SpeechRecognitionEngine can't emulate while recognizing
                _textBoxEmulateSRE.IsEnabled = !_isRecognizing;
                _buttonEmulateSRE.IsEnabled = !_isRecognizing;

                _buttonStart.IsEnabled = !_isRecognizing;
                if (_recognitionMode == RecognitionMode.Asynchronous)
                {
                    _buttonStop.Visibility = Visibility.Visible;
                    _buttonStop.IsEnabled = _isRecognizing;

                    _buttonCancel.Visibility = Visibility.Visible;
                    _buttonCancel.IsEnabled = _isRecognizing;
                }
                else
                {
                    _buttonStop.Visibility = Visibility.Collapsed;
                    _buttonCancel.Visibility = Visibility.Collapsed;
                }
            }
            else
            {
                _radioButtonAsynchronous.IsChecked = false;
                _radioButtonSynchronous.IsChecked = false;
            }
        }

        void _buttonCreateSRE_Click(object sender, RoutedEventArgs e)
        {
            if (_engineCreated)
            {
                if (AskForNewEngine() == false)
                {
                    return;
                }
            }

            CreateRecognitionEngine(EngineType.SpeechRecognitionEngine);

            UpdateTabItemSpeechRecognitionEngine();

            // We create the GrammarBuilders after creating the engine becuase
            // the culture of each GrammarBuilder must match the culture of
            // the engine.
            CreateGrammarBuilders();

            LoadGrammars();

            // Set the input to default audio device.
            _radioButtonDefaultAudioDevice.IsChecked = true;

            // The default recognition method is asynchronous.
            _radioButtonAsynchronous.IsChecked = true;
        }

        void _radioButtonDefaultAudioDevice_Checked(object sender, RoutedEventArgs e)
        {
            SetInputToDefaultAudioDevice();
            UpdateGroupBoxAudioInput();
        }

        void _radioButtonWaveFile_Checked(object sender, RoutedEventArgs e)
        {
            // If input can be successfully set to wave file then update the controls,
            // else set input to default audio device
            if (SetInputToWaveFile())
            {
                UpdateGroupBoxAudioInput();
            }
            else
            {
                _radioButtonDefaultAudioDevice.IsChecked = true;
            }
        }

        void _buttonWaveFile_Click(object sender, RoutedEventArgs e)
        {
            // If input can't be successfully set to new wave file then
            // set input to default audio device
            if (SetInputToWaveFile() == false)
            {
                _radioButtonDefaultAudioDevice.IsChecked = true;
            }
        }

        void _radioButtonAsynchronous_Checked(object sender, RoutedEventArgs e)
        {
            PrepareForAsynchronousRecognition();
            UpdateGroupBoxRecognitionSRE();
        }

        void _radioButtonSynchronous_Checked(object sender, RoutedEventArgs e)
        {
            PrepareForSynchronousRecognition();
            UpdateGroupBoxRecognitionSRE();
        }

        void _buttonStart_Click(object sender, RoutedEventArgs e)
        {
            StartRecognition();

            UpdateGroupBoxAudioInput();
            UpdateGroupBoxRecognitionSRE();
        }

        void _buttonStop_Click(object sender, RoutedEventArgs e)
        {
            _speechRecognitionEngine.RecognizeAsyncStop();

            UpdateGroupBoxAudioInput();
            UpdateGroupBoxRecognitionSRE();
        }

        void _buttonCancel_Click(object sender, RoutedEventArgs e)
        {
            _speechRecognitionEngine.RecognizeAsyncCancel();
            _isRecognizing = false;

            UpdateGroupBoxAudioInput();
            UpdateGroupBoxRecognitionSRE();
        }

        #endregion Methods and Event Handlers for SpeechRecognitionEngine Tab


        #region Methods and Event Handlers for Grammars GroupBox

        // This method initializes the Grammar Group Box by creating three items in the
        // tree view for the three grammar types: Dictation Grammars, GrammarBuilders and
        // SRGS Grammars.
        private void InitializeGroupBoxGrammars()
        {
            // The background brush of the tree view is used for animation which takes
            // place when the recognition is rejected
            _treeViewGrammarsBackground = new SolidColorBrush(_colorDefaultBackground);
            _treeViewGrammars.Background = _treeViewGrammarsBackground;

            _dictationGrammars = new TreeViewItem();
            _dictationGrammars.Header = "Dictation Grammars";
            _treeViewGrammars.Items.Add(_dictationGrammars);


            _grammarBuilders = new TreeViewItem();
            _grammarBuilders.Header = "GrammarBuilders";
            _treeViewGrammars.Items.Add(_grammarBuilders);


            _SRGSGrammars = new TreeViewItem();
            _SRGSGrammars.Header = "SRGS Grammars";
            _treeViewGrammars.Items.Add(_SRGSGrammars);
        }

        // We create the dictation grammars only once
        private void CreateDictationGrammars()
        {
            Grammar defaultDictationGrammar = new DictationGrammar();
            defaultDictationGrammar.Name = "Default Dictation";
            defaultDictationGrammar.Enabled = true;
            _dictationGrammars.Items.Add(CreateGrammarItem(defaultDictationGrammar));

            Grammar spellingDictationGrammar = new DictationGrammar("grammar:dictation#spelling");
            spellingDictationGrammar.Name = "Spelling Dictation";
            spellingDictationGrammar.Enabled = false;
            _dictationGrammars.Items.Add(CreateGrammarItem(spellingDictationGrammar));

            _dictationGrammars.IsExpanded = true;
        }

        // We create the GrammarBuilder objects each time after creating the recognizers
        // because the culture of the GrammarBuilder must match the culture of the recognizer
        private void CreateGrammarBuilders()
        {
            System.Globalization.CultureInfo culture = GetCulture();
            
            Grammar flightStatusGrammar = FlightStatusGrammarBuilder(culture);
            flightStatusGrammar.Name = "Flight Status Grammar";
            flightStatusGrammar.Enabled = true;
            _grammarBuilders.Items.Add(CreateGrammarItem(flightStatusGrammar));

            _grammarBuilders.IsExpanded = true;
        }

        private void DisposeGrammarBuilders()
        {
            _grammarBuilders.Items.Clear();
        }

        // This method creates a TreeViewItem which will be placed under one of the three
        // nodes in the grammar tree view. Each item has a check box for enabling and disabling
        // it; a label for displaying the name of the grammar and a brush for animation which
        // takes place when a speech event related to the grammar is raised.
        private TreeViewItem CreateGrammarItem(Grammar grammar)
        {
            TreeViewItem grammarItem = new TreeViewItem();

            // ChechBox and Label will be contained in a DockPanel
            DockPanel dockPanel = new DockPanel();

            CheckBox checkBox = new CheckBox();
            checkBox.VerticalAlignment = VerticalAlignment.Center;
            checkBox.IsChecked = grammar.Enabled;
            checkBox.Tag = grammar;
            checkBox.Checked += new RoutedEventHandler(checkBoxGrammarItem_Checked);
            checkBox.Unchecked += new RoutedEventHandler(checkBoxGrammarItem_Unchecked);

            Label label = new Label();
            label.Content = grammar.Name;
            label.Tag = grammar;

            // Brush will be used for animation. We use a dictionary to retrieve the corresponding
            // brush for each grammar
            SolidColorBrush brush = new SolidColorBrush(_colorDefaultBackground);
            label.Background = brush;
            _dictionaryBrushes.Add(grammar, brush);

            dockPanel.Children.Add(checkBox);
            dockPanel.Children.Add(label);

            grammarItem.Header = dockPanel;
            grammarItem.Tag = grammar;
            grammarItem.Selected += new RoutedEventHandler(grammarItem_Selected);
            grammarItem.Unselected += new RoutedEventHandler(grammarItem_Unselected);

            return grammarItem;
        }

        // When the CheckBox is checked for a grammar, enable it
        void checkBoxGrammarItem_Checked(object sender, RoutedEventArgs e)
        {
            CheckBox checkBox = (CheckBox)sender;
            Grammar grammar = (Grammar)(checkBox.Tag);

            grammar.Enabled = true;
            LogLine(grammar.Name + " enabled");
        }

        // When the CheckBox is unchecked for a grammar, disable it
        void checkBoxGrammarItem_Unchecked(object sender, RoutedEventArgs e)
        {
            CheckBox checkBox = (CheckBox)sender;
            Grammar grammar = (Grammar)(checkBox.Tag);

            grammar.Enabled = false;
            LogLine(grammar.Name + " disabled");
        }

        void grammarItem_Selected(object sender, RoutedEventArgs e)
        {
            TreeViewItem grammarItem = (TreeViewItem)sender;

            // If the selected grammar is an SRGS Grammar then we enable the
            // "Remove SRGS" button
            if (_SRGSGrammars.Items.Contains(grammarItem))
            {
                _buttonRemoveSRGS.IsEnabled = true;
            }
        }

        void grammarItem_Unselected(object sender, RoutedEventArgs e)
        {
            _buttonRemoveSRGS.IsEnabled = false;
        }

        // When this button is clicked, display an OpenFileDialog and load the
        // grammar from the file
        void _buttonAddSRGS_Click(object sender, RoutedEventArgs e)
        {
            OpenFileDialog dialog = new OpenFileDialog();
            dialog.Filter = "SRGS Grammars (*.xml;*.grxml)|*.xml;*.grxml";
            dialog.Title = "Select SRGS Grammar";

            Nullable<bool> result = dialog.ShowDialog();

            if (result == true)
            {
                string fileName = dialog.FileName;
                Log("Opening grammar " + fileName + "... ");
                Grammar grammar;

                try
                {
                    grammar = new Grammar(fileName);
                    if (grammar.Name.Length == 0)
                    {
                        grammar.Name = fileName;
                    }
                }
                catch (FormatException exception)
                {
                    LogLine("Failed");
                    LogLine(exception.Message);
                    return;
                }
                catch (InvalidOperationException exception)
                {
                    LogLine("Failed");
                    LogLine(exception.Message);
                    return;
                }

                LogLine("Done");

                // The grammar is successfully opened so add it to the list of SRGS Grammars
                TreeViewItem grammarItem = CreateGrammarItem(grammar);
                _SRGSGrammars.Items.Add(grammarItem);
                _SRGSGrammars.IsExpanded = true;

                if (_engineCreated)
                {
                    // If the engine is already created then try to load the grammar into the engine.
                    // Disable the item control if the load operation fails.
                    grammarItem.IsEnabled = LoadGrammar(grammar);
                }
                else
                {
                    grammarItem.IsEnabled = true;
                }
            }
        }

        void _buttonRemoveSRGS_Click(object sender, RoutedEventArgs e)
        {
            _buttonRemoveSRGS.IsEnabled = false;

            TreeViewItem selectedGrammarItem = (TreeViewItem)_treeViewGrammars.SelectedItem;

            // Unload and dispose the selected grammar if it's an SRGS Grammar.
            if (_SRGSGrammars.Items.Contains(selectedGrammarItem))
            {
                if (_engineCreated && selectedGrammarItem.IsEnabled)
                {
                    UnloadGrammar((Grammar)selectedGrammarItem.Tag);
                }
                _SRGSGrammars.Items.Remove(selectedGrammarItem);
            }
        }

        #endregion Methods and Event Handlers for Grammars GroupBox


        #region Methods and Event Handlers for Controls for displaying the status
        
        // This method initializes the status controls which are used to display the
        // status and result of the recognition operation.
        private void InitializeStatusControls()
        {
            // Create the paragraph which is used to display the recognition result
            // and add it to the flow document
            _paragraphStatus = new Paragraph();
            _flowDocumentStatus.Blocks.Add(_paragraphStatus);

            // Initially there's no recognized audio
            SetRecognizedAudio(null);
        }

        // Formats the message and the arguments and then displays it in the rich text box.
        private void Log(string message, params object[] args)
        {
            string str = string.Format(System.Globalization.CultureInfo.CurrentUICulture, message, args);
            _paragraphStatus.Inlines.Add(str);
            _richTextBoxStatus.ScrollToEnd();
        }

        // Formats the message and the arguments and then displays it in the rich text box.
        // The next log starts in a new line.
        private void LogLine(string message, params object[] args)
        {
            string str = string.Format(System.Globalization.CultureInfo.CurrentUICulture, message, args);
            _paragraphStatus.Inlines.Add(str);
            _paragraphStatus.Inlines.Add(new LineBreak());
            _richTextBoxStatus.ScrollToEnd();
        }

        // A specialized log for reporting events in different colors.
        private void LogEvent(Brush color, string eventName, string message, params object[] args)
        {
            string str = string.Format(System.Globalization.CultureInfo.CurrentUICulture, message, args);
            Run run = new Run("Event " + eventName + ": " + str);
            run.Foreground = color;
            _paragraphStatus.Inlines.Add(run);
            _paragraphStatus.Inlines.Add(new LineBreak());
            _richTextBoxStatus.ScrollToEnd();
        }

        // First clears the semantics tree view and then create the nodes using semanticValue
        private void DisplaySemantics(SemanticValue semanticValue)
        {
            _treeViewSemantics.Items.Clear();

            if (semanticValue != null)
            {
                // Create the root item
                _treeViewSemantics.Items.Add(CreateSemanticItem("root", semanticValue));
            }
        }

        // Creates the semantic items recursively.
        private TreeViewItem CreateSemanticItem(string key, SemanticValue semanticValue)
        {
            if (semanticValue == null)
            {
                return null;
            }
            else
            {
                // The semantics item will be displayed as "key = value". If there's no
                // value then just the "key" will be displayed               
                TreeViewItem item = new TreeViewItem();
                string header = key;
                if (semanticValue.Value != null)
                {
                    header += " = " + semanticValue.Value.ToString();
                }
                item.Header = header;

                // Expand the item
                item.IsExpanded = true;

                // Generate the child items recursively
                foreach (KeyValuePair<String, SemanticValue> child in semanticValue)
                {
                    item.Items.Add(CreateSemanticItem(child.Key, child.Value));
                }

                return item;
            }
        }

        // Loads the recognizedAudio into a memory stream and creates a Soundplayer object
        // for playing the audio. Passing null just disables the button
        private void SetRecognizedAudio(RecognizedAudio recognizedAudio)
        {
            if (recognizedAudio == null)
            {
                _recognizedAudioStream = null;
                _recognizedAudioPlayer = null;

                _buttonRecognizedAudio.IsEnabled = false;
            }
            else
            {
                _recognizedAudioStream = new MemoryStream();
                recognizedAudio.WriteToWaveStream(_recognizedAudioStream);
                _recognizedAudioStream.Position = 0;
                _recognizedAudioPlayer = new System.Media.SoundPlayer(_recognizedAudioStream);

                _buttonRecognizedAudio.IsEnabled = true;
            }
        }

        void _buttonRecognizedAudio_Click(object sender, RoutedEventArgs e)
        {
            // Play the recognized audio
            _recognizedAudioPlayer.Play();
        }

        #endregion Methods and Event Handlers for Controls for displaying the status


        #region Methods for Speech Recognition Engines

        private void CreateRecognitionEngine(EngineType engineType)
        {
            _engineType = engineType;

            // Create the paragraph and add it to the flow document which will
            // display the result. Recognition result for each individual engine
            // will be on a seperate paragraph.
            _paragraphResult = new Paragraph();
            _hypothesis = null;
            _flowDocumentResult.Blocks.Add(_paragraphResult);

            if (_engineType == EngineType.SpeechRecognizer)
            {
                Log("Creating SpeechRecognizer... ");
                _speechRecognizer = new SpeechRecognizer();
                _labelEngineState.Content = _speechRecognizer.State.ToString();

                // Register the events
                _speechRecognizer.StateChanged += 
                    new EventHandler<StateChangedEventArgs>(SpeechEngine_StateChanged);
                _speechRecognizer.AudioLevelUpdated += 
                    new EventHandler<AudioLevelUpdatedEventArgs>(SpeechEngine_AudioLevelUpdated);
                _speechRecognizer.SpeechDetected += 
                    new EventHandler<SpeechDetectedEventArgs>(SpeechEngine_SpeechDetected);
                _speechRecognizer.SpeechHypothesized += 
                    new EventHandler<SpeechHypothesizedEventArgs>(SpeechEngine_SpeechHypothesized);
                _speechRecognizer.SpeechRecognized += 
                    new EventHandler<SpeechRecognizedEventArgs>(SpeechEngine_SpeechRecognized);
                _speechRecognizer.SpeechRecognitionRejected += 
                    new EventHandler<SpeechRecognitionRejectedEventArgs>(SpeechEngine_SpeechRecognitionRejected);
            }
            else
            {
                Log("Creating SpeechRecognitionEngine... ");
                ComboBoxItem selectedItem = (ComboBoxItem)_comboBoxInstalledRecognizers.SelectedItem;
                RecognizerInfo selectedRecognizer = (RecognizerInfo)selectedItem.Tag;
                _speechRecognitionEngine = new SpeechRecognitionEngine(selectedRecognizer);

                _isRecognizing = false;

                // Register the events
                _speechRecognitionEngine.RecognizeCompleted += 
                    new EventHandler<RecognizeCompletedEventArgs>(SpeechEngine_RecognizeCompleted);
                _speechRecognitionEngine.AudioLevelUpdated += 
                    new EventHandler<AudioLevelUpdatedEventArgs>(SpeechEngine_AudioLevelUpdated);
                _speechRecognitionEngine.SpeechDetected += 
                    new EventHandler<SpeechDetectedEventArgs>(SpeechEngine_SpeechDetected);
                _speechRecognitionEngine.SpeechHypothesized += 
                    new EventHandler<SpeechHypothesizedEventArgs>(SpeechEngine_SpeechHypothesized);
                _speechRecognitionEngine.SpeechRecognized += 
                    new EventHandler<SpeechRecognizedEventArgs>(SpeechEngine_SpeechRecognized);
                _speechRecognitionEngine.SpeechRecognitionRejected += 
                    new EventHandler<SpeechRecognitionRejectedEventArgs>(SpeechEngine_SpeechRecognitionRejected);
            }

            _engineCreated = true;
            LogLine("Done");
        }

        private void DisposeRecognitionEngine()
        {
            if (_engineType == EngineType.SpeechRecognizer)
            {
                Log("Disposing SpeechRecognizer... ");
                _speechRecognizer.Dispose();
                _speechRecognizer = null;

                _labelEngineState.Content = "N/A";
            }
            else
            {
                Log("Disposing SpeechRecognitionEngine... ");
                _speechRecognitionEngine.Dispose();
                _speechRecognitionEngine = null;
            }

            _engineCreated = false;
            SetRecognizedAudio(null);
            LogLine("Done");
        }

        private bool AskForNewEngine()
        {
            string SR = "System.Speech.Recognition.SpeechRecognizer";
            string SRE = "System.Speech.Recognition.SpeechRecognitionEngine";
            string currentEngine, newEngine;

            if (_engineType == EngineType.SpeechRecognizer)
            {
                currentEngine = SR;
                newEngine = SRE;
            }
            else
            {
                currentEngine = SRE;
                newEngine = SR;
            }

            MessageBoxResult result = MessageBox.Show(
                "This tool allows you to use only one engine at a time. You have to dispose the " +
                currentEngine + " object that you've created before in order to create a new engine. " +
                "Do you want me to dispose it now and create a new " + newEngine + "?",
                
                "Engine Already Created", MessageBoxButton.YesNo);

            if (result == MessageBoxResult.Yes)
            {
                // First unload all the grammars then dispose grammars created by GrammarBuilders.
                // Other grammars will not be disposed since they will be used by other
                // recognition engines later.
                UnloadGrammars();
                DisposeGrammarBuilders();

                // Dispose the engine and update the controls
                DisposeRecognitionEngine();
                UpdateTabItemSpeechRecognizer();
                UpdateTabItemSpeechRecognitionEngine();

                return true;
            }
            else
            {
                return false;
            }
        }

        private System.Globalization.CultureInfo GetCulture()
        {
            if (_engineType == EngineType.SpeechRecognizer)
            {
                return _speechRecognizer.RecognizerInfo.Culture;
            }
            else
            {
                return _speechRecognitionEngine.RecognizerInfo.Culture;
            }
        }

        private void LoadGrammars()
        {
            LogLine("Loading grammars");
            LoadGrammars(_dictationGrammars);
            LoadGrammars(_grammarBuilders);
            LoadGrammars(_SRGSGrammars);
        }

        private void LoadGrammars(TreeViewItem grammars)
        {
            foreach (TreeViewItem grammarItem in grammars.Items)
            {
                grammarItem.IsEnabled = LoadGrammar((Grammar)grammarItem.Tag);
            }
        }

        private bool LoadGrammar(Grammar grammar)
        {
            try
            {
                if (_engineType == EngineType.SpeechRecognizer)
                {
                    _speechRecognizer.LoadGrammar(grammar);
                }
                else
                {
                    _speechRecognitionEngine.LoadGrammar(grammar);
                }
            }
            catch (InvalidOperationException e)
            {
                LogLine("Failed to load grammar {0}: {1}", grammar.Name, e.Message);
                return false;
            }
            return true;
        }

        private bool UnloadGrammar(Grammar grammar)
        {
            try
            {
                if (_engineType == EngineType.SpeechRecognizer)
                {
                    _speechRecognizer.UnloadGrammar(grammar);
                }
                else
                {
                    _speechRecognitionEngine.UnloadGrammar(grammar);
                }
            }
            catch (InvalidOperationException e)
            {
                LogLine("Failed to unload grammar {0}: {1}", grammar.Name, e.Message);
                return false;
            }
            return true;
        }

        private void UnloadGrammars()
        {
            Log("Unloading grammars... ");
            if (_engineType == EngineType.SpeechRecognizer)
            {
                _speechRecognizer.UnloadAllGrammars();
            }
            else
            {
                _speechRecognitionEngine.UnloadAllGrammars();
            }
            LogLine("Done");

            EnableGrammarItems(_dictationGrammars);
            EnableGrammarItems(_grammarBuilders);
            EnableGrammarItems(_SRGSGrammars);
        }

        static private void EnableGrammarItems(TreeViewItem grammars)
        {
            foreach (TreeViewItem grammarItem in grammars.Items)
            {
                grammarItem.IsEnabled = true;
            }
        }

        private bool SetInputToWaveFile()
        {
            OpenFileDialog dialog = new OpenFileDialog();
            dialog.DefaultExt = ".wav";
            dialog.Filter = "Audio Files (*.wav)|*.wav";
            dialog.Title = "Select Audio Input file";

            Nullable<bool> result = dialog.ShowDialog();

            if (result == true)
            {
                string fileName = dialog.FileName;
                Log("Setting input to wave file " + fileName + "... ");

                try
                {
                    _speechRecognitionEngine.SetInputToWaveFile(fileName);
                }
                catch (FormatException)
                {
                    LogLine("Failed");
                    LogLine(fileName + " is not a valid wave file.");

                    return false;
                }
                catch (InvalidOperationException e)
                {
                    LogLine("Failed");
                    LogLine(e.Message);

                    return false;
                }

                _textBoxWaveFile.Text = fileName;
                _audioInput = AudioInput.AudioFile;
                LogLine("Done");

                return true;
            }
            else
            {
                return false;
            }
        }

        private void SetInputToDefaultAudioDevice()
        {
            Log("Setting input to default audio device... ");
            _speechRecognitionEngine.SetInputToDefaultAudioDevice();
            _audioInput = AudioInput.DefaultAudioDevice;
            LogLine("Done");
        }

        private void EnableSpeechRecognizer()
        {
            Log("Enabling SpeechRecognizer... ");
            _speechRecognizer.Enabled = true;
            LogLine("Done");
        }

        private void DisableSpeechRecognizer()
        {
            Log("Disabling SpeechRecognizer... ");
            _speechRecognizer.Enabled = false;
            LogLine("Done");
        }

        private void PrepareForAsynchronousRecognition()
        {
            _recognitionMode = RecognitionMode.Asynchronous;
            LogLine("Ready for asynchronous recognition");
        }

        private void PrepareForSynchronousRecognition()
        {
            _recognitionMode = RecognitionMode.Synchronous;
            LogLine("Ready for synchronous recognition");
        }

        // Calls either the RecognizeAsync or Recognize method of SpeechRecognitionEngine
        // class depending on the recognition mode selection of the user.
        private bool StartRecognition()
        {
            if (_recognitionMode == RecognitionMode.Asynchronous)
            {
                Log("Starting asynchronous recognition... ");
                try
                {
                    _speechRecognitionEngine.RecognizeAsync(RecognizeMode.Multiple);
                }
                catch (InvalidOperationException e)
                {
                    _isRecognizing = false;
                    LogLine("Failed");
                    LogLine(e.Message);
                    return false;
                }

                _isRecognizing = true;
                LogLine("Done, Please Speak");
                return true;
            }
            else
            {
                LogLine("Recognizing synchronously");
                try
                {
                    _speechRecognitionEngine.Recognize();
                }
                catch (InvalidOperationException e)
                {
                    LogLine(e.Message);
                    return false;
                }
                return true;
            }
        }

        // Emulates recognition by calling EmulateRocgnizeAsync method.
        private void EmulateRecognize()
        {
            try
            {
                if (_engineType == EngineType.SpeechRecognizer)
                {
                    _speechRecognizer.EmulateRecognizeAsync(_textBoxEmulateSR.Text);
                }
                else
                {
                    _speechRecognitionEngine.EmulateRecognizeAsync(_textBoxEmulateSRE.Text);
                }
            }
            catch (ArgumentException)
            {
                LogLine("Please enter a text to emulate");
            }
            catch (InvalidOperationException e)
            {
                LogLine("Can't emulate: " + e.Message);
            }
        }

        #endregion Methods for Speech Recognition Engines


        #region Event Handlers for Speech Recognition Engines

        // StateChanged event is generated when the state of the SpeechRecognizer
        // object changes
        void SpeechEngine_StateChanged(object sender, StateChangedEventArgs e)
        {
            _labelEngineState.Content = e.RecognizerState.ToString();
            LogEvent(_brushDefault, "StateChanged", "RecognizerState={0}", e.RecognizerState);
        }

        // RecognizeCompleted event is generated by SpeechRecognitionEngine when
        // an asynchronous recognition operation completes.
        void SpeechEngine_RecognizeCompleted(object sender, RecognizeCompletedEventArgs e)
        {
            _isRecognizing = false;

            UpdateGroupBoxAudioInput();
            UpdateGroupBoxRecognitionSRE();
        }

        // AudioLevelUpdated event is generated by recognition engines (both SpeechRecognizer and
        // SpeechRecognitionEngine) when there's a change in the input audio level.
        void SpeechEngine_AudioLevelUpdated(object sender, AudioLevelUpdatedEventArgs e)
        {
            _progressBarAudioLevel.Value = e.AudioLevel;
        }

        // SpeechDetected event is generated by recognition engines (both SpeechRecognizer and
        // SpeechRecognitionEngine) when speech is detected.
        void SpeechEngine_SpeechDetected(object sender, SpeechDetectedEventArgs e)
        {
            LogEvent(_brushDefault, "SpeechDetected", "AudioPosition={0}", e.AudioPosition);
        }

        // SpeechHypothesized event is generated by recognition engines (both SpeechRecognizer and
        // SpeechRecognitionEngine) when part of the audio input speech has been tentatively recognized.
        // This Method displays the last hypothesis in different color temporarily in the result text box
        // and starts a background color animation on the grammar which generated the hypothesis.
        void SpeechEngine_SpeechHypothesized(object sender, SpeechHypothesizedEventArgs e)
        {
            LogEvent(_brushHypothesized, "SpeechHypothesized", "Confidence={0:0.00} Grammar={1} Hypothesis=\"{2}\"",
                    e.Result.Confidence, e.Result.Grammar.Name, e.Result.Text);

            if (_hypothesis != null)
            {
                // Remove the previous hypothesis from the result.
                _paragraphResult.Inlines.Remove(_hypothesis);
            }
            // Display the new hypothesis in a different color.
            _hypothesis = new Run(e.Result.Text);
            _hypothesis.Foreground = _brushHypothesized;
            _paragraphResult.Inlines.Add(_hypothesis);
            _richTextBoxResult.ScrollToEnd();

            // Get the background brush for the grammar which generated the hypothesis.
            SolidColorBrush brush = _dictionaryBrushes[e.Result.Grammar];
            // Start a two second color animation.
            ColorAnimation animation = new ColorAnimation(_colorHypothesized, _colorDefaultBackground, new Duration(TimeSpan.FromSeconds(2)));
            brush.BeginAnimation(SolidColorBrush.ColorProperty, animation);
        }

        // SpeechRecognized event is generated by recognition engines (both SpeechRecognizer and
        // SpeechRecognitionEngine) when speech has been recognized.
        // This Method displays the recognition result in the result text box, starts a background
        // color animation on the grammar which recognized the speech, displays the semantic
        // information and stores the recognized audio so it can be listened later.
        void SpeechEngine_SpeechRecognized(object sender, SpeechRecognizedEventArgs e)
        {
            LogEvent(_brushRecognized, "SpeechRecognized", "Confidence={0:0.00} Grammar={1} Result=\"{2}\"",
                    e.Result.Confidence, e.Result.Grammar.Name, e.Result.Text);

            // Remove the hypothesis from the result if there's any.
            if (_hypothesis != null)
            {
                _paragraphResult.Inlines.Remove(_hypothesis);
                _hypothesis = null;
            }

            // Display the result.
            Run result = new Run(e.Result.Text + ".  ");
            result.Foreground = _brushRecognized;
            _paragraphResult.Inlines.Add(result);
            _richTextBoxResult.ScrollToEnd();

            // Get the background brush for the grammar which recognized the speech.
            SolidColorBrush brush = _dictionaryBrushes[e.Result.Grammar];
            // Start a two second color animation.
            ColorAnimation animation = new ColorAnimation(_colorRecognized, _colorDefaultBackground, new Duration(TimeSpan.FromSeconds(3)));
            brush.BeginAnimation(SolidColorBrush.ColorProperty, animation);

            // Display the semantics for the result.
            DisplaySemantics(e.Result.Semantics);

            // Store the reconized audio.
            SetRecognizedAudio(e.Result.Audio);
        }

        // SpeechRecognitionRejected event is generated by recognition engines (both SpeechRecognizer and
        // SpeechRecognitionEngine) when the engine detects speech, but can only return candidate phrases
        // with low confidence levels.
        void SpeechEngine_SpeechRecognitionRejected(object sender, SpeechRecognitionRejectedEventArgs e)
        {
            LogEvent(_brushRejected, "SpeechRecognitionRejected", "Confidence={0:0.00}", e.Result.Confidence);

            if (_hypothesis != null)
            {
                _paragraphResult.Inlines.Remove(_hypothesis);
                _hypothesis = null;
            }

            // Start a color animation.
            ColorAnimation animation = new ColorAnimation(_colorRejected, _colorDefaultBackground, new Duration(TimeSpan.FromSeconds(1)));
            _treeViewGrammarsBackground.BeginAnimation(SolidColorBrush.ColorProperty, animation);
        }

        #endregion Event Handlers for Speech Recognition Engines


        #region Sample GrammarBuilders

        // Recognizes phrases like:
        // Please check the status of yesterday's flight from Seattle to Boston
        // What is the status of today's flight from Sea-Tac to T. F. Green
        // Status of tomorrow's flight from Providence to Boston Logan International Airport
        static private Grammar FlightStatusGrammarBuilder(System.Globalization.CultureInfo culture)
        {
            //Providence T. F. Green Internatinoal Airport
            GrammarBuilder PVD = new GrammarBuilder();
            Choices PVDchoices = new Choices("Providence",
                                             "T. F. Green",
                                             "Providence T. F. Green Internatinoal Airport");
            PVD.Append(PVDchoices);
            PVD.Append(new SemanticResultValue("PVD"));


            //Boston Logan Internatinoal Airport
            GrammarBuilder BOS = new GrammarBuilder();
            Choices BOSchoices = new Choices("Boston",
                                             "Logan",
                                             "Boston Logan Internatinoal Airport");
            BOS.Append(BOSchoices);
            BOS.Append(new SemanticResultValue("BOS"));


            //Seattle-Tacoma International Airport
            GrammarBuilder SEA = new GrammarBuilder();
            Choices SEAchoices = new Choices("Seattle",
                                             "Tacoma",
                                             "Sea-Tac",
                                             "Seattle-Tacoma Internatinoal Airport");
            SEA.Append(SEAchoices);
            SEA.Append(new SemanticResultValue("SEA"));


            //All airports
            GrammarBuilder airports = new GrammarBuilder(new Choices(PVD, BOS, SEA));


            //Dates
            DateTime today = DateTime.Today;
            DateTime yesterday = today.AddDays(-1.0);
            DateTime tomorrow = today.AddDays(1.0);

            Choices datesChoices = new Choices();
            datesChoices.Add(new GrammarBuilder(new SemanticResultValue("yesterday's", yesterday.ToShortDateString())),
                             new GrammarBuilder(new SemanticResultValue("today's", today.ToShortDateString())),
                             new GrammarBuilder(new SemanticResultValue("tomorrow's", tomorrow.ToShortDateString())));
            GrammarBuilder dates = new GrammarBuilder(datesChoices);


            //Final Grammar
            GrammarBuilder flightStatusGrammar = new GrammarBuilder();
            flightStatusGrammar.Culture = culture;

            flightStatusGrammar.Append(new Choices("Please check the status of",
                                                   "What is the status of",
                                                   "Status of"));
            flightStatusGrammar.Append(new SemanticResultKey("date", dates));
            flightStatusGrammar.Append("flight from");                                                   
            flightStatusGrammar.Append(new SemanticResultKey("from", airports));
            flightStatusGrammar.Append("to");
            flightStatusGrammar.Append(new SemanticResultKey("to", airports));

            return new Grammar(flightStatusGrammar);
        }

        #endregion Sample GrammarBuilders


        #region IDisposable Members

        public void Dispose()
        {
            Dispose(true);
            // Take yourself off the Finalization queue 
            // to prevent finalization code for this object
            // from executing a second time.
            GC.SuppressFinalize(this);
        }

        private void Dispose(bool disposing)
        {
            // Check to see if Dispose has already been called.
            if (!disposed)
            {
                // If disposing equals true, dispose all managed 
                // and unmanaged resources.
                if (disposing)
                {
                    // Dispose managed resources.
                    if (_speechRecognizer != null)
                    {
                        _speechRecognizer.Dispose();
                        _speechRecognizer = null;
                    }

                    if (_speechRecognitionEngine != null)
                    {
                        _speechRecognitionEngine.Dispose();
                        _speechRecognitionEngine = null;
                    }

                    if (_recognizedAudioStream != null)
                    {
                        _recognizedAudioStream.Dispose();
                        _recognizedAudioStream = null;
                    }

                    if (_recognizedAudioPlayer != null)
                    {
                        _recognizedAudioPlayer.Dispose();
                        _recognizedAudioPlayer = null;
                    }
                }
                // There are no unmanaged resources to release.
            }
            disposed = true;
        }

        #endregion
    }
}

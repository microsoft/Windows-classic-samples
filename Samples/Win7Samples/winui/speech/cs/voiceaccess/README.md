# Voice Access User Control Sample

This sample demonstrates a WPF UserControl that displays the current mode (Translator/Recognizer) based on a configuration flag.

## Features

- **Mode Display**: Shows current mode in a styled TextBlock at the top of the control
- **Mode Toggle**: Button to switch between Translator Mode and Recognizer Mode
- **Status Updates**: Real-time status messages with timestamps
- **Property Change Notifications**: Implements INotifyPropertyChanged for data binding
- **Clean UI**: Professional styling that matches Windows application standards

## Architecture

### Key Components

1. **VoiceAccessUserControl.xaml**: XAML layout defining the user interface
2. **VoiceAccessUserControl.xaml.cs**: C# code-behind with the control logic
3. **VoiceAccessUserControl.cpp**: C++/CLI equivalent implementation

### Core Functionality

- `m_useTranslatorConfig` flag controls the mode display
- `ModeText` property shows "Translator Mode" or "Recognizer Mode"
- Property change notifications update the UI automatically
- Event handlers for mode toggling and start/stop operations

## Implementation Details

### Mode Logic
```csharp
private void UpdateModeText()
{
    ModeText = m_useTranslatorConfig ? "Translator Mode" : "Recognizer Mode";
}
```

### Property Change Notification
```csharp
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
```

## UI Elements

- **Mode Display Area**: Styled border with centered text showing current mode
- **Control Panel**: Buttons for mode toggle, start, and stop operations
- **Status Area**: Scrollable text area for status messages

## Building and Running

### Prerequisites
- Visual Studio 2008 or later
- .NET Framework 3.5 or later

### Build Instructions
1. Open `VoiceAccess.sln` in Visual Studio
2. Build the solution (F6)
3. Run the application (F5)

### Testing
1. Click "Toggle Mode" to switch between modes
2. Observe the mode text changes in the display area
3. Click "Start" to simulate starting the service
4. Check status messages for real-time feedback

## Usage Example

```csharp
// Create the control
var voiceControl = new VoiceAccessUserControl();

// Set to translator mode
voiceControl.UseTranslatorConfig = true;

// The UI will automatically show "Translator Mode"
```

## C++ Implementation

The C++/CLI version (`VoiceAccessUserControl.cpp`) provides equivalent functionality for native C++ applications:

- Same XAML interface
- C++/CLI property definitions
- Event handler implementations
- Property change notifications

## Styling

The control uses a professional blue theme with:
- Mode display: Light blue background with dark blue border
- Proper spacing and padding
- Clear typography hierarchy
- Responsive layout design

## Future Enhancements

- Integration with actual speech recognition services
- Support for multiple languages
- Voice command configuration
- Audio level visualization
- Configuration persistence
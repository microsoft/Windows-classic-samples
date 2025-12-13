# IPC Demo - Parent-Child Process Communication

This project demonstrates interprocess communication (IPC) between a full trust parent process and a sandboxed (AppContainer) child process using named pipes.

## Architecture

- **demo_parent.exe**: Full trust console application that:
  - Spawns the child process using the app execution alias (`demo_child_ac.exe`)
  - Creates a named pipe with proper security attributes to allow AppContainer access
  - Sends messages to the child and receives echoed responses
  - Manages the child process lifecycle

- **demo_child.exe**: Sandboxed (AppContainer) console application that:
  - Runs in a restricted security context
  - Connects to the named pipe created by the parent
  - Receives messages and echoes them back
  - Exits gracefully when receiving the EXIT command

## Key Features

### Security Attributes for Named Pipe

The parent process creates the named pipe with a security descriptor that allows AppContainer processes to access it:

```cpp
// SDDL: Grants Generic All to World Domain and Generic Read/Write to ALL APPLICATION PACKAGES
LPCWSTR sddl = L"D:(A;;GA;;;WD)(A;;GRGW;;;AC)";
```

This is critical for allowing the sandboxed child process to communicate with the full trust parent.

### App Execution Alias

The manifest uses the `uap5:AppExecutionAlias` feature to create aliases:
- `demo_parent.exe` - Full trust parent
- `demo_child_ac.exe` - Sandboxed child (alias for `demo_child.exe`)

The parent spawns the child using `demo_child_ac.exe`, which automatically runs it in the AppContainer sandbox.

## Building

1. Open `IPCDemo.sln` in Visual Studio 2022 or later
2. Build the solution (both Debug and Release configurations supported)
3. Executables will be output to `bin\Debug\` or `bin\Release\`

## Creating the APPX Package

To deploy this as a packaged application:

1. Create a folder structure:
   ```
   Package\
   ├── demo_parent.exe
   ├── demo_child.exe
   ├── AppxManifest.xml
   └── Images\
       ├── StoreLogo.png
       └── AppList.png
   ```

2. Copy the built executables to the Package folder

3. Create placeholder images (or use real ones):
   - StoreLogo.png (150x150)
   - AppList.png (44x44)

4. Use `MakeAppx.exe` to create the package:
   ```powershell
   MakeAppx.exe pack /d Package /p IPCDemo.appx
   ```

5. Sign the package (for testing, use a test certificate):
   ```powershell
   SignTool.exe sign /fd SHA256 /a /f TestCert.pfx IPCDemo.appx
   ```

## Running

### From Visual Studio (Debug)

Simply run the `demo_parent` project. It will automatically launch the child process.

### From Command Line (Packaged)

1. Install the package: `Add-AppxPackage .\IPCDemo.appx`
2. Run: `demo_parent.exe`

## Communication Flow

1. Parent starts and creates security descriptor
2. Parent launches child using `demo_child_ac.exe` alias
3. Parent creates named pipe with AppContainer-accessible security
4. Parent waits for child to connect
5. Child waits for pipe availability and connects
6. Parent sends "Hello from parent!" → Child echoes back
7. Parent sends "This is message number 2" → Child echoes back
8. Parent sends "EXIT" command
9. Child closes pipe and exits
10. Parent waits for child exit and closes

## Important Notes

- The child process runs in an AppContainer, which has restricted access to system resources
- The named pipe security descriptor must explicitly grant access to AppContainer processes (SID: S-1-15-2-1)
- The app execution alias mechanism ensures the child runs in the correct security context
- Both processes are console applications for easy debugging and demonstration

## Requirements

- Windows 10 version 1809 (17763) or later
- Visual Studio 2022 with C++ development tools
- Windows SDK 10.0 or later

## Troubleshooting

If the child fails to connect:
- Ensure the security descriptor on the pipe includes AppContainer access (`AC` in SDDL)
- Verify the child is being launched with the `_ac.exe` alias
- Check that the package is properly installed and the manifest is correct
- Look for error codes in the console output

## References

- [AppContainer Isolation](https://docs.microsoft.com/en-us/windows/win32/secauthz/appcontainer-isolation)
- [Named Pipes](https://docs.microsoft.com/en-us/windows/win32/ipc/named-pipes)
- [Security Descriptor String Format (SDDL)](https://docs.microsoft.com/en-us/windows/win32/secauthz/security-descriptor-string-format)
- [App Execution Alias](https://docs.microsoft.com/en-us/windows/uwp/launch-resume/execute-in-app-context)

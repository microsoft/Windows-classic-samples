---
page_type: sample
languages:
- cpp
products:
- windows
urlFragment: Biometrics
description: "Determine system support for Enhanced sign-in security (ESS) biometric authentication methods, such as facial and fingerprint recognition."
---

# Enhanced sign-in security (ESS) biometric authentication

[Enhanced sign-in security (ESS)](https://learn.microsoft.com/windows-hardware/design/device-experiences/windows-hello-enhanced-sign-in-security)
adds a level of security to biometric data with the use of specialized hardware and software components
to protect user authentication data, and to secure the data communication channel.


This sample is written in C++ and demonstrates Windows Biometric APIs that determine if the active device is Enhanced sign-in security (ESS) capable and if yes, whether ESS is enabled/disabled/enrolled.

## SDK requirements

This sample requires Windows SDK version 10.0.26100.7175 or higher.

## Operating system requirements

Windows 11 Insider Edition Beta Channel. Build Major Version = 26120 and Minor Version >= 6772.

## Building the sample

To build the sample using command prompt:

1. Open command prompt and navigate to the Biometrics cpp directory.
2. Run msbuild Biometrics.sln

To build the sample using Visual Studio:

1. Open Windows Explorer and navigate to the Biometrics cpp directory.
2. Open the .sln (solution) file using Visual Studio.
3. In the build menu, select Build Solution. The application will be built in the default Debug directory.

## Running the sample 

To run the sample using command prompt:

1. Navigate to the directory containing the new executable.
2. Run Biometrics.exe

To run the sample using Windows Explorer:

1. Navigate to the directory containing the new executable.
2. Double-click the Biometrics.exe icon to launch the executable.

---
page_type: sample
languages:
- cpp
products:
- windows-api-win32
name: IAmsiStream interface sample
urlFragment: iamsistream-sample
description: Demonstrates how to use the Antimalware Scan Interface to scan a stream. 
extendedZipContent:
- path: LICENSE
  target: LICENSE
---

# IAmsiStream interface sample

Demonstrates how to use the Antimalware Scan Interface to scan a stream.

The sample implements the [IAmsiStream](https://msdn.microsoft.com/en-us/library/windows/desktop/dn889589(v=vs.85).aspx) interface so that an antimalware provider can use it to scan the contents of a stream.

The sample demonstrates a stream where the data comes from a file and a stream where the data comes from an in-memory buffer.

## Instructions
1. Load the Project solution.
2. Go to **Project Properties, Debugging**.
3. To scan an in-memory buffer, leave the *Command Arguments* blank. To scan a file, enter the file's complete path in the *Command Arguments*.
4. Press **F5** to build and run.

## Sample output

    Creating stream object with file name: C:\sample.txt
    Calling antimalware->Scan() ...
    GetAttribute() called with: attribute = 0, bufferSize = 1
    GetAttribute() called with: attribute = 0, bufferSize = 68
    GetAttribute() called with: attribute = 1, bufferSize = 1
    GetAttribute() called with: attribute = 1, bufferSize = 28
    GetAttribute() called with: attribute = 2, bufferSize = 8
    GetAttribute() called with: attribute = 3, bufferSize = 8
    Read() called with: position = 0, size = 507
    GetAttribute() called with: attribute = 4, bufferSize = 8
    GetAttribute() called with: attribute = 6, bufferSize = 8
    GetAttribute() called with: attribute = 8, bufferSize = 8
    GetAttribute() called with: attribute = 2, bufferSize = 8
    GetAttribute() called with: attribute = 3, bufferSize = 8
    Scan result is 1. IsMalware: 0
    Provider display name: Windows Defender Antivirus
    Leaving with hr = 0x0

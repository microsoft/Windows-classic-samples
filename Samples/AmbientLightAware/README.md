Ambient light sensor sample
===========================

This sample demonstrates how to optimize a label's font size, based on light conditions reported by an Ambient Light sensor.

The sample creates a smaller font when it detects indoor lighting conditions. It creates a larger font when it detects outdoor lighting conditions.

**Note**  

This sample is not optimized for light conditions. It is intended to demonstrate one use of the sensor platform.

Related topics
--------------

[Sensor API](http://msdn.microsoft.com/en-us/library/windows/desktop/dd318953)

Operating system requirements
-----------------------------

Client

Windows 7

Server

None supported

Build the sample
----------------

To build the sample by using Visual Studio 2013 or later (preferred method):

1.  Open Windows Explorer and navigate to the directory.
2.  Double-click the icon for the **.sln** (solution) file to open the file in Visual Studio.
3.  In the **Build** menu, select **Build Solution**. The application is built in the default \\Debug or \\Release directory.

To build the sample by using the command line:

1.  Open the **Command Prompt** window and navigate to the directory.
2.  Type **msbuild ambientlightaware.sln**.

Run the sample
--------------

To run the sample:

1.  Navigate to the directory that contains the new executable, by using the command prompt.
2.  Type **ambientlightaware.exe** at the command line.


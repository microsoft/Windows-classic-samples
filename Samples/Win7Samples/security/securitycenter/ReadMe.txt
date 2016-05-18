Windows Security Center Sample

This console application demonstrates the Windows Security Center monitoring the health of the Intenet Zone settings and all Security Center protection technologies.  
The WscGetSecurityProviderHealth function returns the health state of Internet settings (OK or Not OK). The application monitors for any changes and reports these changes to the console application. 


To build this project, you must follow the steps in the SDK as listed in the readme file of the Microsoft SDK. 

1. To set the include path, open Visual Studio 2005. In Tools -> Options, expand Projects and Solutions node and locate VC++ Directories. Under Show directories, select Include files from the drop down and add the following: 
        "<Your Full SDK install Path>\include"
        "<Your Full SDK install Path>\include\gl"
        "<Your Full SDK install Path>\VC\INCLUDE"
        "<Your Full SDK install Path>\VC\INCLUDE\SYS"

Note: Ensure that the paths are listed first and before any other paths.

2. To set the include path, open Visual Studio 2005. In Tools -> Options, expand Projects and Solutions node and locate VC++ Directories. Under Show directories, select Library files from the drop down and add the following: 
        "<Your Full SDK install Path>\Lib"
        "<Your Full SDK install Path>\VC\LIB"

Note: Ensure that the paths are listed first and before any other paths.



 



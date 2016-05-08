Windows Firewall Sample
===============================
        Sample code for 'Windows Firewall with Advanced Security' COM interfaces.

        Illustrates correct usage of following methods/properties of INetFwPolicy2 COM interface
        when multiple firewall profiles are active/current at the same time:
          - CurrentProfileTypes
          - IsRuleGroupCurrentlyEnabled
          - IsRuleGroupEnabled
          - LocalPolicyModifyState


Sample Language Implementations
===============================
     This sample is available in the following language implementations:
     C++


To build the sample using the command prompt:
=============================================
     1. Open the Command Prompt window and navigate to the directory.
     2. Type msbuild <solution file>.


To build the sample using Visual Studio:
=======================================
     1. Open Windows Explorer and navigate to the  directory.
     2. Double-click the icon for the .sln (solution) file to open the file in Visual Studio.
     3. In the Build menu, select Build Solution.
     The application will be built in the default Debug/Release directory.

Additional Build Steps:
=======================
     1. nmake


To run the sample:
=================
     1. Navigate to the directory that contains the new executable, using the command prompt or Windows Explorer.
     2. Launch the executable by specifying the executable name from command line or by double-clicking it in explorer.


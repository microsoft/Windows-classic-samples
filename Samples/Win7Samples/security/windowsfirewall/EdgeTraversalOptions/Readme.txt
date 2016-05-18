Windows Firewall Samples (Illustrates how to add firewall rule with the EdgeTraversalOptions)
===============================
    Sample code for 'Windows Firewall with Advanced Security' COM interfaces.

    Illustrates how to add firewall rule with the EdgeTraversalOptions.

    Note that in order for Windows Firewall to dynamically allow edge traffic to the application 
    (i.e allow edge traffic only when the app indicates so, block otherwise) two things must be done:
    1. Application should use the IPV6_PROTECTION_LEVEL socket option on the listening socket
        and set it to PROTECTION_LEVEL_UNRESTRICTED whenever it wants to receive edge traffic. And reset it
        back to other options when edge traffic is not needed.
    2. The Windows Firewall rule added for the application should set 
            EdgeTraversalOptions = NET_FW_EDGE_TRAVERSAL_TYPE_DEFER_TO_APP

    For help on the IPV6_PROTECTION_LEVEL socket option refer Winsock reference documentation on MSDN. 

    This sample only illustrates one of the EdgeTraversalOptions values.
    You can find the complete set of Windows 7 supported EdgeTraversalOptions values by looking up the 
    enumerated type 'NET_FW_EDGE_TRAVERSAL_TYPE' for 'Windows Firewall with Advanced Security' on MSDN.
    Posting values here for quick reference but always check MSDN for correct values:
        NET_FW_EDGE_TRAVERSAL_TYPE_DENY               = 0  'always block edge traffic.
        NET_FW_EDGE_TRAVERSAL_TYPE_ALLOW              = 1  'always allow edge traffic.
        NET_FW_EDGE_TRAVERSAL_TYPE_DEFER_TO_APP       = 2  'dynamically allow edge traffic based on when app sets the IPV6_PROTECTION_LEVEL socket option to 'Unrestricted'.
        NET_FW_EDGE_TRAVERSAL_TYPE_DEFER_TO_USER      = 3  'generate Windows Security Alert to ask the user to permit edge traffic when app sets the IPV6_PROTECTION_LEVEL socket option to 'Unrestricted'.
                                                           'Note that in order to use the DEFER_TO_USER option, the firewall rule must only have application path and protocol scopes specified, nothing more, nothing less.



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
     2. Create %ProgramFiles%\EdgeTraversalOptions directory.
     3. Copy the executable EdgeTraversalOptions.exe to %ProgramFiles%\EdgeTraversalOptions directory.
     4. Launch the executable by specifying the executable name "EdgeTraversalOptions.exe" from an elevated command line or 
          by double-clicking the executable file in explorer and launching it with "Run as administrator".


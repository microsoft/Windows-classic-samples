Tablet PC Info Sample
---------------------

This program checks the presence and configuration of the Microsoft® Tablet PC Platform core components. It determines whether Tablet PC components are enabled in the operating system, and lists the names and version information for the core controls and default handwriting and speech recognizer.

The application uses the GetSystemMetrics Windows API, passing in SM_TABLETPC, to determine whether the application running on a Tablet PC. SM_TABLETPC is defined in WinUser.h.

Of particular interest is the way the application uses the Recognizers collection to provide information about the default recognizer. Before attempting to use the Recognizers collection and Recognizer object, the application tests for their successful creation.


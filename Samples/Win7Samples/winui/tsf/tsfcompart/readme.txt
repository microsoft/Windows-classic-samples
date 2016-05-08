The Compart sample demonstrates how to use the Text Services Framework (TSF) component of Windows XP to monitor the status of various TSF text services. Specifically, this sample monitors the status of the speech input.

The Compart sample implements a dialog box that displays the status of the speech input text service. The status of the text service is obtained by using the ITfCompartment interface. Status change notifications are received by implementing and installing an ITfCompartmentEventSink object.

The available text services can be seen by enabling the language bar. The language bar can be viewed by performing the following steps on Windows XP Professional.

1. Open the Control Panel
2. Open the Text Services control panel application.
3. Press the Language Bar pushbutton.
4. Check the “Show the Language bar on the desktop” checkbox.
5. Press the OK button in the Language Bar Settings dialog box.
6. Press the OK button in the Text Services control panel application.

The language bar should now be visible. The installed text services can be accessed and activated using the Language bar.

The Compart sample can be built from the Platform SDK build environment by simply running “nmake” from the Compart sample directory.


GetImage


Summary
-------

GetImage is a sample application for the demonstration of the Windows Image
Acquisition (WIA) application programming interfaces (API). 

The WIA API set contains simple API's for file transfers, but there is no 
simple API call for in-memory tranfers. 

GetImage implements a wrapper function named WiaGetImage that combines the 
functionality of SelectDeviceDlg, DeviceDlg and idtGetBandedData API's to 
completely encapsulate image acquisition within a single function call.

To compile and run GetImage, open the project file GetImage.dsp


Files
-----

    WiaWrap.cpp:       Contains wrapper functions for simpler WIA operations
    WiaWrap.h:         Header file for WiaWrap.cpp

    EventCallback.cpp: Implements the callback function for WIA device events
    EventCallback.h:   Header file for EventCallback.cpp

    DataCallback.cpp:  Implements the callback function for data transfers
    DataCallback.h:    Header file for DataCallback.cpp

    BitmapUtil.cpp:    Contains device independent bitmap (DIB) functions
    BitmapUtil.h:      Header file for BitmapUtil.cpp

    ProgressDlg.cpp:   Implements a simple progress dialog
    ProgressDlg.h:     Header file for ProgressDlg.cpp

    GetImage.cpp:      Implements the main entry point for the application
    GetImage.dsp:      VC 6.0 project settings file
    GetImage.rc:       Contains the project resources
    resource.h:        Header file for the resouce identifiers

    MainWnd.cpp:       Implements the main frame window
    MainWnd.h:         Header file for MainWnd.cpp

    BitmapWnd.cpp:     Implements an image display window
    BitmapWnd.h:       Header file for BitmapWnd.cpp

    StdAfx.cpp:        Precompiled headers file
    StdAfx.h:          Includes frequently used standard system files


Notes
-----

- It is recommended that applications make device and image selection 
  available through a menu item named "From Scanner or Camera..." on the 
  "File" menu. This item could be grayed if there are no WIA devices on the
  system. 

- It is recommended that the application developers test their applications 
  with serial and USB cameras, and flatbed, scroll-fed and ADF scanners. 
  Also, software only device emulators available on the Windows DDK can be 
  used for testing. 

- Scroll fed scanners generally do not know the image height when the 
  data transfer starts, so they may return 0 for the image size and for the
  image height in the bitmap header. In this case, the callback function 
  should be able to expand its buffer when new data arrives and should 
  explicitly calculate the height when the transfer completes.

- When using the DeviceDlg API on an automatic document feeder (ADF), 
  the API will set the ADF to scan one page. If multiple pages are desired, 
  the application should explicity set the WIA_DPS_PAGES to the number
  of pages it requests or to the ALL_PAGES value.

- SelectDeviceDlg, DeviceDlg, idtGetData and idtGetBandedData API's
  return S_FALSE if the user cancels, so it is not suffient to check the
  return value with the SUCCEEDED() macro, the programmer must explicity 
  check S_FALSE. Similary, ReadMultiple and WriteMultiple API's return 
  S_FALSE whenthe function arguments are correct, but the function cannot 
  perform the requested operation, so the programmer must explicity 
  check the return value against S_FALSE.

- If the application wants to display a cancellable progress dialog,
  it should display the dialog in separate thread. The data trasfer thread
  will be blocked until the transfer is complete, so it will not be able to 
  process the window messages as soon as they arrive. So if the the progress 
  dialog is created in the same thread, the dialog (and the cancel button) 
  will be very sluggish to user input.

- The data trasfer can be cancelled only when BandedDataCallback returns and
  since BandedDataCallback may be called every few seconds, the application 
  should indicate the user that the cancel operation is in progress, for 
  example it can disable the cancel button, or display a wait message.

========================================================================
   SAMPLE : Scan Dialog
========================================================================

Description:
----------------

The sample demonstrates the ScanDialog UI (including the select device UI)
which can be used to preview/acquire images. It also allows change of
item properties. Basically once IWiaDevMgr2->GetImageUI() is called, the
control is handed over to WIA and the scan dialog can be used by the user to
perform various operations on items.   

Details:
--------

The sample creates a WIA Device Manager and then calls the API
IWiaDevMgr2->GetImageUI() to display SelectDevice UI and ScanDialog UI. 

Note: All the files/images will be downloaded to the Temp directory (returned
by API GetTempPath() ), i.e. directory represented by %TEMP%. 

The filename given as argument to IWiaDevMgr2->GetImageUI() is "ScanDialogFile".
Hence this is the file that will contain the image downloaded/previewed from
Scan Dialog. 


PreCondition:
-------------------
This sample is supported for Windows Vista.
For this sample to produce some result, atleast one WIA device should be installed on the system.



 




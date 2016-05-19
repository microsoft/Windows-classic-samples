===============================================================================================================
   SAMPLE : ErrorHandler
================================================================================================================

Description:
----------------
The sample demonstrates the error handling mechanism introduced in Windows
Vista , which can be used by applications to gracefully handle and possibly
recover from errors and delays during data transfer, like paper empty problem,
scanner cover open etc.

Details:
--------
The sample creates a WIA Device Manager, enumerates various devices and then creates a device, thereby getting an interface pointer to 
root item of a Wia device. After that the sample iterates the item tree and
downloads items. Now any problem encountered during download can be handled by
the interface IWiaAppErrorHandler implemented by the class callback.
IWiaAppErrorHandler contains a function called ReportStatus() which can be
used to seek user intervention for various errors as desired by the
application.

We are handling the following device status reported by the driver:
1) WIA_STATUS_WARMING_UP
2) WIA_ERROR_WARMING_UP
3) WIA_ERROR_COVER_OPEN
4) WIA_ERROR_PAPER_EMPTY


PreCondition:
-------------------
This sample is supported for Windows Vista.
For this sample to produce some result, atleast one WIA device should be installed on the system.



 




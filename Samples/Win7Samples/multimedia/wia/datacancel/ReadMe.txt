===============================================================================================================
   SAMPLE : Data Cancel
================================================================================================================

Description:
----------------
The sample demonstrates data transfer cancellation on a WIA item.

There are 2 ways to cancel data transfer introduced in Windows Vista:

1) Call IWiaTransfer::cancel() on a WIA item being transferred.
2) Return S_FALSE from TransferCallback() function implemented by Application
callback.

Details:
--------

The sample creates a WIA Device Manager, enumerates various devices and then creates a device, thereby getting an interface pointer to 
root item of a Wia device. After that the sample iterates the item tree and then calls data transfer functions on 
transferrable file items.
When WIA service calls the TransferCallback() function of the Application
callback to notify the application of the transfer progress, the sample calls IWiaTransfer::cancel()
function from TransferCallback() to cancel the transfer of data from that item. The application callback stores a pointer to
IWiaTransfer interface as a private data member. This pointer is passed while
initializing the callback.

Another method to cancel data without calling IWiaTransfer::cancel() is to
return S_FALSE from TransferCallback() function of the callback object of
application.

Note: All the files/images will be downloaded to the Temp directory (returned
by API GetTempPath() ), i.e. directory represented by %TEMP%.
The images will be of lesser size than normal since the sample cancels the
data transfer.


PreCondition:
-------------------
This sample is supported for Windows Vista.
For this sample to produce some result, atleast one WIA device should be installed on the system.



 




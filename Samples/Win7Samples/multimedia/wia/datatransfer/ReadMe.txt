========================================================================
   SAMPLE : Data Transfer
========================================================================

Description:
----------------
Use the new IWiaTransfer interface exposed by WIA. 

The sample makes use of the new functionalities supported by IWiaTransfer interface like ability to tranfer multiple pages with a 
single transfer call, abilty to do multi-item transfers with 1 API call and uploading data to a device .

Details:
--------

The sample creates a WIA Device Manager, enumerates various devices and then creates a device, thereby getting an interface pointer to 
root item of a Wia device. After that the sample iterates the item tree and then calls data transfer functions on those items(files,folders) 
depending on the item type.

For items marked with WiaItemTypeTransfer flag , the sample calls function to download item  data. For Storage folder, it uploads 
Upload.BMP (present in the DataTransfer directory) to the device storage.


Mainly the sample demonstrates the following 3 features:

1. The sample uses API IWiaTransfer::upload( ) on a WIA item to transfer item data to device storage. 
2. The sample uses API IwiaTransfer::download( ) on a folder item to transfer data from multiple items inside a folder.
3. The sample does a multi-page transfer(feeder) of data in BMP format using a single transfer call using the flag WIA_TRANSFER_ACQUIRE_CHILDREN.

Note: All the files/images will be downloaded to the Temp directory (returned
by API GetTempPath() ), i.e. directory represented by %TEMP%.


PreCondition:
-------------------
This sample is supported for Windows Vista.
For this sample to produce some result, atleast one WIA device should be installed on the system.

The path from where the sample is run should contain Upload.BMP file. 
 




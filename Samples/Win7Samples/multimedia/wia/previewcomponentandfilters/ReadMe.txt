========================================================================
   SAMPLE : PreviewComponentAndFilters
========================================================================

Description:
----------------
Use the Preview Component exposed by WIA and invoke segmentation/image processing filters through it. 

In Vista, a new component WIA Preview Component has been introduced which serves two primary purposes: 

1) Enables accurate live previews by calling into the driver's image processing filter. "Live" means that an application won't have
   to re-acqiure the image from the scanner once it changes a few property settings.

2) Enables an application to take advantage of a driver's segmentation filter to performm multi-region scanning.

The WIA Preview Component is provided by Microsoft whereas the components that it calls namely, Image processing filter and segmentation filter,
are typically provided by hardware vendors.

Details:
--------

The sample creates a WIA Device Manager, enumerates various devices and then creates a device, thereby getting an interface pointer to 
root item of a Wia device. After that the sample iterates the item tree and then calls GetPreviewComponent() to get an instance of WIA
Preview Component.


Mainly the sample demonstrates the following :

1. The sample creates an instance of WIA Preview component for all transferrable file items except those with WIA_IPA_CATEGORY property value
of WIA_CATEGORY_FINISHED_FILE. Note that previewing cannot be performed on finished files since one cannot change any property like
brightness and contrast. Also the preview component does not support folder acquisitions.

2. The sample uses API IWiaPreview::GetNewPreview() which transfers the data for the item from the scanner for the first time though the Image 
processing filter. This function is very similar to IWiaTransfer::Download(). The image transferred is internally cached by the preview component.

3. The sample uses API IWiaPreview::DetectRegions() to  invoke the driver's segmentation filter which passes the IWiaItem2 interface that
was previously passed into IWiaPreview::GetNewPreview(). Before calling this function, the sample checks whether the driver supports the 
property WIA_IPS_SEGMENTATION.

4. The sample uses API IWiaPreview::UpdatePreview() after changing the brightness and contrast of the item that was passed to 
IWiaPreview::GetNewPreview(). This function will cause the image processing filter to perform filtering on the item 
(changing brightness/contrast) and to transfer the filtered image without going to the scanner.Hence this transfer will be quite fast. 
Before calling this function, the sample checks whether the driver comes with an image processing filter by calling
IWiaItem2::CheckExtension().

Note: All the files/images will be downloaded to the Temp directory (returned
by API GetTempPath() ), i.e. directory represented by %TEMP%. The images
downloaded through IWiaPreview::UpdatePreview() will overwrite the images got
through IWiaPreview::GetNewPreview().
 

PreCondition:
-------------------
This sample is supported for Windows Vista.
For this sample to produce some result, atleast one WIA device should be installed on the system.




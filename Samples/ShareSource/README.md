# WPF Sharing content source app sample

Shows how a managed Win32 app can participate in the [Share contract](https://docs.microsoft.com/en-us/windows/uwp/app-to-app/share-data).
There is a corresponding [Sharing content UWP source app sample](http://go.microsoft.com/fwlink/p/?LinkId=620596) available. 

Some of the classes you might want to review in more detail are the 
[DataTransferManager](http://msdn.microsoft.com/library/windows/apps/br205932) class, 
which you use to initiate a share operation, and the [DataPackage](http://msdn.microsoft.com/library/windows/apps/br205873) class, 
which you use to package the content. Because each share scenario usually involves two apps (the source app 
and a target app that receives the content) we recommend you install and deploy the 
[Sharing content target app sample](http://go.microsoft.com/fwlink/p/?linkid=231519) when you install and run this one. 
This way, you can see how sharing works from end to end. 

**Note** Win32 is not supported for Sharing content target apps.

This sample covers how to share content in a variety of formats, including:

-   Text
-   Web link
-   Files

**Note** This sample requires Visual Studio 2015 to build and Windows 10 to execute.
 
To obtain information about Windows 10 development, go to the [Windows Dev Center](http://go.microsoft.com/fwlink/?LinkID=532421)

To obtain information about Microsoft Visual Studio 2015 and the tools for developing Windows apps, go to [Visual Studio 2015](http://go.microsoft.com/fwlink/?LinkID=532422)

## Related topics

### Reference

To learn more about sharing and the [Windows.ApplicationModel.DataTransfer](http://msdn.microsoft.com/library/windows/apps/br205967) namespace, we recommend you take a look at the [Sharing and exchanging data](http://msdn.microsoft.com/library/windows/apps/hh464923) section of our documentation, which describes how sharing works and contains several how-to topics that cover how to share [text](http://msdn.microsoft.com/library/windows/apps/hh758313), an [image](http://msdn.microsoft.com/library/windows/apps/hh758305), files, and other formats. Our [Guidelines for sharing content](http://msdn.microsoft.com/library/windows/apps/hh465251) can also help you create a great user experience with the share feature.

For more info about the concepts and APIs demonstrated in this sample, see these topics:

-   [Sharing content UWP source app sample](http://go.microsoft.com/fwlink/p/?LinkId=620596)
-   [Sharing content target app sample](http://go.microsoft.com/fwlink/p/?linkid=231519)
-   [Sharing and exchanging data](http://msdn.microsoft.com/library/windows/apps/hh464923)
-   [How to share files (HTML)](http://msdn.microsoft.com/library/windows/apps/hh758308)
-   [How to share files (XAML)](http://msdn.microsoft.com/library/windows/apps/hh871371)
-   [How to share HTML (HTML)](http://msdn.microsoft.com/library/windows/apps/hh758310)
-   [How to share HTML (XAML)](http://msdn.microsoft.com/library/windows/apps/hh973055)
-   [How to share text (HTML)](http://msdn.microsoft.com/library/windows/apps/hh758313)
-   [How to share text (XAML)](http://msdn.microsoft.com/library/windows/apps/hh871372)
-   [Quickstart: Sharing content (HTML)](http://msdn.microsoft.com/library/windows/apps/hh465261)
-   [Share data](https://msdn.microsoft.com/library/windows/apps/mt243293)
-   [DataPackageView](http://msdn.microsoft.com/library/windows/apps/hh738408)
-   [ShareOperation](http://msdn.microsoft.com/library/windows/apps/br205977)
-   [Windows.ApplicationModel.DataTransfer](http://msdn.microsoft.com/library/windows/apps/br205967)
-   [Windows.ApplicationModel.DataTransfer.Share](http://msdn.microsoft.com/library/windows/apps/br205989)

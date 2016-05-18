==================================
Element Behaviors - Ad Banner
==================================
Last Updated: Aug.01,2000


SUMMARY
========
Element behaviors are a new technology in Microsoft® Internet Explorer 5.5. They 
allow Web authors to encapsulate reusable functionality inside an HTML component 
(HTC) file. A lightweight HTC--an HTC with less memory overhead that doesn't come 
with its own document--allows users to create lightweight components and reuse them 
many times on the same page.  


DETAILS
========
In the demo, the script that implements the "my:addbanner" custom tags is encapsulated 
in a separate file. The sample renders any HTML content inside of it in the top right 
position by default. By specifying values for the "bannerfromTop" and "bannerfromRight" 
properties on the behavior the banner can be made to render at a different position than 
the default position which is to the top right most corner of the page. The behaviors 
re-positions the content inside it when the page is either resized or scrolled. In order 
to allow the scrolling and resize events be notified of the behavior, attach the onresize 
and onscroll functions on the body of the HTML Page to call the behaviors relocate public 
method. In the page, additional functionality, (footerClick() function), is added to show 
and hide the floating advertisement. This is optional and can be removed to have the 
advertisement always floating. 


USAGE
======
Use of this behavior requires some basic declarations: 

 - "Xmlns" declaration inside the HTML tag. 
 - Import declaration with the appropriate namespace, as specified in the xmlns 
   declaration and the path to the Adbanner.htc file. 
 - The custom tags themselves should be included in the body of the HTML page, 
   <my:adbanner ..>Content</my:adbanner>.  
 - Required -- the onresize and onscroll event handlers on the Primary Document 
   BODY Element with the event handlers calling the Behaviors relocate public method. 
 - Optional -- bannerfromTop and bannerfromRight properties on the behavior specifying 
   the position the content inside the behavior should render. 

Syntax
------
<HTML xmlns:my>
<?import namespace=my implementation="adbanner.htc" />
:	:
<BODY onresize="behav.relocate();" onscroll="behav.relocate();">
:	:
<my:adbanner id=behav bannerfromTop=1 bannerfromRight=130>
	<!--	Content inside here.. 	-->
	<IMG SRC="myImage.gif">With some text here.
</my:adbanner>
:	:


BROWSER/PLATFORM COMPATIBILITY
===============================
This DHTML Behaviors sample is supported in Microsoft® Internet Explorer 5.5 or later 
on the Microsoft Win32® platform.  



SOURCE FILES
=============
AdBanner.htc
AdBanner.htm


SEE ALSO
=========
For more information on Element Behaviors - Lightweight HTC Files, go to:
http://msdn.microsoft.com/workshop/author/behaviors/overview/identityb_ovw.asp

For an HTC Reference, go to:
http://msdn.microsoft.com/workshop/components/htc/reference/htcref.asp

For DHTML Behaviors, go to:
http://msdn.microsoft.com/workshop/author/behaviors/overview.asp

For the DHTML Object Model, go to:
http://msdn.microsoft.com/workshop/author/default.asp


==================================
Copyright (c) Microsoft Corporation. All rights reserved.

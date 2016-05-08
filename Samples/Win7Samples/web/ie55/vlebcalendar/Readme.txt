=================================================
IE 5.5 Demos - ViewLink Element Behavior Calendar
=================================================
Last Updated: October 04, 2000


SUMMARY
========
Viewlink is a feature of element behaviors that enables you to write fully 
encapsulated DHTML behaviors and import them as robust custom elements in a 
Web page. Viewlinked document content, otherwise known as the document fragment, 
renders and behaves as regular HTML content in the main Web page. Either binary 
DHTML behaviors or scripted HTML Components (HTC) can be used to implement an 
element behavior with a viewlink, but this overview focuses primarily on the use 
of viewlink in scripted HTC files.

 
DETAILS
========
This sample uses a Calendar component implemented using two different variations: the 
first is an element behavior and the second is an element behavior with a viewlink. 
Although very similar in appearance and function, the Calendars illustrate some 
interesting differences that result from Viewlinking an element behavior.

Each component has a button to display the HTML document tree, highlighting the difference 
in primary document structure that results from each variation. In the element behavior 
version, the primary document tree structure is considerably more complex than the viewlink 
version. The primary document for the element behavior is filled with numerous elements 
branching off from the custom element, whereas the viewlink version shows how the contents 
of the document fragment have been encapsulated in the HTC file. The sample also illustrates 
the difference in Cascading Style Sheets (CSS) inheritance for each variation. A button 
applies a background style to the table elements in the primary document. When the style 
is applied, elements inside the element behavior inherit the updated style property. This 
result could be undesirable, depending on the requirements for the component. For the viewlink 
version of the Calendar, applying the style in the primary document has no effect on the 
rendering of the Calendar control.


USAGE
======
Creating a ViewLink:
--------------------
Before going through the basic steps to create a simple viewlink, it is worth taking a 
quick look at the defaults object and its properties. This object is fundamental to 
understanding how element behaviors make use of the viewlink feature. The defaults 
object has several properties that are used in association with viewlink.

The defaults object is used to set and retrieve the default properties for element 
behaviors, so it follows that viewLink is one of the properties of this object. A viewlink 
is established for an element behavior when the viewLink property is set to an object 
containing document content. The object that is assigned to the viewLink property is the 
document fragment. By default, an element behavior does not have a viewlink defined, so 
the initial value of the viewLink property is null.

The PUBLIC:DEFAULTS element is the declarative form of the defaults object. The attributes 
supported by the PUBLIC:DEFAULTS element correspond to the properties collection of the 
defaults; so either script or declarations can be used to set the defaults object properties. 
The PUBLIC:DEFAULTS element is used in the component section of an HTC file, where the 
initial properties of the defaults object can be set.

Three basic steps are required to create and implement an element behavior that uses a 
viewlink: 

 - Create an element behavior. 
 - Define a viewlink. 
 - Import the HTC file in the primary document. 

Each step is described in the following sections. 

Creating an Element Behavior:
-----------------------------
Viewlink is a feature of element behaviors; therefore, the process of writing element 
behaviors with or without a viewlink is quite similar. The process of creating an element 
behavior is covered in detail in the Element Behaviors Overview.

Defining a ViewLink:
--------------------
A viewlink can be defined using script or by inserting the appropriate declaration in 
the component section of the HTC file. 

To set up a viewlink between a primary document and a document fragment using script, 
the following statement can be used. This statement is placed within script located in 
the HTC file, ideally the script block should be placed after the PUBLIC:COMPONENT element. 

    defaults.viewLink=document;

A viewlink can also be established using a declaration in the component definition. The 
following example automatically links the root element of the document fragment in the 
HTC file to the master element in the primary document.

    <PUBLIC:DEFAULTS viewLinkContent/>

Importing the HTC file:
-----------------------
This section explains how to import an HTC file as an implementation of an element behavior 
that uses viewlink. Importing a master element into the primary document is a trivial step, 
but the master element itself deserves some commentary. The name of the master element is 
declared in the HTC file using the tagName attribute in the PUBLIC:COMPONENT element, as 
shown in the previous sample code snippet. The master element takes the namespace as its 
prefix, followed by a colon (:), and then the tag name. As many master elements as necessary 
can be included in the primary document. 

A viewlink displays the document fragment in the primary document at the location of the 
master element. The contents of the master element are not rendered in the primary document 
unless the behavior fails to initialize. If the component initialization does fail, then 
the contents of the master element are rendered instead of the document fragment. The 
contents of the master element are available in script through the innerHTML and innerText 
properties. 


BROWSER/PLATFORM COMPATIBILITY
===============================
This element behavior sample is supported in Microsoft® Internet Explorer 5.5 or 
later on the Microsoft Win32® platform.  


SOURCE FILES
=============
Vlbuildtree.htc
Vlcalendar_vl.htc
Vlcalendarnv.htc
Vlcomparison.htm
Vlelementbehavior.htm
Vlviewlink.htm


SEE ALSO
=========
For more information on the defaults Object, go to:
http://msdn.microsoft.com/workshop/author/dhtml/reference/objects/defaults.asp

For more information on the viewLink Property, go to:
http://msdn.microsoft.com/workshop/author/dhtml/reference/properties/viewLink.asp

For more information on the PUBLIC:DEFAULTS Element, go to:
http://msdn.microsoft.com/workshop/components/htc/reference/elements/defaults.asp

For more information on Element Behaviors, go to:
http://msdn.microsoft.com/workshop/author/behaviors/overview/elementb_ovw.asp

For the HTC Reference, go to:
http://msdn.microsoft.com/workshop/components/htc/reference/htcref.asp

For information on DHTML Behaviors, go to:
http://msdn.microsoft.com/workshop/author/behaviors/overview.asp

==================================
Copyright (c) Microsoft Corporation. All rights reserved.

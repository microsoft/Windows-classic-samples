===========================
Filters - Editable Regions
===========================
Last Updated: OCT. 18, 2001              


SUMMARY
========
The Editable Regions sample shows how to use the contentEditable property to control 
whether the user can edit the content of an object. 

This example demonstrates the features of Microsoft® Internet Explorer that enable 
users to edit the content of an HTML element directly from the browser. 


DETAILS
========
You can now incorporate Internet Explorer's sophisticated HTML editing functionality 
directly in your Web pages! Microsoft's HTML editor is a built-in extension to the 
HTML parsing and rendering engine in Internet Explorer. Since Internet Explorer 4.0, 
the MSHTML editor has provided an HTML editing platform for host applications 
developing HTML-based text editors and Web authoring applications. In previous 
versions, however, the editor was only enabled when a document was in "design" mode. 
With Internet Explorer 5.5, you can declare a document (or individual elements within 
a document) editable at run time, enabling Web authors to support WYSIWYG (what you 
see is what you get) HTML editing inside the browser. 

The MSHTML editor exposes a simple but powerful scripting model that supports most 
commonly-used editing features for both text and forms. This support allows you to 
develop sophisticated online editing applications, to create unique forms that allow 
users to enter formatted text and images, or to create a page on a Web site that 
each user can customize with their own content. A Web author might use the text 
editing features to develop an online text editor, or the forms editing features 
to write an online greeting card application.


USAGE
======
There are two ways to activate the editor. One option puts the whole document in 
design mode; the other option can be used in browse mode to make individual elements 
editable at run time. If you want to make the whole document editable at browse time, 
set the contentEditable attribute on the document's body. 

Putting a Document in Design Mode
---------------------------------
To put an entire document in design mode, set the designMode property on the document 
object itself. When a document is in design mode, scripts don't run. While it may 
seem like a good idea to set a button to toggle design mode on and off from within 
a document, it won't work. After a user clicks the button to turn on design mode, 
the document will remain in design mode. The next time the user clicks the button, 
it will be selected rather than activated; when the user clicks the button again, 
its value will be editable. If you plan to use design mode, it's best to set the 
designMode property on a document that's inside a frame or an IFrame. The 
following example shows how to turn on design mode for a document in an IFrame: 

  <script for="btnDesign" event="onclick">
    targetDoc = document.frames(0).document;
    if (targetDoc.designMode == "On")
      targetDoc.designMode = "Off";
    else
      targetDoc.designMode = "On";
  </script>
  <button id=btnDesign>DesignMode</button>
  <iframe src="blank.htm" style="border: black thin; width:100%; height:200px">
  </iframe>

The value of the designMode property is always stored with initial capitalization, 
even if it was originally set using all lowercase text. Be sure to remember this 
when testing its value. The default value of the designMode property is "Inherit." 

Making Elements Editable in Browse Mode (or Noneditable in Design Mode)
-----------------------------------------------------------------------
In Internet Explorer 5.5, it's possible to activate the editor on a per-element basis 
while the document remains in browse mode. You can make an element editable at browse 
time by setting its contentEditable attribute to True. The following example
shows how to set this attribute declaratively, creating a SPAN element that behaves 
like a text box: 

  <span contentEditable=true style="width:150; border:lightgrey 3px inset"></span>

Making an element editable through scripting is just as easy. The following example 
shows how to set the contentEditable attribute from script on an element whose 
identifier is "orangesicle":

  orangesicle.contentEditable=True;

To prevent an element from being editable in design mode, set the contentEditable 
attribute to False. This will cause the element to have layout, however, so when 
it's in design mode, the user will be able to control-select it, drag it, resize 
it, etc.: 

  orangesicle.contentEditable=False;

Caution: For purposes of demonstration, the Editable Regions sample renders user input from a TEXTAREA element as HTML in another DIV element. If you employ the contentEditable property in your own application to display text as HTML, take precautions against trusting user input.  Inappropriate handling of user input can compromise the security of your application.

BROWSER/PLATFORM COMPATIBILITY
===============================
This Filters sample is supported in Internet Explorer 5.5 or later on the 
Microsoft Win32® platform. 


SOURCE FILES
=============
EditRegions.htm


SEE ALSO
=========
For Creating Editable Web Pages in Internet Explorer 5.5, go to:
http://msdn.microsoft.com/workshop/browser/createwp.asp

For more information on Filters and Transitions, go to:
http://msdn.microsoft.com/workshop/author/filter/filters.asp

For Visual Filters and Transitions Reference, go to:
http://msdn.microsoft.com/workshop/author/filter/reference/reference.asp

For more information on the designMode Property, go to:
http://msdn.microsoft.com/workshop/Author/dhtml/reference/properties/designMode.asp

For more information on CONTENTEDITABLE Attribute | contentEditable Property, go to:
http://msdn.microsoft.com/workshop/author/dhtml/reference/properties/contentEditable.asp

==================================
© Microsoft Corporation  

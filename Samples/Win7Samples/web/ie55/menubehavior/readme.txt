==================================
DHTML Behaviors - Menu Behavior
==================================
Last Updated: OCT. 18,  2001              


SUMMARY
========
Element behaviors are a new technology in Microsoft® Internet Explorer 5.5. They 
allow Web authors to encapsulate reusable functionality inside an HTML component 
(HTC) file. A lightweight HTC is an HTC file with less memory overhead that doesn't come 
with its own document. It allows you to create lightweight components and reuse them 
many times on the same page.  

Pop-up behaviors, another new technology introduced in Internet Explorer 5.5, allow 
Web authors to create menus that "pop-up" over other HTML elements on the page. 
Pop-up menus can also extend outside the browser window onto the desktop. 


DETAILS
========
In the following demo, the script that implements the "myMenu:menu" custom tags is
encapsulated in a separate file. Each top-level menu item has an event associated
with it: onsubmenu_click. The event fires when the user clicks a top menu item's
child. This event in turn calls its associated event handler "doFunction()" within
the primary document containing the result. Any event handler name that you supply
is acceptable. It is not necessary to use the same name as in this example.

All custom tags also have an ID attribute specified, which is essential when you 
are programming with this behavior. The ID attribute is used to specify what menu item is clicked
by the user when the event handler is called.


USAGE
======
To use Menu Behavior (IE 5.5), complete the following steps.  

 1. Include an "XMLNS" declaration inside the HTML tag.

 2. Include the "IMPORT" declaration with the appropriate namespace, as specified
    in the XMLNS declaration and the path to the menu.htc file.

 3. Include the custom tags in the BODY of the HTML page. The top-level menu
    item is the first custom element: its children are nested inside the main
    element. Be sure to include the onsubmenu_click event on the top-level
    menu items to handle user interaction and an ID for every menu item. Also,
    specify the event handler to call when this event is fired within the primary
    document.

 4. Define the event handler within the SCRIPT tags that you specified for the
    onsubmenu_click event in Step 3. Use the event.result syntax
    within this event handler to retrieve the ID of the menu item that was clicked.


BROWSER/PLATFORM COMPATIBILITY
===============================
This DHTML Behaviors sample is supported in Microsoft® Internet Explorer 5.5 or later 
on the Microsoft Win32® platform.  

Syntax
------
Here is a short snippet combining steps 1 thru 4. 

<HTML XMLNS:myMenu>
<IMPORT namespace="myMenu" implementation="menu.htc"/>
…
<SCRIPT>
function doFunction(){
	switch(event.result){
		case "oChild1": // Do something.
			break;
		case "oChild2": // Do something.
			break;
		default: // Do something.
			break;
	}
}
</SCRIPT>
…
<BODY>
<myMenu:menu onsubmenu_click="doFunction()" id="oParent1">Home
<myMenu:menu id="oChild1">Child 1</myMenu:menu>
<myMenu:menu id="oChild2">Child 2</myMenu:menu>
</myMenu:menu>
</BODY>
</HTML>


SOURCE FILES
=============
menu.htc
menu.htm


SEE ALSO
=========
For more information on Element Behaviors, see:
http://msdn.microsoft.com/workshop/author/behaviors/overview/identityb_ovw.asp

For more information on createPopUp Method, see:
http://msdn.microsoft.com/workshop/author/dhtml/reference/methods/createpopup.asp

For an HTC Reference, see:
http://msdn.microsoft.com/workshop/components/htc/reference/htcref.asp

For DHTML Behaviors, see:
http://msdn.microsoft.com/workshop/author/behaviors/overview.asp

For the DHTML Object Model, see:
http://msdn.microsoft.com/workshop/author/default.asp


==================================
© Microsoft Corporation 

===================================
IE 5.5 Demos - HTML+Time Animations
===================================
Last Updated: October 03, 2000


SUMMARY
========
HTML+TIME (Timed Interactive Multimedia Extensions), first released in Microsoft® 
Internet Explorer 5, adds timing and media synchronization support to HTML pages. 
Using a few eXtensible Markup Language (XML)-based elements and attributes, you can 
add images, video, and sounds to an HTML page and synchronize them with HTML text 
elements over a specified amount of time. In short, you can use HTML+TIME technology 
to quickly create multimedia-rich, interactive presentations, easily and with little 
or no scripting.  


DETAILS
========
These animations use hidden HTML elements. Over time, the display property is set to 
block using the t:SET element. As each element is displayed, different attributes of 
the element are animated using the t:ANIMATE element. Changing the left and top CSS
attributes animate the element positions. On the top animation, the background 
cycles through multiple colors using the t:ANIMATECOLOR element.

You can create various effects by animating filters that are applied to elements. For 
example, to create a fading effect, apply the alpha filter and animate the filter's 
opacity property.


USAGE
======
This behavior requires some basic declarations: 

 - An "xmlns" declaration inside the HTML tag. 
 - An import declaration with the appropriate namespace as specified in the "xmlns" 
   declaration.

Syntax:

The following code shows a basic HTML+TIME document structure with the "xmlns" 
declaration and the import declaration. 

  <HTML xmlns:t ="urn:schemas-microsoft-com:time">
  <?IMPORT namespace="t" implementation="#default#time2">   
  <BODY>
  </BODY>  
  </HTML>  

The following code demonstrates how to animate a color change on an element. 

  <t:animatecolor  
	targetElement="aColorElement" 
	attributeName="backgroundColor" 
	values="blue;green;red;yellow" 
	begin="0" 
	dur="10" 
	autoreverse="true"/>

The following code demonstrates how to animate a filter on an element. 

  <t:animate  
	targetElement="aFilterElement" 
	attributeName="filters.item(0).opacity" 
	from="100" 
	to="0" 
	begin="0" 
	dur="1"/>


BROWSER/PLATFORM COMPATIBILITY
===============================
This HTML+TIME sample is supported in Microsoft® Internet Explorer 5.5 or 
later on the Microsoft Win32® platform. 

HTML+TIME 1.0 is a submission to the World Wide Web Consortium (W3C). 
HTML+TIME 2.0 supersedes HTML+TIME 1 in Microsoft® Internet Explorer 5.5. 
HTML+TIME 2.0 is based on the HTML+SMIL language profile  in the SMIL-Boston 
specification. SMIL Boston is the W3C successor to SMIL 1.0  


SOURCE FILES
=============
Animations.htm
Animation1.htm
Animation2.htm

GRAPHICS FILES
===============
Bgclockgray.gif
Blacke.gif
Dot.gif
H.gif
IETexter.gif
L.gif
M.gif
Plus.gif
T.gif


SEE ALSO
=========
For more information on HTML+Time, go to:
http://msdn.microsoft.com/workshop/author/behaviors/time.asp

For information on the HTML+Time 1.0 submission to the World Wide Web Consortium (W3), go to:
http://www.w3.org/Submission/1998/14/

For information on the HTML+SMIL language profile, go to:
http://www.w3.org/TR/2000/WD-smil-boston-20000622/html-smil-profile.html

==================================
Copyright (c) Microsoft Corporation. All rights reserved.

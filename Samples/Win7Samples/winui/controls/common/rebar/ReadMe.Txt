Rebar Control Sample "Rebar"

Rebar is a simple example of implementing a rebar common control in an application. 

This example creates a rebar control that has two bands. One contains a ComboBox and the other contains a Pushbutton.

The rebar control can be placed on any of the four sides of the parent window by selecting the desired side from the "View" menu. When the side is changed, the existing rebar control is destroyed and another one is created. The reason for this is that the control statically stores some styles at creation time and they cannot be changed "on the fly".

To close the application, simply select "Exit" from the "File" menu or click the close button on the caption bar.

Virtual ListView Control Sample "VListVw"

VListVw is a simple example of implementing a virtual ListView common control in an application. A virtual ListView is not a separate control, but a standard ListView with the LVS_OWNERDATA style. This example creates a ListView control that has the LVS_OWNERDATA style and "virtually" holds 100,000 items. The items are never actually added. Instead, the virtual ListView control is "told" how many items it contains with the LVM_SETITEMCOUNT message. When an item needs to be drawn, the ListView control queries the parent window for display information with the LVN_GETDISPINFO notification.

The ListView control can be placed in any of the four different view modes by selecting the desired view from the "View" menu. When the view is changed, the sample changes the style "on the fly" using the SetWindowLong API.

To close the application, simply select "Exit" from the "File" menu or click the close button on the caption bar.

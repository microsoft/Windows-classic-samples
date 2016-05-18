Ink Zoom
--------

This sample program demonstrates how to zoom and scroll ink. In particular, it enables the user to zoom in and out of ink in increments. It also demonstrates how to zoom into a particular region using a zoom rectangle. Finally, this sample illustrates how to collect ink at different zoom ratios and how to set up scrolling within the zoomed drawing area.

In the sample, the Renderer object's view and object transforms are used to perform zooming and scrolling. The view transform applies to the points and the pen width. The object transform applies only to the points. The user can control which transform is used by changing the Scale Pen Width item on the Mode menu.

The following features are used in this sample:

* The InkCollector object
* The Renderer object's SetViewTransform method
* The Renderer object's SetObjectTransform method



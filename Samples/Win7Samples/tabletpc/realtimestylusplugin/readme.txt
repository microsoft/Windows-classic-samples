RealTimeStylus Plug-in Sample
-----------------------------

This application demonstrates working with the RealTimeStylus class. For a detailed overview of the StylusInput APIs, including the RealTimeStylus class, see Accessing and Manipulating Stylus Input. For information about synchronous and asynchronous plug-ins, see Plug-ins and the RealTimeStylus Class.

Plug-ins, objects that implement the IStylusSyncPlugin or IStylusAsyncPlugin interface can be added to a RealTimeStylus object. This sample application uses several types of plug-in: 

Packet Filter Plug-in: Modifies packets. The packet filter plug-in in this sample modifies packet information by constraining all (x,y) packet data within a rectangular area.

Custom Dynamic Renderer Plug-in: Modifies dynamic rendering qualities. The custom dynamic rendering plug-in in this sample modifies the way ink is rendered by drawing a small circle around each (x,y) point on a stroke.

Dynamic Renderer Plug-in: Modifies dynamic rendering qualities. This sample demonstrates use of the DynamicRenderer object provided by the Microsoft® Windows® XP Tablet PC Edition Software Development Kit (SDK) Platform application programming interface (API) as a plug-in to handle dynamic rendering of ink.

Gesture Recognizer Plug-in: Recognizes application gesture. This sample demonstrates use of the GestureRecognizer object provided by the Tablet PC SDK Platform API as a plug-in to recognize application gestures (when running on a system with the Microsoft gesture recognizer present).

In addition, this sample provides a user interface that enables the user to add, remove, and change the order of each plug-in in the collection. The sample solution contains two projects, RealTimeStylusPluginApp and RealTimeStylusPlugins. RealTimeStylusPluginApp contains the user interface for the sample. RealTimeStylusPlugins contains the implementations of the plug-ins. The RealTimeStylusPlugins project defines the RealTimeStylusPlugins namespace, which contains the packet filter and custom dynamic renderer plug-ins. 

This namespace is referenced by the RealTimeStylusPluginApp project. The RealTimeStylusPlugins project uses the Microsoft.Ink, Microsoft.StylusInput, and Microsoft.StylusInput.PluginData namespaces.


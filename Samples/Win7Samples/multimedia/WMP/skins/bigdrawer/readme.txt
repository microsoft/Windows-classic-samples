Bigdrawer Skin Sample

This sample might require certain digital media files to be added to your library. To do this, run the file named loader.hta, which is located in the Windows Media Player SDK sample media folder, and follow the directions.

The following tasks are performed by this skin:

* Play a known audio file (laure.wma).
* Play a known video file (seattle.wmv).
* Displaying a visualization when media is played.
* Stop the Player.
* Close the Player.
* Open and close an auxiliary drawer.
* Provide a circular volume control.
* Provide a seek bar to control the position of the media.
* Display the status of the Player.
* Show video in a hidden drawer.
* Provide buttons in a hidden drawer to change visualizations.

The user interface for this application is divided into two parts: the main view, which provides the most common tasks needed by a player, and a hidden drawer, which displays video and provides buttons to change the visualization.

The code for this application is in two views: the main sub view (called “mainview”) contains the code for the primary user interface; the code for the drawer is contained in a sub view called “drawer”. The main view uses text buttons for the navigation (play audio, play video, stop, toggle drawer display, and exiting the application) as well as a seekslider for the seek bar and a custom slider for the round volume control. A narrow button along the left edge is also provided to open and close the hidden drawer.

The drawer is hidden when the skin starts up, but will be displayed when the Jscript function moveDrawer is called; the function is in the .js file with the same name as the application and is loaded when the skin is loaded. The isOpen variable determines whether the drawer will be opened or not. The drawer has a lower zIndex than the main part of the skin and therefore will be hidden; but when the drawer is moved to the left end of the skin, it is displayed. The drawer is opened when the Video text control is clicked, is opened or closed by the Drawer text control, and is also toggled by the long narrow button on the left edge of the skin. The drawer is initially shut when the audio file plays.

The seekslider control defines a left-right slider that allows the user to slide the slider to a new position. When the mouse clicks on the slider thumb position, the onmousedown event is triggered, which in turn changes the value of the slider. When the slider value changes, the onchange event changes the pointer to the position of the media and the media begins playing at that point.

The volume control uses a custom slider. One file (dial.bmp) contains pictures for 13 different views of a knob; the pictures are laid out in a horizontal row and are the same size. Another file (dialmap.bmp) uses a grayscale image to define what section of dial.bmp will be displayed when the knob is clicked on by the user. If the user clicks on the portion of the knob that corresponds to the darkest part of the grayscale mapping image, the leftmost picture will be displayed; if the click corresponds to the lightest part, the rightmost image will be displayed.

A visualization will play whenever any audio is playing. The visualization preset can be changed by opening the drawer and clicking one of the two buttons at the bottom of the drawer.

When the drawer is open and a video is playing, the video picture is displayed. When the drawer is open but there is no video playing, a text box will display the message “No video” but this message will be covered by any playing video. 

The following samples are replaced by this sample:

* Drawer
* Openmed
* Progress
* Slider
* Text
* Video
* Vizskin

See the Windows Media Player SDK for more information.

Copyright (c) Microsoft Corporation. All rights reserved.

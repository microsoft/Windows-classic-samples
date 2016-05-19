Multiple Language Sample Code

This sample might require certain digital media files to be added to your library. To do this, run the file named loader.hta, which is located in the Windows Media Player SDK sample media folder, and follow the directions.

This sample demonstrates how to use the multiple language track feature of the Windows Media format with the Windows Media Player ActiveX control.

Preparation of the Sample Media

The media for this sample is contained in the multilang.wma file. This file must be added to the library of the Player for this sample to work. Each language has a LCID (locale ID) that is used by Windows and the Player to identify spoken languages. The file contains 5 separate audio tracks, one for each of the following languages:

Language	Region		LCID

English		United States	1033
Portuguese	Brazil		1046
Japanese	Japan		1041
German		Germany		1031
French		France		1036

The following procedure was used to create the media:

1. Five audio files were prepared. Each file had different music as well as a different person speaking a short phrase in the language for that track. Initially the files were prepared as Windows wave files.

2. The files were then processed by Windows Media 9 Series Encoder. Each file was converted to the Windows Media format and given the extension .wma.  Using the Compression tab of the Properties dialog, the Edit button provides a dialog that lets you choose the compression and language. The compression chosen for each was CD Audio with a constant bit rate; the target bit rate was approximately 70 Kbps. When choosing compression rates, you must make the rate the same for each file that you will use to create the multi-track file. When the language and bit rate were chosen, clicking the Start Encoding button created a new file with the language ID embedded in it.

3. When all five files were encoded, the Windows Media 9 Series Streaming Editor was used to combined the five files into one new file. When combining the files, be sure to click only the Audio portion of each file; do not click the Audience portion of the tree. The output should contain only one Audience with one Audio and five streams attached to the Audio. You must first click the Create File button, then choose the new filename, and finally click the Start button to create the combined file.

4. Next the Windows Media 9 Series File Editor was used to add in the correct attributes for the new file (Title, Author, and so on). Once the attributes were inserted, the new file was ready for play. If you play the file in the Player, you can switch language tracks by choosing Audio and Language Tracks from the Play menu and then selecting the language you wish to hear. You can also choose a default language from the same menu selection.

Programming with Multiple Language Tracks

The Player SDK has a sample Web page called multilang.htm that demonstrates how to use the multiple language track technology with the Player in a web page (skins can also use the same technology and the same methods, properties, and events). This example uses Microsoft JScript, but any scripting language could be used that is compatible with Windows Script Hosting.

The user interface for this sample uses two buttons (Play and Stop) to control the Player, a window to display visualizations, two text boxes to display information about the number of tracks available and the name and LCID of the language that is currently playing, and a drop-down selection box to let the user choose a new language track.

The first part of the code creates the user interface with simple HTML elements and embeds the Player. The Player is embedded in an OBJECT element; the uiMode PARAM is set to "none" so that only the video window will display, and the autoStart PARAM is set to true so that the Player will begin playing as soon as a file is added to the current playlist. Two INPUT buttons are used to call functions that will start and stop the Player. Two text INPUT controls are added to display the number of language tracks available and the name and LCID of the currently playing track; they are given the readonly attribute so that the user cannot change the value of the text. Finally, a SELECT element is added that will allow the user to choose a new language track. Initially the SELECT element will have no items in it; they will be added dynamically later in the code.

This example contains three different SCRIPT elements: the first is a general SCRIPT element that contains code that will be loaded when the page is initialized; the other two SCRIPT elements are event handlers that handle the playStateChange and audioLanguageCHange events.

The first SCRIPT element contains four functions. The first function is called when the Play button is clicked; it gets the media item called "multilang" from the media collection and puts it in the current playlist, which starts the media playing. The second function is called when the Stop button is clicked; it stops the Player using the stop method. The third function is called when a new language is selected from the SELECT element; it gets the value (a LCID) of the selected item in the SELECT object, and assigns that LCID to the currentAudioLanguage property, which immediately changes the track that is playing to the desired new language track. The fourth function updates the display of the currently playing track name and LCID; the currentAudioLanguage property and the getLanguageName are used to generate the value of the text box.

The second SCRIPT element is called whenever a playStateChange event happens in the Player. This handler is used to determine what and how many language tracks are present in the currently playing media. Only when the Player is in the play state of 3 (Playing) can the correct information be determined about language tracks. This information only needs to be gathered once. To make sure that the block of code is only called once, the expression evaluated in the if statement will only be true if the play state is 3 and the text box used to display the number of languages has not been written to yet. First the number of languages is obtained with the audioLanguageCount property and assigned to the value of the countText INPUT box.

Next the OPTION element is filled with items, one for each language track. Each OPTION item will have a text that will display the LCID and the name of the language; each item will also contain a value, which is the LCID alone. This value is used by the NewLangSelected function to switch to a new language track. Dynamic HTML is used to add items to the OPTION element with a for loop with a count of the number of languages: first a new OPTION element is created with the createElement method; next the ID for that language is obtained from the getAudioLanguage method using the index of the language (note that the LCID index is not zero-based) and the name is obtained from the getLanguageName method using the LCID; next the name and LCID are used to create the text for the new option; in addition, the LCID is also assigned to the value of the new option (option items have a text, which is displayed, and a value, which can be used to assign a value in an expression); finally the OPTION elements selected attribute is set to zero with the selectedIndex property and the function NewLangSelected is called (this function is called whenever the user makes a new selection, firing the onChange event of the OPTION element, but this function needs to be called here because the event only fires when the user clicks on the OPTION element), changing the display of the OPTION box to the first item and also making the corresponding track the current track. After the OPTION element is filled and initialized and the current track matches the selected option, the ChangeDisplay function is called to display the updated current track. 

The third SCRIPT element is called whenever an audio language is changed. When the user clicks the OPTION box and selects a new option, the audioLanguageChange event is fired. All this handler script does is call the ChangeDisplay function so that the display of the currently playing track will updated. 

See the Windows Media Player SDK for more information about programming with multiple language tracks. 

Copyright (c) Microsoft Corporation. All rights reserved.

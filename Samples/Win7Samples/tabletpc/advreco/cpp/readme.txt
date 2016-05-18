Advanced Recognition Sample
---------------------------

The advanced recognition sample demonstrates advanced features of the Microsoft Tablet PC Automation application programming interface (API) used for handwriting recognition.

It includes the following features:

* Enumerating the installed recognizer
* Creating a recognizer context with a specific language
* Using the recognizer object
* Setting recognition factoid and wordlists
* Using guides to improve the recognition quality
* Dynamic background recognition
* Gesture recognition

The interfaces used are: IInkRecognizers, IInkRecognizer, IInkRecoContext, IInkRecognitionResult, IInkRecognitionGuide, IInkWordList, IInkGesture, IInkCollector, IInkDisp, IInkRenderer, IInkDrawingAttributes, IInkStrokes, and IInkStroke.

Requirements
One or more handwriting recognizers must be installed on the system. Appropriate Asian fonts need to be installed to output the results of the Asian recognizers.

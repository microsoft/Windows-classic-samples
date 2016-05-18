Description of multiple recognizers sample for the Tablet PC.
Multiple Recognizers Sample
---------------------------

This sample demonstrates advanced features of the Microsoft® Tablet PC Automation application programming interface (API) used for handwriting recognition.

It includes the following:

* Enumerating the installed recognizers
* Creating a recognizer context with a specific language recognizer
* Serializing recognition results with a stroke collection
* Organizing stroke collections into a custom collection within the InkDisp object
* Serializing ink objects to and retrieving them from an ink serialized format (ISF) file
* Setting recognizer input guides
* Using synchronous and asynchronous recognition

The interfaces used are: IInkRecognizers, IInkRecognizer, IInkRecognizerContext, IInkRecognitionResult, IInkRecognizerGuide, IInkCollector, IInkDisp, IInkRenderer, IInkDrawingAttributes, IInkCustomStrokes, IInkStrokes, and IInkStroke.

Requirements:

One or more handwriting recognizers must be installed on the system. Appropriate Asian fonts need to be installed to output the results of the Asian recognizers.

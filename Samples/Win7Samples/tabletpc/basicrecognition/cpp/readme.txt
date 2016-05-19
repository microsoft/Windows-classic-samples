Basic Recognition Sample
------------------------

This application demonstrates how you can build a simple handwriting recognition application.

This program creates an InkCollector object to ink-enable the window and a default recognizer context object. Upon receiving the "Recognize!" command, fired from the application's menu, the collected ink strokes are passed to the recognizer context. The best result string is presented in a message box.

The interfaces used are: IInkCollector, IInkDisp, IInkDrawingAttributes, IInkStrokes, IInkRecognizerContext, and IInkRecognitionResult.


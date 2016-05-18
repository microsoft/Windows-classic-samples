Auto Claims Form Sample
-----------------------

The Auto Claims sample addresses a hypothetical scenario for an insurance assessor. The assessor's work requires him or her to visit with clients at their home or business and to enter their claim information into a form. To increase the assessor's productivity, his IT department develops a tablet application that enables him or her to quickly and accurately enter claim information through two ink controls: InkEdit and InkPicture controls.

In this sample, a InkEdit control is used for each text input field. A user enters the relevant information about an insurance policy and vehicle into these fields with a pen. The InkPicture control is used to add ink over an automobile image to highlight damaged areas of the automobile. The Auto Claims sample is available for C# and Microsoft® Visual Basic® .NET. 

The AutoClaims Class is defined as a subclass of System.Windows.Forms.Form, and a nested class is defined for creating and managing layers of ink for different types of damage. Four event handlers are defined to perform the following tasks:

* Initializing the form and ink layers.
* Redrawing the InkPicture control.
* Selecting an ink layer through the list box.
* Changing the visibility of an ink layer.

This sample demonstrates how to use the Property System APIs to read and write values to and from files. You can try this sample for reading/writing files on file types including .jpg, .tiff, .doc, .mp3, .wma files. (and more if they have property handlers associated with them.) You can also use this sample in conjunction with the Recipe Property Handler sample which shows a sample property handler for .recipe file to debug that handler.

Usage: propertyedit [OPTIONS] [Filename]

Options:
 -get <PropertyName>   Get the value for the property defined
                       by its Canonical Name in <propertyName>
 -set <PropertyName>   Set the value for the property defined
      <PropertyValue>    by <PropertyName> with value <PropertyValue>
 -enum                 Enumerate all the properties.
 -info <PropertyName>  Get schema information on property.

Examples:
PropertyEdit -get "System.Author" foo.jpg
PropertyEdit -set "System.Author" "John Doe" foo.jpg
PropertyEdit -enum foo.jpg
PropertyEdit -info "System.Author"

A more complete property system tool, prop.exe can be found on http://www.codeplex.com/prop


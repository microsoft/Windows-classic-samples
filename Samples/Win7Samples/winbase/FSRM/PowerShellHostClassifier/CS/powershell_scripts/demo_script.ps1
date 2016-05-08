# this is an example powershell script that will classify string/multistring properties
# and it will set these properties to the filename without the .txt
#
# foocat.txt would be set to foocat

process{
	$propertyBag = $_

	#get the file name
	$fileName = $propertyBag.Name


	if($fileName -like "*.txt")
	{
		#set the property to the name without .txt
		$fileName.Substring(0, $fileName.Length-4 )
	}
}
# this is an example powershell script that can be used as in the yes no configuration - see readme about yes no
#
# this script simply states yes if the file is a .txt file


process{
	$propertyBag = $_

	#get the file name
	$fileName = $propertyBag.Name


	if($fileName -like "*.txt")
	{
		$true
	}
}
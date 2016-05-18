#this script will search the contents of txt files and see if they match a list of regular expressions
#each regular expression is associated with a multichoice value, and for each regular expression that matches for that file that multichoice will be set
#
#
# example rule additional parameters to get this to work
#
#   name              |   Value
# ----------------------------------------
# ScriptFileName      |  content_classifier.ps1
# PropertyList        |  PhoneNumber|SSN|ZipCode
# PhoneNumber         | \(?\s*\d{3}\s*\)?\s*-?\s*\d{3}\s*-?\s*\d{4}
# SSN                 | \d{3}-\d{2}\d{4}
# ZipCode             | \d{5}(-?\d{4})?
#
# this example will set a multichoice value and state if the file contents contains a US phone number, a SSN or a US ZipCode
# using regular expressions
#
# this script demonstrates using the rule parameters


#you can do preprocessing here but do not emit values!!!
begin{

	#ALL FUNCTIONS NEED TO BE DECLARED INSIDE BEGIN!!!!!

	#this is just an example function that joins an array with "|"
	function JoinReturn($UnJoinedVariables)
	{
		$UnJoinedVariables -join "|"
	}


	#a hash that will contain all the rule parameters and provide easy lookup
	$ruleParams = @{}

	#process the rule parameters
	foreach($ruleParam in $rule.Parameters){

		$key,$value = $ruleParam -split "=",2
		$ruleParams[$key] = $value
	}

	#split the propertyList up to make it easier to enumerate over
	$ruleParams.PropertyList = $ruleParams.PropertyList -split ","
}



#In Classifier Supplied Value mode inside the process block, for each property bag if you want the property updated emit the correct object, otherwise emit no object
#emitting multiple values in a single loop is an error and will fail the property setting for that property bag

#Bool - a boolean object either $true or $false
#Number - an integer
#string - a string
#orderedlist - a string
#multichoice - a string composed of values optionally concatenated by pipe ex1: "value1"  ex2: "value1|value2|value3"
#multistring - a string composed of multiple strings concatenated by pipe ex1: "some random string"  ex2: "anystring|some other random string|the number 5"
#date-time - a datetime object

#this sample is multichoice
process{

	$propertyBag = $_

	#if this isn't a .txt file don't process it
	if(!($propertyBag.Name -like "*.txt"))
	{
		return
	}


	$returnValue = @()

	$fileStream = $propertyBag.GetStream()
	$fileStreamReader = new-object System.IO.StreamReader($fileStream)

	#search each line for a match
	#this can be done reading the whole stream into a string
	#this is just a much friendlier way of searching as it doesn't allocate lots of memory
	while(!$fileStreamReader.EndOfStream)
	{
		$line = $fileStreamReader.ReadLine()
		foreach($property in $ruleParams.PropertyList)
		{
			#we get a regex match on the current line and it doesn't currently exist add it to the return values
			if( $ruleParams.ContainsKey($property) -and $line -match $ruleParams[$property] -and !($returnValue -contains $property))
			{
				$returnValue+=$property
			}
		}
	}

	#lets make those streams close quicker
	$fileStreamReader.Close()
	$fileStream.Close()
	
	#if got results concatenate them into a string and return them
	if($returnValue.Length -gt 0)
	{		
		#join the return results into a string
		$returnString = JoinReturn $returnValue 

		#write the return string
		$returnString
		
	}

}


# you can do cleanup in here but DO NOT EMIT VALUES!!!
end {


}
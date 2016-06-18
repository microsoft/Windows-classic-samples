#############################################################################
# Contains the implementation of the advanced function Get-Numbers.
#
# This is a sample of using the SupportsPaging interface. The SupportsPaging
# parameter of the CmdletBinding attribute allows a user to add the
# IncludeTotalCount, Skip, and First parameters to a cmdlet. These parameters
# are used for paging the results from a data source query operation.
#
# The Get-Numbers cmdlet generates up to 100 consecutive numbers starting
# from 0. The IncludeTotalCount, Skip, and First parameters enable the user
# to perform paging operations on the set of numbers returned by the cmdlet.
#
# If the data source doesn't natively support paging, the query may return
# all matching results, and the cmdlet can perform paging itself based on
# the values specified for the Skip and First parameters.
#
# If the data source can natively perform paging, then paging parameter
# values could be passed into a query that is executed by the data source.
#############################################################################

function Get-Numbers
{
    [CmdletBinding(SupportsPaging = $true)]
    param(
        [Parameter(Position = 0, ValueFromPipeline = $true)]
        [ValidateRange(0, 100)]
        [Uint64] 
        $NumbersToGenerate = 100
    )
    
    if ($PSCmdlet.PagingParameters.IncludeTotalCount)
    {
        <#
        when using data sources to retrieve results,
            (1) some data sources might have the exact number of results retrieved and in this case would have accuracy 1.0
            (2) some data sources might only have an estimate and in this case would use accuracy between 0.0 and 1.0
            (3) other data sources might not know how many items there are in total and in this case would use accuracy 0.0
        #>
        [double]$Accuracy = 1.0
        $PSCmdlet.PagingParameters.NewTotalCount($NumbersToGenerate, $Accuracy)
    }
    
    if($NumbersToGenerate -gt 0)
    {
        if($PSCmdlet.PagingParameters.Skip -ge $NumbersToGenerate)
        {
            Write-Verbose "No results satisfy the paging parameters"
        }
	elseif($PSCmdlet.PagingParameters.First -eq 0)
	{
	    Write-Verbose "No results satisfy the paging parameters"
	}
        else
        {
            $FirstNumber = $PSCmdlet.PagingParameters.Skip
            $LastNumber = $FirstNumber + 
                [Math]::Min($PSCmdlet.PagingParameters.First, $NumbersToGenerate - $PSCmdlet.PagingParameters.Skip) - 1
                
            $FirstNumber .. $LastNumber | %{ New-Object PSObject -Prop @{
                Number = $_
                Skip = $PSCmdlet.PagingParameters.Skip
                First = $PSCmdlet.PagingParameters.First
                IncludeTotalCount = $PSCmdlet.PagingParameters.IncludeTotalCount
            } }
        }
    }
    else
    {
         Write-Verbose "No results satisfy the specified paging parameters"
    }
}
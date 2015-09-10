// <copyright file="GetNumbersCommand.cs" company="Microsoft Corporation">
// Copyright (c) 2012 Microsoft Corporation. All rights reserved.
// </copyright>
// DISCLAIMER OF WARRANTY: The software is licensed “as-is.” You 
// bear the risk of using it. Microsoft gives no express warranties, 
// guarantees or conditions. You may have additional consumer rights 
// under your local laws which this agreement cannot change. To the extent 
// permitted under your local laws, Microsoft excludes the implied warranties 
// of merchantability, fitness for a particular purpose and non-infringement.

using System;
using System.Management.Automation;

namespace SupportsPaging01
{
    /// <summary>
    /// Contains the implementation of the Get-Numbers cmdlet.
    /// </summary>
    /// <remarks>
    /// The SupportsPaging parameter of the Cmdlet attribute allows a user
    /// to specify the -IncludeTotalCount, -Skip, and -First parameters, which
    /// are used for paging the results from a data-source query operation.
    /// 
    /// The Get-Numbers cmdlet generates upto 100 consecutive numbers starting
    /// from 0, and do paging operations on them.
    /// 
    /// When some data source is used for generating query results, if the data
    /// source can natively perform paging, then paging parameters could be passed
    /// into a query that is executed by the data source.
    /// </remarks>
    [Cmdlet(VerbsCommon.Get, "Numbers", SupportsPaging = true)]
    public sealed class GetNumbersCommandV1 : PSCmdlet
    {
        /// <summary>
        /// User can specify how many numbers to generate, the default is 100.
        /// </summary>
        [Parameter(Position = 0, ValueFromPipeline = true)]
        [ValidateRange(0, 100)]
        public int NumbersToGenerate
        {
            get { return _numbersToGen; }
            set { _numbersToGen = value; }
        }
        private int _numbersToGen = 100;


        protected override void ProcessRecord()
        {
            if (this.PagingParameters.IncludeTotalCount)
            {
                // when using data sources to retrieve results,
                //  (1) some data sources might have the exact number of results retrieved and in this case would have accuracy 1.0
                //  (2) some data sources might only have an estimate and in this case would use accuracy between 0.0 and 1.0
                //  (3) other data sources might not know how many items there are in total and in this case would use accuracy 0.0
                double accuracy = 1.0;
                PSObject totalCountResult = this.PagingParameters.NewTotalCount((UInt64)_numbersToGen, accuracy);
                this.WriteObject(totalCountResult);
            }

            if (_numbersToGen > 0)
            {
                if (this.PagingParameters.Skip >= (UInt64)_numbersToGen)
                {
                    this.WriteVerbose("No results satisfy the paging parameters");
                }
                else
                {
                    UInt64 firstNumber = this.PagingParameters.Skip;
                    UInt64 lastNumber = firstNumber +
                        Math.Min(this.PagingParameters.First, (UInt64)_numbersToGen - this.PagingParameters.Skip);

                    for (UInt64 i = firstNumber; i < lastNumber; i++)
                    {
                        Result result = new Result(
                            i,
                            this.PagingParameters.Skip,
                            this.PagingParameters.First,
                            this.PagingParameters.IncludeTotalCount);

                        this.WriteObject(result);
                    }
                }
            }
            else
            {
                this.WriteVerbose("No results generated");
            }
        }
    }
}

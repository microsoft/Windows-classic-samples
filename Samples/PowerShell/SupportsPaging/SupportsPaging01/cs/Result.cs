// <copyright file="Result.cs" company="Microsoft Corporation">
// Copyright (c) 2012 Microsoft Corporation. All rights reserved.
// </copyright>
// DISCLAIMER OF WARRANTY: The software is licensed “as-is.” You 
// bear the risk of using it. Microsoft gives no express warranties, 
// guarantees or conditions. You may have additional consumer rights 
// under your local laws which this agreement cannot change. To the extent 
// permitted under your local laws, Microsoft excludes the implied warranties 
// of merchantability, fitness for a particular purpose and non-infringement.

using System;

namespace SupportsPaging01
{
    // A helper class for displaying the sample reults
    public sealed class Result
    {
        private UInt64 _number;
        private UInt64 _skip;
        private UInt64 _first;
        private bool _includeTotalCount;

        public Result(UInt64 number, UInt64 skip, UInt64 first, bool includeTotalCount)
        {
            this._number = number;
            this._skip = skip;
            this._first = first;
            this._includeTotalCount = includeTotalCount;
        }

        public UInt64 Number { get { return _number; } }
        public UInt64 Skip { get { return _skip; } }
        public UInt64 First { get { return _first; } }
        public bool IncludeTotalCount { get { return _includeTotalCount; } }
    }
}

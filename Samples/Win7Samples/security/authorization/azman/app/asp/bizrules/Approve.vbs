'THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
'ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
'TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
'PARTICULAR PURPOSE.

'Copyright (C) 1987 - 2002.  Microsoft Corporation.  All rights reserved.

AzBizRuleContext.BusinessRuleResult = False
Dim Amount

Amount = AzBizRuleContext.GetParameter("ExpAmount")

' Do not accept approvals on Thursdays. When not Thursday, only allow
' approvals for amounts less than $500.
If ( Not ( Weekday( Now ) = 5 ) ) Then
	If ( Amount < 500 ) Then AzBizRuleContext.BusinessRuleResult = True
End If

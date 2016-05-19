' BizRule to check if the current user is the direct manager of the person
' identified as the submitter
'

On error resume next


' Assume direct manager check fails
'
AzBizRuleContext.BusinessRuleResult = FAlSE


'-Get Needed BizRule Params
'
SubmitterName = AzBizRuleContext.GetParameter( "SubmitterName")
UserName = AzBizRuleContext.GetParameter( "UserName")

if SubmitterName = "NA" then
   wscript.quit
End if


'-Build DN of current user
'
UserNameAsDN = "CN=" & UserName & ",CN=TreyExternalUsers"


'-Bind to the Submitter's object
'
set UserObj = GetObject("LDAP://LH-T4Q9ESSVU2JS/cn=" & SubmitterName & ",CN=TreyExternalUsers" )
if Err.Number <> 0 then
   '- if we didn't get the account for any reason just quit
   AzBizRuleContext.BusinessRuleString = "Unexpected error in direct manager bizrule"
   AzBizRuleContext.BusinessRuleResult = FAlSE
   WScript.Quit
end if


'-Check if the current user is the submitter's manager
'
if StrComp (UserObj.Manager, UserNameAsDN, vbTextCompare) = 0 then
   AzBizRuleContext.BusinessRuleResult = TRUE
else
   AzBizRuleContext.BusinessRuleString = "Only direct managers can approve expenses"
   AzBizRuleContext.BusinessRuleResult = FAlSE
End if
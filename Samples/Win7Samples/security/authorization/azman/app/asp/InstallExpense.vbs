
'--- Initilaize the admin manager object

Dim pAzManStore
Set pAzManStore = CreateObject("AzRoles.AzAuthorizationStore")


'--- Create a new store for expense app
' AZ_AZSTORE_FLAG_CREATE	= 0x1,
' AZ_AZSTORE_FLAG_MANAGE_STORE_ONLY	= 0x2,
' AZ_AZSTORE_FLAG_BATCH_UPDATE	= 0x4,

pAzManStore.Initialize 1+2, "msxml://C:\AzStore.xml"
'pAzManStore.Initialize 1+2, "msldap://CN=MyWebAppsAzStore,CN=Program Data,DC=azroles,DC=com"

pAzManStore.Submit

Dim App1
Set App1 = pAzManStore.CreateApplication("Expense Web")
App1.Submit



'--- create operations -----------------------

Dim Op1
Set Op1=App1.CreateOperation("Submit")
Op1.OperationID = CLng(1)
Op1.Submit

Dim Op2
Set Op2=App1.CreateOperation("Approve")
Op2.OperationID = CLng(2)
Op2.Submit

Dim Op3
Set Op3=App1.CreateOperation("ReadExpense")
Op3.OperationID = CLng(3)
Op3.Submit

Dim Op4
Set Op4=App1.CreateOperation("ListExpenses")
Op4.OperationID = CLng(4)
Op4.Submit

'--- Create Tasks ------------------------------

Dim Task2
Set Task2 = App1.CreateTask("Submit Expense")
Task2.BizRuleLanguage = CStr("VBScript")
Task2.AddOperation CStr("Submit")
Task2.BizRule = "Dim Amount" & vbnewline  & _
                "AzBizRuleContext.BusinessRuleResult = FALSE" & vbnewline & _
                "Amount = AzBizRuleContext.GetParameter( " & Chr(34) & _
                              "ExpAmount" & Chr(34) & ")"  & vbNewLine & _
                "if Amount < 500 then AzBizRuleContext.BusinessRuleResult = TRUE"

Task2.Submit

Set Task2 = App1.CreateTask("Approve Expense")
Task2.BizRuleLanguage = CStr("VBScript")
Task2.AddOperation CStr("Approve")

Task2.Submit

'--- Create Role definitions ------------------------------
Set Task3 = App1.CreateTask("Submitter")
Task3.AddTask CStr("Submit Expense")
Task3.IsRoleDefinition = TRUE
Task3.Submit

Set Task3 = App1.CreateTask("Approver")
Task3.AddTask CStr("Approve Expense")
Task3.IsRoleDefinition = TRUE
Task3.Submit

'--- Create Initial Scopes and Roles ------------------------------
'--- only one scope in this app (we may instead choose to use no scope)

Set RoleA=App1.CreateRole("Submitter")
RoleA.AddTask("Submitter")
RoleA.Submit

Set RoleB=App1.CreateRole("Approver")
RoleB.AddTask("Approver")
RoleB.Submit

'--- Create Application Group --------------------------

Set Group1 = pAzManStore.CreateApplicationGroup("Managers")
Group1.Type = 1
Group1.LdapQuery = "(title=Manager)"
Group1.Submit


'--- demo - add everyone to Expense User Role --------------------------
RoleA.AddMember("S-1-1-0")
RoleA.Submit

'--- demo - add managers to Manager Role --------------------------
RoleB.AddAppMember("Managers")
RoleB.Submit


'-------------------------------------
' setup url auth
'-------------------------------------
Dim App2
Set App2 = pAzManStore.CreateApplication("IIS 6.0 URL Authorization")
App2.Submit

Set Op21=App2.CreateOperation("AccessURL")
Op21.OperationID = 1
Op21.Submit


Set Scope1 = App2.CreateScope("Expense Web")
Scope1.Submit

'use adsi scripting for IIS to configure
'AzImpersonationLevel,AzEnable,AzScopeName,AzStoreName in IIS metabase
'SetURLAuth.vbs demonstrates setting the IIS meta-base attributes.




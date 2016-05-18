Option Explicit

'--- declare flags
Dim AZ_AZSTORE_FLAG_CREATE : AZ_AZSTORE_FLAG_CREATE	                  = 1
Dim AZ_AZSTORE_FLAG_MANAGE_STORE_ONLY : AZ_AZSTORE_FLAG_MANAGE_STORE_ONLY = 2
Dim AZ_AZSTORE_FLAG_BATCH_UPDATE : AZ_AZSTORE_FLAG_BATCH_UPDATE           = 4
Dim AZ_AZSTORE_NT6_FUNCTION_LEVEL : AZ_AZSTORE_NT6_FUNCTION_LEVEL         = 32

'--- Initilaize the authorization store object
Dim pAzManStore
Set pAzManStore = CreateObject("AzRoles.AzAuthorizationStore")


'--- Create a new store for expense app
'pAzManStore.Initialize 1+2, "msxml://C:\AzStore.xml"
'--- Uncomment the following line to use AD as the policy store
'pAzManStore.Initialize 1+2, "msldap://CN=AzStore,CN=Program Data,DC=demo-man,DC=com"
'pAzManStore.Initialize 1+2, "msldap://AZMANDEMO:389/CN=PolicyStore,CN=AzMan"
pAzManStore.Initialize 1+2, "mssql://Driver={SQL Server};Server={LH-T4Q9ESSVU2JS};/AzManDB/WebExpenseStore"

pAzManStore.Submit

pAzManStore.AddPolicyReaderName("Network Service")
pAzManStore.UpgradeStoresFunctionalLevel (AZ_AZSTORE_NT6_FUNCTION_LEVEL)
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

Dim Op5
Set Op5=App1.CreateOperation("Administer")
Op5.OperationID = CLng(5)
Op5.Submit

'--- Create Tasks ------------------------------

Dim Task2
Set Task2 = App1.CreateTask("Submit Expense")
Task2.BizRuleLanguage = CStr("VBScript")
Task2.AddOperation CStr("Submit")
Task2.AddOperation CStr("ReadExpense")
Task2.BizRule = "Dim Amount" & vbnewline  & _
                "AzBizRuleContext.BusinessRuleResult = FALSE" & vbnewline & _
                "AzBizRuleContext.BusinessRuleString = " & Chr(34) & "You are not authorized to submit an expense for the specified amount." & Chr(34) & vbnewline & _
                "Amount = AzBizRuleContext.GetParameter( " & Chr(34) & _
                              "Amount" & Chr(34) & ")"  & vbNewLine & _
                "if Amount < 500 then AzBizRuleContext.BusinessRuleResult = TRUE"
'Task2.BizRulePath = "none"
Task2.Submit

'AzApp.BizRulesEnabled = True

Set Task2 = App1.CreateTask("Approve Expense")
Task2.AddOperation CStr("Approve")
Task2.AddOperation CStr("ReadExpense")
Task2.Submit

Set Task2 = App1.CreateTask("Administer Settings")
Task2.AddOperation CStr("Administer")
Task2.Submit

Set Task2 = App1.CreateTask("View Pending Expenses")
Task2.AddOperation CStr("ListExpenses")
Task2.Submit

'--- Create Role definitions ------------------------------
Dim Task3
Set Task3 = App1.CreateTask("Expense Manager")
Task3.AddTask CStr("View Pending Expenses")
Task3.IsRoleDefinition = TRUE
Task3.Submit

Set Task3 = App1.CreateTask("Submitter")
Task3.AddTask CStr("Submit Expense")
Task3.IsRoleDefinition = TRUE
Task3.Submit

Set Task3 = App1.CreateTask("Approver")
Task3.AddTask CStr("Approve Expense")
Task3.IsRoleDefinition = TRUE
Task3.Submit

Set Task3 = App1.CreateTask("Administrator")
Task3.AddTask CStr("Administer Settings")
Task3.IsRoleDefinition = TRUE
Task3.Submit

'--- Create Initial Scopes and Roles ------------------------------
'--- only one scope in this app (we may instead choose to use no scope)
Dim RoleA
Set RoleA=App1.CreateRole("Submitter")
RoleA.AddTask("Submitter")
RoleA.Submit

Dim RoleB
Set RoleB = App1.CreateRole("Approver")
RoleB.AddTask("Approver")
RoleB.Submit

Dim RoleC
Set RoleC = App1.CreateRole("Expense Manager")
RoleC.AddTask("Expense Manager")
RoleC.Submit

Dim RoleD
Set RoleD = App1.CreateRole("Administrator")
RoleD.AddTask("Administrator")
RoleD.Submit

'--- Create Application Group --------------------------
Dim Group1
Set Group1 = pAzManStore.CreateApplicationGroup("Managers")
Group1.Type = 1
Group1.LdapQuery = "(title=Manager)"
Group1.Submit

Dim Group2
Set Group2 = pAzManStore.CreateApplicationGroup("Admins")
Group2.Type = 1
Group2.LdapQuery = "(title=Admin)"
Group2.Submit



'--- demo - add managers to Manager Role --------------------------
Rolec.AddAppMember("Managers")
Rolec.Submit

RoleD.AddAppMember("Admins")
RoleD.Submit

wscript.echo "Done"
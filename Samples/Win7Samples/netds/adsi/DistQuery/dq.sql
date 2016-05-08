'-- This tells SQL Server to associate word 'ADSI' with ADSI OLE DB provider - 'ADSDSOObject'
sp_addlinkedserver 'ADSI', 'Active Directory Services 2.5', 'ADSDSOObject', 'adsdatasource'
go

'-- Get the information for Active Directory
SELECT * FROM OpenQuery( ADSI,'<LDAP://DC=fabrikam,DC=com>;(objectClass=user);adspath;subtree')

'-- Or you can also use SQL Dialect
SELECT * FROM OpenQuery( ADSI, 'SELECT name, adsPath FROM ''LDAP://DC=Fabrikam,DC=com'' WHERE objectCategory = ''Person'' AND objectClass= ''user''')

'--Creating a view
CREATE VIEW viewADUsers AS
SELECT * FROM OpenQuery( ADSI,'<LDAP://DC=Fabrikam,DC=com>;(&(objectCategory=Person)(objectClass=user));name, adspath;subtree')

SELECT * from viewADUsers

'-- Creating a SQL table, a employee performance review table

CREATE TABLE EMP_REVIEW
(
userName varChar(40),
reviewDate datetime,
rating decimal 
)

'--Insert few records

INSERT EMP_REVIEW VALUES('Administrator', '2/15/1998', 4.5 )
INSERT EMP_REVIEW VALUES('Administrator', '7/15/1998', 4.0 )



'--Now join the two!

SELECT ADsPath, userName, ReviewDate, Rating 
FROM EMP_REVIEW, viewADUsers
WHERE userName = Name

'--- Creating a report for this join
CREATE VIEW reviewReport
SELECT ADsPath, userName, ReviewDate, Rating 
FROM EMP_REVIEW, viewADUsers
WHERE userName = Name


'-----------------------------------------------------
'--- Advanced Operations
'------------------------------------------------------
--Maps the user name to the OLE DB user name (in this case Active Directory)
sp_addlinkedsrvlogin ADSI, false, 'MICROSOFT\Administrator', 'CN=Administrator,CN=Users,DC=Microsoft,DC=com', 'passwordHere'
go

'-- To stop supplying credential
sp_droplinkedsrvlogin ADSI,'MICROSOFT\Administrator'


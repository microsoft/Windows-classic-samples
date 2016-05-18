- Setup SQL Server.
    - Create a database named wmslog
    - Create a SQL user with password
    - In SQL Server Properties, security tab, check "SQL Server and Windows" under Authentication.

- Open VBDBLoggingPlugin.sln with Visual Studio .Net.

- Change strDataSource to your SQL server name

- Change strUserId to your SQL user name

- Change strPassword to your SQL user password

- Build the project.

- Refresh the server event plug-in list.

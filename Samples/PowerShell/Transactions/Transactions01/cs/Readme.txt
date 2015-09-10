Transacted Comment Sample
=========================
     This sample shows a set of cmdlets that participate in Windows PowerShell transactions. These
     cmdlets provide a comment log that can be changed, completed, or rolled back along with the
     rest of the transaction. It also provides a simple transactional resource manager that stores
     a string.

     For Windows PowerShell information on MSDN, see http://go.microsoft.com/fwlink/?LinkID=178145


Sample Objectives
=================
     This sample demonstrates the following:

     1. Usage of SupportsTransactionsAttribute
     2. Usage of CurrentPSTransaction
     3. Implementation of a .NET transacted resource manager


Sample Language Implementations
===============================
     This sample is available in the following language implementations:

     - C#


Building the Sample Using Visual Studio
=======================================
     1. Open File Explorer and navigate to TransactedComment under the samples directory.
     2. Double-click the icon for the .sln (solution) file to open the file in Visual Studio.
     3. In the Build menu, select Build Solution.
     4. The library will be built in the default \bin or \bin\Debug directory.


Running the Sample
==================
     1. Store the assembly in the following module folder:
        [user]/Documents/WindowsPowerShell/Modules/TransactedComment
     2. Start Windows PowerShell.
     3. Run the following command: Import-Module TransactedComment
        (This command loads the assembly into Windows PowerShell.)


Using the Sample
================
     1. Start-Transaction
     2. Add-TransactedComment -UseTransaction "Hello World!"
     3. Get-TransactedComment
     4. Get-TransactedComment -UseTransaction
     5. Complete-Transaction
     6. Get-TransactedComment

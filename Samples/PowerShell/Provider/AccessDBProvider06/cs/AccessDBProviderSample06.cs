// <copyright file="AccessDBProviderSample06.cs" company="Microsoft Corporation">
// Copyright (c) 2009 Microsoft Corporation. All rights reserved.
// </copyright>
// DISCLAIMER OF WARRANTY: The software is licensed “as-is.” You 
// bear the risk of using it. Microsoft gives no express warranties, 
// guarantees or conditions. You may have additional consumer rights 
// under your local laws which this agreement cannot change. To the extent 
// permitted under your local laws, Microsoft excludes the implied warranties 
// of merchantability, fitness for a particular purpose and non-infringement.

using System;
using System.IO;
using System.Data;
using System.Data.Odbc;
using System.Diagnostics;
using System.Collections;
using System.Collections.ObjectModel;
using System.Management.Automation;
using System.Management.Automation.Provider;
using System.Text;
using System.Text.RegularExpressions;
using System.ComponentModel;
using System.Globalization;

namespace Microsoft.Samples.PowerShell.Providers
{
   #region AccessDBProvider

   /// <summary>
   /// This example implements the content methods.
   /// </summary>
   [CmdletProvider("AccessDB", ProviderCapabilities.None)]
   public class AccessDBProvider : NavigationCmdletProvider, IContentCmdletProvider
   {

       #region Drive Manipulation

        /// <summary>
        /// Create a new drive.  Create a connection to the database file and set
        /// the Connection property in the PSDriveInfo.
        /// </summary>
        /// <param name="drive">
        /// Information describing the drive to add.
        /// </param>
        /// <returns>The added drive.</returns>
        protected override PSDriveInfo NewDrive(PSDriveInfo drive)
        {
            // check if drive object is null
            if (drive == null)
            {
                WriteError(new ErrorRecord(
                    new ArgumentNullException("drive"),
                    "NullDrive",
                    ErrorCategory.InvalidArgument,
                    null)
                );

                return null;
            }

            // check if drive root is not null or empty
            // and if its an existing file
            if (String.IsNullOrEmpty(drive.Root) || (File.Exists(drive.Root) == false))
            {
                WriteError(new ErrorRecord(
                    new ArgumentException("drive.Root"),
                    "NoRoot",
                    ErrorCategory.InvalidArgument,
                    drive)
                );

                return null;
            }

            // create a new drive and create an ODBC connection to the new drive
            AccessDBPSDriveInfo accessDBPSDriveInfo = new AccessDBPSDriveInfo(drive);

            OdbcConnectionStringBuilder builder = new OdbcConnectionStringBuilder();

            builder.Driver = "Microsoft Access Driver (*.mdb)";
            builder.Add("DBQ", drive.Root);

            OdbcConnection conn = new OdbcConnection(builder.ConnectionString);
            conn.Open();
            accessDBPSDriveInfo.Connection = conn;

            return accessDBPSDriveInfo;
        } // NewDrive

        /// <summary>
        /// Removes a drive from the provider.
        /// </summary>
        /// <param name="drive">The drive to remove.</param>
        /// <returns>The drive removed.</returns>
        protected override PSDriveInfo RemoveDrive(PSDriveInfo drive)
        {
            // check if drive object is null
            if (drive == null)
            {
                WriteError(new ErrorRecord(
                    new ArgumentNullException("drive"),
                    "NullDrive",
                    ErrorCategory.InvalidArgument,
                    drive)
                );

                return null;
            }

            // close ODBC connection to the drive
            AccessDBPSDriveInfo accessDBPSDriveInfo = drive as AccessDBPSDriveInfo;

            if (accessDBPSDriveInfo == null)
            {
                return null;
            }
            accessDBPSDriveInfo.Connection.Close();

            return accessDBPSDriveInfo;
        } // RemoveDrive

        #endregion Drive Manipulation

       #region Item Methods

        /// <summary>
        /// Retrieves an item using the specified path.
        /// </summary>
        /// <param name="path">The path to the item to return.</param>
        protected override void GetItem(string path)
        {
            // check if the path represented is a drive
            if (PathIsDrive(path))
            {
                WriteItemObject(this.PSDriveInfo, path, true);
                return;
            }// if (PathIsDrive...

            // Get table name and row information from the path and do 
            // necessary actions
            string tableName;
            int rowNumber;

            PathType type = GetNamesFromPath(path, out tableName, out rowNumber);

            if (type == PathType.Table)
            {
                DatabaseTableInfo table = GetTable(tableName);
                WriteItemObject(table, path, true);
            }
            else if (type == PathType.Row)
            {
                DatabaseRowInfo row = GetRow(tableName, rowNumber);
                WriteItemObject(row, path, false);
            }
            else
            {
                ThrowTerminatingInvalidPathException(path);
            }

        } // GetItem

        /// <summary>
        /// Set the content of a row of data specified by the supplied path
        /// parameter.
        /// </summary>
        /// <param name="path">Specifies the path to the row whose columns
        /// will be updated.</param>
        /// <param name="values">Comma separated string of values</param>
        protected override void SetItem(string path, object values)
        {
            // Get type, table name and row number from the path specified
            string tableName;
            int rowNumber;

            PathType type = GetNamesFromPath(path, out tableName, out rowNumber);

            if (type != PathType.Row)
            {
                WriteError(new ErrorRecord(new NotSupportedException(
                      "SetNotSupported"), "",
                   ErrorCategory.InvalidOperation, path));

                return;
            }

            // Get in-memory representation of table
            OdbcDataAdapter da = GetAdapterForTable(tableName);

            if (da == null)
            {
                return;
            }
            DataSet ds = GetDataSetForTable(da, tableName);
            DataTable table = GetDataTable(ds, tableName);

            if (rowNumber >= table.Rows.Count)
            {
                // The specified row number has to be available. If not
                // NewItem has to be used to add a new row
                throw new ArgumentException("Row specified is not available");
            } // if (rowNum...

            string[] colValues = (values as string).Split(',');

            // set the specified row
            DataRow row = table.Rows[rowNumber];

            for (int i = 0; i < colValues.Length; i++)
            {
                row[i] = colValues[i];
            }

            // Update the table
            if (ShouldProcess(path, "SetItem"))
            {
                da.Update(ds, tableName);
            }

        } // SetItem

        /// <summary>
        /// Test to see if the specified item exists.
        /// </summary>
        /// <param name="path">The path to the item to verify.</param>
        /// <returns>True if the item is found.</returns>
        protected override bool ItemExists(string path)
        {
            // check if the path represented is a drive
            if (PathIsDrive(path))
            {
                return true;
            }

            // Obtain type, table name and row number from path
            string tableName;
            int rowNumber;

            PathType type = GetNamesFromPath(path, out tableName, out rowNumber);

            DatabaseTableInfo table = GetTable(tableName);

            if (type == PathType.Table)
            {
                // if specified path represents a table then DatabaseTableInfo
                // object for the same should exist
                if (table != null)
                {
                    return true;
                }
            }
            else if (type == PathType.Row)
            {
                // if specified path represents a row then DatabaseTableInfo should
                // exist for the table and then specified row number must be within
                // the maximum row count in the table
                if (table != null && rowNumber < table.RowCount)
                {
                    return true;
                }
            }

            return false;

        } // ItemExists

        /// <summary>
        /// Test to see if the specified path is syntactically valid.
        /// </summary>
        /// <param name="path">The path to validate.</param>
        /// <returns>True if the specified path is valid.</returns>
        protected override bool IsValidPath(string path)
        {
            bool result = true;

            // check if the path is null or empty
            if (String.IsNullOrEmpty(path))
            {
                result = false;
            }

            // convert all separators in the path to a uniform one
            path = NormalizePath(path);

            // split the path into individual chunks
            string[] pathChunks = path.Split(pathSeparator.ToCharArray());

            foreach (string pathChunk in pathChunks)
            {
                if (pathChunk.Length == 0)
                {
                    result = false;
                }
            }
            return result;
        } // IsValidPath

        #endregion Item Overloads

       #region Container Overloads

        /// <summary>
        /// Return either the tables in the database or the datarows
        /// </summary>
        /// <param name="path">The path to the parent</param>
        /// <param name="recurse">True to return all child items recursively.
        /// </param>
        protected override void GetChildItems(string path, bool recurse)
        {
            // If path represented is a drive then the children in the path are 
            // tables. Hence all tables in the drive represented will have to be
            // returned
            if (PathIsDrive(path))
            {
                foreach (DatabaseTableInfo table in GetTables())
                {
                    WriteItemObject(table, path, true);

                    // if the specified item exists and recurse has been set then 
                    // all child items within it have to be obtained as well
                    if (ItemExists(path) && recurse)
                    {
                        GetChildItems(path + pathSeparator + table.Name, recurse);
                    }
                } // foreach (DatabaseTableInfo...
            } // if (PathIsDrive...
            else
            {
                // Get the table name, row number and type of path from the
                // path specified
                string tableName;
                int rowNumber;

                PathType type = GetNamesFromPath(path, out tableName, out rowNumber);

                if (type == PathType.Table)
                {
                    // Obtain all the rows within the table
                    foreach (DatabaseRowInfo row in GetRows(tableName))
                    {
                        WriteItemObject(row, path + pathSeparator + row.RowNumber,
                                false);
                    } // foreach (DatabaseRowInfo...
                }
                else if (type == PathType.Row)
                {
                    // In this case the user has directly specified a row, hence
                    // just give that particular row
                    DatabaseRowInfo row = GetRow(tableName, rowNumber);
                    WriteItemObject(row, path + pathSeparator + row.RowNumber,
                                false);
                }
                else
                {
                    // In this case, the path specified is not valid
                    ThrowTerminatingInvalidPathException(path);
                }
            } // else
        } // GetChildItems

        /// <summary>
        /// Return the names of all child items.
        /// </summary>
        /// <param name="path">The root path.</param>
        /// <param name="returnContainers">Not used.</param>
        protected override void GetChildNames(string path,
                                      ReturnContainers returnContainers)
        {
            // If the path represented is a drive, then the child items are
            // tables. get the names of all the tables in the drive.
            if (PathIsDrive(path))
            {
                foreach (DatabaseTableInfo table in GetTables())
                {
                    WriteItemObject(table.Name, path, true);
                } // foreach (DatabaseTableInfo...
            } // if (PathIsDrive...
            else
            {
                // Get type, table name and row number from path specified
                string tableName;
                int rowNumber;

                PathType type = GetNamesFromPath(path, out tableName, out rowNumber);

                if (type == PathType.Table)
                {
                    // Get all the rows in the table and then write out the 
                    // row numbers.
                    foreach (DatabaseRowInfo row in GetRows(tableName))
                    {
                        WriteItemObject(row.RowNumber, path, false);
                    } // foreach (DatabaseRowInfo...
                }
                else if (type == PathType.Row)
                {
                    // In this case the user has directly specified a row, hence
                    // just give that particular row
                    DatabaseRowInfo row = GetRow(tableName, rowNumber);

                    WriteItemObject(row.RowNumber, path, false);
                }
                else
                {
                    ThrowTerminatingInvalidPathException(path);
                }
            } // else
        } // GetChildNames

        /// <summary>
        /// Determines if the specified path has child items.
        /// </summary>
        /// <param name="path">The path to examine.</param>
        /// <returns>
        /// True if the specified path has child items.
        /// </returns>
        protected override bool HasChildItems(string path)
        {
            if (PathIsDrive(path))
            {
                return true;
            }

            return (ChunkPath(path).Length == 1);
        } // HasChildItems

        /// <summary>
        /// Creates a new item at the specified path.
        /// </summary>
        /// 
        /// <param name="path">
        /// The path to the new item.
        /// </param>
        /// 
        /// <param name="type">
        /// Type for the object to create. "Table" for creating a new table and
        /// "Row" for creating a new row in a table.
        /// </param>
        /// 
        /// <param name="newItemValue">
        /// Object for creating new instance of a type at the specified path. For
        /// creating a "Table" the object parameter is ignored and for creating
        /// a "Row" the object must be of type string which will contain comma 
        /// separated values of the rows to insert.
        /// </param>
        protected override void NewItem(string path, string type,
                                    object newItemValue)
        {
            string tableName;
            int rowNumber;

            PathType pt = GetNamesFromPath(path, out tableName, out rowNumber);

            if (pt == PathType.Invalid)
            {
                ThrowTerminatingInvalidPathException(path);
            }

            // Check if type is either "table" or "row", if not throw an 
            // exception
            if (!String.Equals(type, "table", StringComparison.OrdinalIgnoreCase)
                && !String.Equals(type, "row", StringComparison.OrdinalIgnoreCase))
            {
                WriteError(new ErrorRecord
                                  (new ArgumentException("Type must be either a table or row"),
                                      "CannotCreateSpecifiedObject",
                                         ErrorCategory.InvalidArgument,
                                              path
                                   )
                          );

                throw new ArgumentException("This provider can only create items of type \"table\" or \"row\"");
            }

            // Path type is the type of path of the container. So if a drive
            // is specified, then a table can be created under it and if a table
            // is specified, then a row can be created under it. For the sake of 
            // completeness, if a row is specified, then if the row specified by
            // the path does not exist, a new row is created. However, the row 
            // number may not match as the row numbers only get incremented based 
            // on the number of rows

            if (PathIsDrive(path))
            {
                if (String.Equals(type, "table", StringComparison.OrdinalIgnoreCase))
                {
                    // Execute command using ODBC connection to create a table
                    try
                    {
                        // create the table using an sql statement
                        string newTableName = newItemValue.ToString();
                        string sql = "create table " + newTableName
                                             + " (ID INT)";

                        // Create the table using the Odbc connection from the 
                        // drive.
                        AccessDBPSDriveInfo di = this.PSDriveInfo as AccessDBPSDriveInfo;

                        if (di == null)
                        {
                            return;
                        }
                        OdbcConnection connection = di.Connection;

                        if (ShouldProcess(newTableName, "create"))
                        {
                            OdbcCommand cmd = new OdbcCommand(sql, connection);
                            cmd.ExecuteScalar();
                        }
                    }
                    catch (Exception ex)
                    {
                        WriteError(new ErrorRecord(ex, "CannotCreateSpecifiedTable",
                                  ErrorCategory.InvalidOperation, path)
                                  );
                    }
                } // if (String...
                else if (String.Equals(type, "row", StringComparison.OrdinalIgnoreCase))
                {
                    throw new
                        ArgumentException("A row cannot be created under a database, specify a path that represents a Table");
                }
            }// if (PathIsDrive...
            else
            {
                if (String.Equals(type, "table", StringComparison.OrdinalIgnoreCase))
                {
                    if (rowNumber < 0)
                    {
                        throw new
                            ArgumentException("A table cannot be created within another table, specify a path that represents a database");
                    }
                    else
                    {
                        throw new
                            ArgumentException("A table cannot be created inside a row, specify a path that represents a database");
                    }
                } //if (String.Equals....
                // if path specified is a row, create a new row
                else if (String.Equals(type, "row", StringComparison.OrdinalIgnoreCase))
                {
                    // The user is required to specify the values to be inserted 
                    // into the table in a single string separated by commas
                    string value = newItemValue as string;

                    if (String.IsNullOrEmpty(value))
                    {
                        throw new
                            ArgumentException("Value argument must have comma separated values of each column in a row");
                    }
                    string[] rowValues = value.Split(',');

                    OdbcDataAdapter da = GetAdapterForTable(tableName);

                    if (da == null)
                    {
                        return;
                    }

                    DataSet ds = GetDataSetForTable(da, tableName);
                    DataTable table = GetDataTable(ds, tableName);

                    if (rowValues.Length != table.Columns.Count)
                    {
                        string message = String.Format(CultureInfo.CurrentCulture,
                                            "The table has {0} columns and the value specified must have so many comma separated values",
                                                table.Columns.Count);

                        throw new ArgumentException(message);
                    }

                    if (!Force && (rowNumber >= 0 && rowNumber < table.Rows.Count))
                    {
                        string message = String.Format(CultureInfo.CurrentCulture, 
                                            "The row {0} already exists. To create a new row specify row number as {1}, or specify path to a table, or use the -Force parameter",
                                                rowNumber, table.Rows.Count);

                        throw new ArgumentException(message);
                    }

                    if (rowNumber > table.Rows.Count)
                    {
                        string message = String.Format(CultureInfo.CurrentCulture, 
                                            "To create a new row specify row number as {0}, or specify path to a table",
                                                 table.Rows.Count);

                        throw new ArgumentException(message);
                    }

                    // Create a new row and update the row with the input
                    // provided by the user
                    DataRow row = table.NewRow();
                    for (int i = 0; i < rowValues.Length; i++)
                    {
                        row[i] = rowValues[i];
                    }
                    table.Rows.Add(row);

                    if (ShouldProcess(tableName, "update rows"))
                    {
                        // Update the table from memory back to the data source
                        da.Update(ds, tableName);
                    }

                }// else if (String...
            }// else ...

        } // NewItem

        /// <summary>
        /// Copies an item at the specified path to the location specified
        /// </summary>
        /// 
        /// <param name="path">
        /// Path of the item to copy
        /// </param>
        /// 
        /// <param name="copyPath">
        /// Path of the item to copy to
        /// </param>
        /// 
        /// <param name="recurse">
        /// Tells the provider to recurse subcontainers when copying
        /// </param>
        /// 
        protected override void CopyItem(string path, string copyPath, bool recurse)
        {
            string tableName, copyTableName;
            int rowNumber, copyRowNumber;

            PathType type = GetNamesFromPath(path, out tableName, out rowNumber);
            PathType copyType = GetNamesFromPath(copyPath, out copyTableName, out copyRowNumber);

            if (type == PathType.Invalid)
            {
                ThrowTerminatingInvalidPathException(path);
            }

            if (type == PathType.Invalid)
            {
                ThrowTerminatingInvalidPathException(copyPath);
            }

            // Get the table and the table to copy to 
            OdbcDataAdapter da = GetAdapterForTable(tableName);
            if (da == null)
            {
                return;
            }

            DataSet ds = GetDataSetForTable(da, tableName);
            DataTable table = GetDataTable(ds, tableName);

            OdbcDataAdapter cda = GetAdapterForTable(copyTableName);
            if (cda == null)
            {
                return;
            }

            DataSet cds = GetDataSetForTable(cda, copyTableName);
            DataTable copyTable = GetDataTable(cds, copyTableName);

            // if source represents a table
            if (type == PathType.Table)
            {
                // if copyPath does not represent a table
                if (copyType != PathType.Table)
                {
                    ArgumentException e = new ArgumentException("Table can only be copied on to another table location");

                    WriteError(new ErrorRecord(e, "PathNotValid",
                        ErrorCategory.InvalidArgument, copyPath));

                    throw e;
                }

                // if table already exists then force parameter should be set 
                // to force a copy
                if (!Force && GetTable(copyTableName) != null)
                {
                    throw new ArgumentException("Specified path already exists");
                }

                for (int i = 0; i < table.Rows.Count; i++)
                {
                    DataRow row = table.Rows[i];
                    DataRow copyRow = copyTable.NewRow();

                    copyRow.ItemArray = row.ItemArray;
                    copyTable.Rows.Add(copyRow);
                }
            } // if (type == ...
            // if source represents a row
            else
            {
                if (copyType == PathType.Row)
                {
                    if (!Force && (copyRowNumber < copyTable.Rows.Count))
                    {
                        throw new ArgumentException("Specified path already exists.");
                    }

                    DataRow row = table.Rows[rowNumber];
                    DataRow copyRow = null;

                    if (copyRowNumber < copyTable.Rows.Count)
                    {
                        // copy to an existing row
                        copyRow = copyTable.Rows[copyRowNumber];
                        copyRow.ItemArray = row.ItemArray;
                        copyRow[0] = GetNextID(copyTable);
                    }
                    else if (copyRowNumber == copyTable.Rows.Count)
                    {
                        // copy to the next row in the table that will 
                        // be created
                        copyRow = copyTable.NewRow();
                        copyRow.ItemArray = row.ItemArray;
                        copyRow[0] = GetNextID(copyTable);
                        copyTable.Rows.Add(copyRow);
                    }
                    else
                    {
                        // attempting to copy to a nonexistent row or a row
                        // that cannot be created now - throw an exception
                        string message = String.Format(CultureInfo.CurrentCulture, 
                                            "The item cannot be specified to the copied row. Specify row number as {0}, or specify a path to the table.",
                                                table.Rows.Count);

                        throw new ArgumentException(message);
                    }
                }
                else
                {
                    // destination path specified represents a table, 
                    // create a new row and copy the item
                    DataRow copyRow = copyTable.NewRow();
                    copyRow.ItemArray = table.Rows[rowNumber].ItemArray;
                    copyRow[0] = GetNextID(copyTable);
                    copyTable.Rows.Add(copyRow);
                }
            }

            if (ShouldProcess(copyTableName, "CopyItems"))
            {
                cda.Update(cds, copyTableName);
            }

        } //CopyItem

        /// <summary>
        /// Removes (deletes) the item at the specified path
        /// </summary>
        /// 
        /// <param name="path">
        /// The path to the item to remove.
        /// </param>
        /// 
        /// <param name="recurse">
        /// True if all children in a subtree should be removed, false if only
        /// the item at the specified path should be removed. Is applicable
        /// only for container (table) items. Its ignored otherwise (even if
        /// specified).
        /// </param>
        /// 
        /// <remarks>
        /// There are no elements in this store which are hidden from the user.
        /// Hence this method will not check for the presence of the Force
        /// parameter
        /// </remarks>
        /// 
        protected override void RemoveItem(string path, bool recurse)
        {
            string tableName;
            int rowNumber = 0;

            PathType type = GetNamesFromPath(path, out tableName, out rowNumber);

            if (type == PathType.Table)
            {
                // if recurse flag has been specified, delete all the rows as well
                if (recurse)
                {
                    OdbcDataAdapter da = GetAdapterForTable(tableName);
                    if (da == null)
                    {
                        return;
                    }

                    DataSet ds = GetDataSetForTable(da, tableName);
                    DataTable table = GetDataTable(ds, tableName);

                    for (int i = 0; i < table.Rows.Count; i++)
                    {
                        table.Rows[i].Delete();
                    }

                    if (ShouldProcess(path, "RemoveItem"))
                    {
                        da.Update(ds, tableName);
                        RemoveTable(tableName);
                    }
                }//if (recurse...
                else
                {
                    // Remove the table
                    if (ShouldProcess(path, "RemoveItem"))
                    {
                        RemoveTable(tableName);
                    }
                }
            }
            else if (type == PathType.Row)
            {
                OdbcDataAdapter da = GetAdapterForTable(tableName);
                if (da == null)
                {
                    return;
                }

                DataSet ds = GetDataSetForTable(da, tableName);
                DataTable table = GetDataTable(ds, tableName);

                table.Rows[rowNumber].Delete();

                if (ShouldProcess(path, "RemoveItem"))
                {
                    da.Update(ds, tableName);
                }
            }
            else
            {
                ThrowTerminatingInvalidPathException(path);
            }

        } // RemoveItem

        #endregion Container Overloads

       #region Navigation

        /// <summary>
        /// Determine if the path specified is that of a container.
        /// </summary>
        /// <param name="path">The path to check.</param>
        /// <returns>True if the path specifies a container.</returns>
        protected override bool IsItemContainer(string path)
        {
            if (PathIsDrive(path))
            {
                return true;
            }

            string[] pathChunks = ChunkPath(path);
            string tableName;
            int rowNumber;

            PathType type = GetNamesFromPath(path, out tableName, out rowNumber);

            if (type == PathType.Table)
            {
                foreach (DatabaseTableInfo ti in GetTables())
                {
                    if (string.Equals(ti.Name, tableName, StringComparison.OrdinalIgnoreCase))
                    {
                        return true;
                    }
                } // foreach (DatabaseTableInfo...
            } // if (pathChunks...

            return false;
        } // IsItemContainer

        /// <summary>
        /// Get the name of the leaf element in the specified path        
        /// </summary>
        /// 
        /// <param name="path">
        /// The full or partial provider specific path
        /// </param>
        /// 
        /// <returns>
        /// The leaf element in the path
        /// </returns>
        protected override string GetChildName(string path)
        {
            if (PathIsDrive(path))
            {
                return path;
            }

            string tableName;
            int rowNumber;

            PathType type = GetNamesFromPath(path, out tableName, out rowNumber);

            if (type == PathType.Table)
            {
                return tableName;
            }
            else if (type == PathType.Row)
            {
                return rowNumber.ToString(CultureInfo.CurrentCulture);
            }
            else
            {
                ThrowTerminatingInvalidPathException(path);
            }

            return null;
        }

        /// <summary>
        /// Removes the child segment of the path and returns the remaining
        /// parent portion
        /// </summary>
        /// 
        /// <param name="path">
        /// A full or partial provider specific path. The path may be to an
        /// item that may or may not exist.
        /// </param>
        /// 
        /// <param name="root">
        /// The fully qualified path to the root of a drive. This parameter
        /// may be null or empty if a mounted drive is not in use for this
        /// operation.  If this parameter is not null or empty the result
        /// of the method should not be a path to a container that is a
        /// parent or in a different tree than the root.
        /// </param>
        /// 
        /// <returns></returns>

        protected override string GetParentPath(string path, string root)
        {
            // If root is specified then the path has to contain
            // the root. If not nothing should be returned
            if (!String.IsNullOrEmpty(root))
            {
                if (!path.Contains(root))
                {
                    return null;
                }
            }

            return path.Substring(0, path.LastIndexOf(pathSeparator, StringComparison.OrdinalIgnoreCase));
        }

        /// <summary>
        /// Joins two strings with a provider specific path separator.
        /// </summary>
        /// 
        /// <param name="parent">
        /// The parent segment of a path to be joined with the child.
        /// </param>
        /// 
        /// <param name="child">
        /// The child segment of a path to be joined with the parent.
        /// </param>
        /// 
        /// <returns>
        /// A string that represents the parent and child segments of the path
        /// joined by a path separator.
        /// </returns>

        protected override string MakePath(string parent, string child)
        {
            string result;

            string normalParent = NormalizePath(parent);
            normalParent = RemoveDriveFromPath(normalParent);
            string normalChild = NormalizePath(child);
            normalChild = RemoveDriveFromPath(normalChild);

            if (String.IsNullOrEmpty(normalParent) && String.IsNullOrEmpty(normalChild))
            {
                result = String.Empty;
            }
            else if (String.IsNullOrEmpty(normalParent) && !String.IsNullOrEmpty(normalChild))
            {
                result = normalChild;
            }
            else if (!String.IsNullOrEmpty(normalParent) && String.IsNullOrEmpty(normalChild))
            {
                if (normalParent.EndsWith(pathSeparator, StringComparison.OrdinalIgnoreCase))
                {
                    result = normalParent;
                }
                else
                {
                    result = normalParent + pathSeparator;
                }
            } // else if (!String...
            else
            {
                if (!normalParent.Equals(String.Empty, StringComparison.OrdinalIgnoreCase) && 
                    !normalParent.EndsWith(pathSeparator, StringComparison.OrdinalIgnoreCase))
                {
                    result = normalParent + pathSeparator;
                }
                else
                {
                    result = normalParent;
                }

                if (normalChild.StartsWith(pathSeparator, StringComparison.OrdinalIgnoreCase))
                {
                    result += normalChild.Substring(1);
                }
                else
                {
                    result += normalChild;
                }
            } // else

            return result;
        } // MakePath

        /// <summary>
        /// Normalizes the path that was passed in and returns the normalized
        /// path as a relative path to the basePath that was passed.
        /// </summary>
        /// 
        /// <param name="path">
        /// A fully qualified provider specific path to an item.  The item
        /// should exist or the provider should write out an error.
        /// </param>
        /// 
        /// <param name="basepath">
        /// The path that the return value should be relative to.
        /// </param>
        /// 
        /// <returns>
        /// A normalized path that is relative to the basePath that was
        /// passed.  The provider should parse the path parameter, normalize
        /// the path, and then return the normalized path relative to the
        /// basePath.
        /// </returns>

        protected override string NormalizeRelativePath(string path,
                                                             string basepath)
        {
            // Normalize the paths first
            string normalPath = NormalizePath(path);
            normalPath = RemoveDriveFromPath(normalPath);
            string normalBasePath = NormalizePath(basepath);
            normalBasePath = RemoveDriveFromPath(normalBasePath);

            if (String.IsNullOrEmpty(normalBasePath))
            {
                return normalPath;
            }
            else
            {
                if (!normalPath.Contains(normalBasePath))
                {
                    return null;
                }

                return normalPath.Substring(normalBasePath.Length + pathSeparator.Length);
            }
        }

        /// <summary>
        /// Moves the item specified by the path to the specified destination
        /// </summary>
        /// 
        /// <param name="path">
        /// The path to the item to be moved
        /// </param>
        /// 
        /// <param name="destination">
        /// The path of the destination container
        /// </param>

        protected override void MoveItem(string path, string destination)
        {
            // Get type, table name and rowNumber from the path
            string tableName, destTableName;
            int rowNumber, destRowNumber;

            PathType type = GetNamesFromPath(path, out tableName, out rowNumber);

            PathType destType = GetNamesFromPath(destination, out destTableName,
                                     out destRowNumber);

            if (type == PathType.Invalid)
            {
                ThrowTerminatingInvalidPathException(path);
            }

            if (destType == PathType.Invalid)
            {
                ThrowTerminatingInvalidPathException(destination);
            }

            if (type == PathType.Table)
            {
                ArgumentException e = new ArgumentException("Move not supported for tables");

                WriteError(new ErrorRecord(e, "MoveNotSupported",
                    ErrorCategory.InvalidArgument, path));

                throw e;
            }
            else
            {
                OdbcDataAdapter da = GetAdapterForTable(tableName);
                if (da == null)
                {
                    return;
                }

                DataSet ds = GetDataSetForTable(da, tableName);
                DataTable table = GetDataTable(ds, tableName);

                OdbcDataAdapter dda = GetAdapterForTable(destTableName);
                if (dda == null)
                {
                    return;
                }

                DataSet dds = GetDataSetForTable(dda, destTableName);
                DataTable destTable = GetDataTable(dds, destTableName);
                DataRow row = table.Rows[rowNumber];

                if (destType == PathType.Table)
                {
                    DataRow destRow = destTable.NewRow();

                    destRow.ItemArray = row.ItemArray;
                }
                else
                {
                    DataRow destRow = destTable.Rows[destRowNumber];

                    destRow.ItemArray = row.ItemArray;
                }

                // Update the changes
                if (ShouldProcess(path, "MoveItem"))
                {
                    WriteItemObject(row, path, false);
                    dda.Update(dds, destTableName);
                }
            }
        }

        #endregion Navigation

       #region Helper Methods

        /// <summary>
        /// Checks if a given path is actually a drive name.
        /// </summary>
        /// <param name="path">The path to check.</param>
        /// <returns>
        /// True if the path given represents a drive, false otherwise.
        /// </returns>
        private bool PathIsDrive(string path)
        {
            // Remove the drive name and first path separator.  If the 
            // path is reduced to nothing, it is a drive. Also if its
            // just a drive then there wont be any path separators
            if (String.IsNullOrEmpty(
                        path.Replace(this.PSDriveInfo.Root, "")) ||
                String.IsNullOrEmpty(
                        path.Replace(this.PSDriveInfo.Root + pathSeparator, ""))

               )
            {
                return true;
            }
            else
            {
                return false;
            }
        } // PathIsDrive

        /// <summary>
        /// Breaks up the path into individual elements.
        /// </summary>
        /// <param name="path">The path to split.</param>
        /// <returns>An array of path segments.</returns>
        private string[] ChunkPath(string path)
        {
            // Normalize the path before splitting
            string normalPath = NormalizePath(path);

            // Return the path with the drive name and first path 
            // separator character removed, split by the path separator.
            string pathNoDrive = normalPath.Replace(this.PSDriveInfo.Root
                                           + pathSeparator, "");

            return pathNoDrive.Split(pathSeparator.ToCharArray());
        } // ChunkPath

        /// <summary>
        /// Adapts the path, making sure the correct path separator
        /// character is used.
        /// </summary>
        /// <param name="path"></param>
        /// <returns></returns>
        private string NormalizePath(string path)
        {
            string result = path;

            if (!String.IsNullOrEmpty(path))
            {
                result = path.Replace("/", pathSeparator);
            }

            return result;
        } // NormalizePath

        /// <summary>
        /// Ensures that the drive is removed from the specified path
        /// </summary>
        /// 
        /// <param name="path">Path from which drive needs to be removed</param>
        /// <returns>Path with drive information removed</returns>
        private string RemoveDriveFromPath(string path)
        {
            string result = path;
            string root;

            if (this.PSDriveInfo == null)
            {
                root = String.Empty;
            }
            else
            {
                root = this.PSDriveInfo.Root;
            }

            if (result == null)
            {
                result = String.Empty;
            }

            if (result.Contains(root))
            {
                result = result.Substring(result.IndexOf(root, StringComparison.OrdinalIgnoreCase) + root.Length);
            }

            return result;
        }

        /// <summary>
        /// Chunks the path and returns the table name and the row number 
        /// from the path
        /// </summary>
        /// <param name="path">Path to chunk and obtain information</param>
        /// <param name="tableName">Name of the table as represented in the 
        /// path</param>
        /// <param name="rowNumber">Row number obtained from the path</param>
        /// <returns>what the path represents</returns>
        public PathType GetNamesFromPath(string path, out string tableName, out int rowNumber)
        {
            PathType retVal = PathType.Invalid;
            rowNumber = -1;
            tableName = null;

            // Check if the path specified is a drive
            if (PathIsDrive(path))
            {
                return PathType.Database;
            }

            // chunk the path into parts
            string[] pathChunks = ChunkPath(path);

            switch (pathChunks.Length)
            {
                case 1:
                    {
                        string name = pathChunks[0];

                        if (TableNameIsValid(name))
                        {
                            tableName = name;
                            retVal = PathType.Table;
                        }
                    }
                    break;

                case 2:
                    {
                        string name = pathChunks[0];

                        if (TableNameIsValid(name))
                        {
                            tableName = name;
                        }

                        int number = SafeConvertRowNumber(pathChunks[1]);

                        if (number >= 0)
                        {
                            rowNumber = number;
                            retVal = PathType.Row;
                        }
                        else
                        {
                            WriteError(new ErrorRecord(
                                new ArgumentException("Row number is not valid"),
                                "RowNumberNotValid",
                                ErrorCategory.InvalidArgument,
                                path));
                        }
                    }
                    break;

                default:
                    {
                        WriteError(new ErrorRecord(
                            new ArgumentException("The path supplied has too many segments"),
                            "PathNotValid",
                            ErrorCategory.InvalidArgument,
                            path));
                    }
                    break;
            } // switch(pathChunks...

            return retVal;
        } // GetNamesFromPath

        /// <summary>
        /// Throws an argument exception stating that the specified path does
        /// not represent either a table or a row
        /// </summary>
        /// <param name="path">path which is invalid</param>
        private void ThrowTerminatingInvalidPathException(string path)
        {
            StringBuilder message = new StringBuilder("Path must represent either a table or a row :");
            message.Append(path);

            throw new ArgumentException(message.ToString());
        }

        /// <summary>
        /// Retrieve the list of tables from the database.
        /// </summary>
        /// <returns>
        /// Collection of DatabaseTableInfo objects, each object representing
        /// information about one database table
        /// </returns>
        internal Collection<DatabaseTableInfo> GetTables()
        {
            Collection<DatabaseTableInfo> results =
                    new Collection<DatabaseTableInfo>();

            // using ODBC connection to the database and get the schema of tables
            AccessDBPSDriveInfo di = this.PSDriveInfo as AccessDBPSDriveInfo;

            if (di == null)
            {
                return null;
            }

            OdbcConnection connection = di.Connection;
            DataTable dt = connection.GetSchema("Tables");
            int count;

            // iterate through all rows in the schema and create DatabaseTableInfo
            // objects which represents a table
            foreach (DataRow dr in dt.Rows)
            {
                String tableName = dr["TABLE_NAME"] as String;
                DataColumnCollection columns = null;

                // find the number of rows in the table
                try
                {
                    String cmd = "Select count(*) from \"" + tableName + "\"";
                    OdbcCommand command = new OdbcCommand(cmd, connection);

                    count = (Int32)command.ExecuteScalar();
                }
                catch
                {
                    count = 0;
                }

                // create DatabaseTableInfo object representing the table
                DatabaseTableInfo table =
                        new DatabaseTableInfo(dr, tableName, count, columns);

                results.Add(table);
            } // foreach (DataRow...

            return results;
        } // GetTables

        /// <summary>
        /// Return row information from a specified table.
        /// </summary>
        /// <param name="tableName">The name of the database table from 
        /// which to retrieve rows.</param>
        /// <returns>Collection of row information objects.</returns>
        public Collection<DatabaseRowInfo> GetRows(string tableName)
        {
            Collection<DatabaseRowInfo> results =
                        new Collection<DatabaseRowInfo>();

            // Obtain rows in the table and add it to the collection
            try
            {
                OdbcDataAdapter da = GetAdapterForTable(tableName);

                if (da == null)
                {
                    return null;
                }

                DataSet ds = GetDataSetForTable(da, tableName);
                DataTable table = GetDataTable(ds, tableName);

                int i = 0;
                foreach (DataRow row in table.Rows)
                {
                    results.Add(new DatabaseRowInfo(row, i.ToString(CultureInfo.CurrentCulture)));
                    i++;
                } // foreach (DataRow...
            }
            catch (Exception e)
            {
                WriteError(new ErrorRecord(e, "CannotAccessSpecifiedRows",
                    ErrorCategory.InvalidOperation, tableName));
            }

            return results;

        } // GetRows

        /// <summary>
        /// Retrieve information about a single table.
        /// </summary>
        /// <param name="tableName">The table for which to retrieve 
        /// data.</param>
        /// <returns>Table information.</returns>
        private DatabaseTableInfo GetTable(string tableName)
        {
            foreach (DatabaseTableInfo table in GetTables())
            {
                if (String.Equals(tableName, table.Name, StringComparison.OrdinalIgnoreCase))
                {
                    return table;
                }
            }

            return null;
        } // GetTable

        /// <summary>
        /// Removes the specified table from the database
        /// </summary>
        /// <param name="tableName">Name of the table to remove</param>
        private void RemoveTable(string tableName)
        {
            // validate if tablename is valid and if table is present
            if (String.IsNullOrEmpty(tableName) || !TableNameIsValid(tableName) || !TableIsPresent(tableName))
            {
                return;
            }

            // Execute command using ODBC connection to remove a table
            try
            {
                // delete the table using an sql statement
                string sql = "drop table " + tableName;

                AccessDBPSDriveInfo di = this.PSDriveInfo as AccessDBPSDriveInfo;

                if (di == null)
                {
                    return;
                }
                OdbcConnection connection = di.Connection;

                OdbcCommand cmd = new OdbcCommand(sql, connection);
                cmd.ExecuteScalar();
            }
            catch (Exception ex)
            {
                WriteError(new ErrorRecord(ex, "CannotRemoveSpecifiedTable",
                          ErrorCategory.InvalidOperation, null)
                          );
            }

        } // RemoveTable

        /// <summary>
        /// Obtain a data adapter for the specified Table
        /// </summary>
        /// <param name="tableName">Name of the table to obtain the 
        /// adapter for</param>
        /// <returns>Adapter object for the specified table</returns>
        /// <remarks>An adapter serves as a bridge between a DataSet (in memory
        /// representation of table) and the data source</remarks>
        internal OdbcDataAdapter GetAdapterForTable(string tableName)
        {
            OdbcDataAdapter da = null;
            AccessDBPSDriveInfo di = this.PSDriveInfo as AccessDBPSDriveInfo;

            if (di == null || !TableNameIsValid(tableName) || !TableIsPresent(tableName))
            {
                return null;
            }

            OdbcConnection connection = di.Connection;

            try
            {
                // Create a odbc data adpater. This can be sued to update the
                // data source with the records that will be created here
                // using data sets
                string sql = "Select * from " + tableName;
                da = new OdbcDataAdapter(new OdbcCommand(sql, connection));

                // Create a odbc command builder object. This will create sql
                // commands automatically for a single table, thus
                // eliminating the need to create new sql statements for 
                // every operation to be done.
                OdbcCommandBuilder cmd = new OdbcCommandBuilder(da);

                // Set the delete cmd for the table here
                sql = "Delete from " + tableName + " where ID = ?";
                da.DeleteCommand = new OdbcCommand(sql, connection);

                // Specify a DeleteCommand parameter based on the "ID" 
                // column
                da.DeleteCommand.Parameters.Add(new OdbcParameter());
                da.DeleteCommand.Parameters[0].SourceColumn = "ID";

                // Create an InsertCommand based on the sql string
                // Insert into "tablename" values (?,?,?)" where
                // ? represents a column in the table. Note that 
                // the number of ? will be equal to the number of 
                // columnds
                DataSet ds = new DataSet();
                ds.Locale = CultureInfo.InvariantCulture;

                da.FillSchema(ds, SchemaType.Source);

                sql = "Insert into " + tableName + " values ( ";
                for (int i = 0; i < ds.Tables["Table"].Columns.Count; i++)
                {
                    sql += "?, ";
                }
                sql = sql.Substring(0, sql.Length - 2);
                sql += ")";
                da.InsertCommand = new OdbcCommand(sql, connection);

                // Create parameters for the InsertCommand based on the
                // captions of each column
                for (int i = 0; i < ds.Tables["Table"].Columns.Count; i++)
                {
                    da.InsertCommand.Parameters.Add(new OdbcParameter());
                    da.InsertCommand.Parameters[i].SourceColumn =
                                     ds.Tables["Table"].Columns[i].Caption;

                }

                // Open the connection if its not already open                 
                if (connection.State != ConnectionState.Open)
                {
                    connection.Open();
                }
            }
            catch (Exception e)
            {
                WriteError(new ErrorRecord(e, "CannotAccessSpecifiedTable",
                  ErrorCategory.InvalidOperation, tableName));
            }

            return da;
        } // GetAdapterForTable

        /// <summary>
        /// Gets the DataSet (in memory representation) for the table
        /// for the specified adapter
        /// </summary>
        /// <param name="adapter">Adapter to be used for obtaining 
        /// the table</param>
        /// <param name="tableName">Name of the table for which a 
        /// DataSet is required</param>
        /// <returns>The DataSet with the filled in schema</returns>
        internal DataSet GetDataSetForTable(OdbcDataAdapter adapter, string tableName)
        {
            Debug.Assert(adapter != null);

            // Create a dataset object which will provide an in-memory
            // representation of the data being worked upon in the 
            // data source. 
            DataSet ds = new DataSet();

            // Create a table named "Table" which will contain the same
            // schema as in the data source.
            //adapter.FillSchema(ds, SchemaType.Source);
            adapter.Fill(ds, tableName);
            ds.Locale = CultureInfo.InvariantCulture;

            return ds;
        } //GetDataSetForTable

        /// <summary>
        /// Get the DataTable object which can be used to operate on
        /// for the specified table in the data source
        /// </summary>
        /// <param name="ds">DataSet object which contains the tables
        /// schema</param>
        /// <param name="tableName">Name of the table</param>
        /// <returns>Corresponding DataTable object representing 
        /// the table</returns>
        /// 
        internal DataTable GetDataTable(DataSet ds, string tableName)
        {
            Debug.Assert(ds != null);
            Debug.Assert(tableName != null);

            DataTable table = ds.Tables[tableName];
            table.Locale = CultureInfo.InvariantCulture;

            return table;
        } // GetDataTable

       /// <summary>
        /// Retrieves a single row from the named table.
        /// </summary>
        /// <param name="tableName">The table that contains the 
        /// numbered row.</param>
        /// <param name="row">The index of the row to return.</param>
        /// <returns>The specified table row.</returns>
        private DatabaseRowInfo GetRow(string tableName, int row)
        {
            Collection<DatabaseRowInfo> di = GetRows(tableName);

            // if the row is invalid write an appropriate error else return the 
            // corresponding row information
            if (row < di.Count && row >= 0)
            {
                return di[row];
            }
            else
            {
                WriteError(new ErrorRecord(
                   new ItemNotFoundException(),
                   "RowNotFound",
                   ErrorCategory.ObjectNotFound,
                   row.ToString(CultureInfo.CurrentCulture))
                );
            }

            return null;
        } // GetRow

       /// <summary>
        /// Method to safely convert a string representation of a row number 
        /// into its Int32 equivalent
        /// </summary>
        /// <param name="rowNumberAsStr">String representation of the row 
        /// number</param>
        /// <remarks>If there is an exception, -1 is returned</remarks>
       private int SafeConvertRowNumber(string rowNumberAsStr)
        {
            int rowNumber = -1;
            try
            {
                rowNumber = Convert.ToInt32(rowNumberAsStr, CultureInfo.CurrentCulture);
            }
            catch (FormatException fe)
            {
                WriteError(new ErrorRecord(fe, "RowStringFormatNotValid",
                    ErrorCategory.InvalidData, rowNumberAsStr));
            }
            catch (OverflowException oe)
            {
                WriteError(new ErrorRecord(oe, "RowStringConversionToNumberFailed",
                    ErrorCategory.InvalidData, rowNumberAsStr));
            }

            return rowNumber;
        } // 1


       /// <summary>
       /// Check if a table name is valid
       /// </summary>
       /// <param name="tableName">Table name to validate</param>
       /// <remarks>Helps to check for SQL injection attacks</remarks>
       private bool TableNameIsValid(string tableName)
       {
           Regex exp = new Regex(pattern, RegexOptions.Compiled | RegexOptions.IgnoreCase);

           if (exp.IsMatch(tableName))
           {
               return true;
           }
           WriteError(new ErrorRecord(
               new ArgumentException("Table name not valid"), "TableNameNotValid",
                   ErrorCategory.InvalidArgument, tableName));
           return false;
       } // TableNameIsValid

       /// <summary>
       /// Checks to see if the specified table is present in the
       /// database
       /// </summary>
       /// <param name="tableName">Name of the table to check</param>
       /// <returns>true, if table is present, false otherwise</returns>
       private bool TableIsPresent(string tableName)
       {
           // using ODBC connection to the database and get the schema of tables
           AccessDBPSDriveInfo di = this.PSDriveInfo as AccessDBPSDriveInfo;
           if (di == null)
           {
               return false;
           }

           OdbcConnection connection = di.Connection;
           DataTable dt = connection.GetSchema("Tables");

           // check if the specified tableName is available
           // in the list of tables present in the database
           foreach (DataRow dr in dt.Rows)
           {
               string name = dr["TABLE_NAME"] as string;
               if (name.Equals(tableName, StringComparison.OrdinalIgnoreCase))
               {
                   return true;
               }
           }

           WriteError(new ErrorRecord(
               new ArgumentException("Specified Table is not present in database"), "TableNotAvailable",
                    ErrorCategory.InvalidArgument, tableName));

           return false;
       }// TableIsPresent

       /// <summary>
       /// Gets the next available ID in the table
       /// </summary>
       /// <param name="table">DataTable object representing the table to 
       /// search for ID</param>
       /// <returns>next available id</returns>
       private int GetNextID(DataTable table)
       {
           int big = 0;

           for (int i = 0; i < table.Rows.Count; i++)
           {
               DataRow row = table.Rows[i];

               int id = (int)row["ID"];

               if (big < id)
               {
                   big = id;
               }
           }

           big++;
           return big;
       }
       #endregion Helper Methods

       #region Content Methods

       /// <summary>
       /// Clear the contents at the specified location. In this case, clearing
       /// the item amounts to clearing a row
       /// </summary>
       /// <param name="path">The path to the content to clear.</param>
       public void ClearContent(string path)
       {
           string tableName;
           int rowNumber;

           PathType type = GetNamesFromPath(path, out tableName, out rowNumber);

           if (type != PathType.Table)
           {
               WriteError(new ErrorRecord(
                   new InvalidOperationException("Operation not supported. Content can be cleared only for table"),
                       "NotValidRow", ErrorCategory.InvalidArgument,
                           path));
               return;
           }

           OdbcDataAdapter da = GetAdapterForTable(tableName);

           if (da == null)
           {
               return;
           }

           DataSet ds = GetDataSetForTable(da, tableName);
           DataTable table = GetDataTable(ds, tableName);

           // Clear contents at the specified location
           for (int i = 0; i < table.Rows.Count; i++)
           {
               table.Rows[i].Delete();
           }

           if (ShouldProcess(path, "ClearContent"))
           {
               da.Update(ds, tableName);
           }

       } // ClearContent

       /// <summary>
       /// Not implemented.
       /// </summary>
       /// <param name="path"></param>
       /// <returns></returns>
       public object ClearContentDynamicParameters(string path)
       {
           return null;
       }

       /// <summary>
       /// Get a reader at the path specified.
       /// </summary>
       /// <param name="path">The path from which to read.</param>
       /// <returns>A content reader used to read the data.</returns>
       public IContentReader GetContentReader(string path)
       {
           string tableName;
           int rowNumber;

           PathType type = GetNamesFromPath(path, out tableName, out rowNumber);

           if (type == PathType.Invalid)
           {
               ThrowTerminatingInvalidPathException(path);
           }
           else if (type == PathType.Row)
           {
               throw new InvalidOperationException("contents can be obtained only for tables");
           }

           return new AccessDBContentReader(path, this);
       } // GetContentReader

       /// <summary>
       /// Not implemented.
       /// </summary>
       /// <param name="path"></param>
       /// <returns></returns>
       public object GetContentReaderDynamicParameters(string path)
       {
           return null;
       }

       /// <summary>
       /// Get an object used to write content.
       /// </summary>
       /// <param name="path">The root path at which to write.</param>
       /// <returns>A content writer for writing.</returns>
       public IContentWriter GetContentWriter(string path)
       {
           string tableName;
           int rowNumber;

           PathType type = GetNamesFromPath(path, out tableName, out rowNumber);

           if (type == PathType.Invalid)
           {
               ThrowTerminatingInvalidPathException(path);
           }
           else if (type == PathType.Row)
           {
               throw new InvalidOperationException("contents can be added only to tables");
           }

           return new AccessDBContentWriter(path, this);
       }

       /// <summary>
       /// Not implemented.
       /// </summary>
       /// <param name="path"></param>
       /// <returns></returns>
       public object GetContentWriterDynamicParameters(string path)
       {
           return null;
       }

       #endregion Content Methods

       #region Private Properties
      
       private string pathSeparator = "\\";
       private static string pattern = @"^[a-z]+[0-9]*_*$";

       #endregion Private Properties

   } // AccessDBProvider

   #endregion AccessDBProvider

    #region Helper Classes

   #region Public Enumerations

   /// <summary>
   /// Type of item represented by the path
   /// </summary>
   public enum PathType
   {
       /// <summary>
       /// Represents a database
       /// </summary>
       Database,
       /// <summary>
       /// Represents a table
       /// </summary>
       Table,
       /// <summary>
       /// Represents a row
       /// </summary>
       Row,
       /// <summary>
       /// Represents an invalid path
       /// </summary>
       Invalid
   };

   #endregion Public Enumerations

    #region AccessDBPSDriveInfo

    /// <summary>
    /// Any state associated with the drive should be held here.
    /// In this case, it's the connection to the database.
    /// </summary>
    internal class AccessDBPSDriveInfo : PSDriveInfo
    {
        private OdbcConnection connection;

        /// <summary>
        /// ODBC connection information.
        /// </summary>
        public OdbcConnection Connection
        {
            get { return connection; }
            set { connection = value; }
        }

        /// <summary>
        /// Constructor that takes one argument
        /// </summary>
        /// <param name="driveInfo">Drive provided by this provider</param>
        public AccessDBPSDriveInfo(PSDriveInfo driveInfo)
            : base(driveInfo)
        { }

    } // class AccessDBPSDriveInfo

    #endregion AccessDBPSDriveInfo

    #region DatabaseTableInfo

    /// <summary>
    /// Contains information specific to the database table.
    /// Similar to the DirectoryInfo class.
    /// </summary>
    public class DatabaseTableInfo
    {
        /// <summary>
        /// Row from the "tables" schema
        /// </summary>
        public DataRow Data
        {
            get
            {
                return data;
            }
            set
            {
                data = value;
            }
        }
        private DataRow data;

        /// <summary>
        /// The table name.
        /// </summary>
        public string Name
        {
            get
            {
                return name;
            }
            set
            {
                name = value;
            }
        }
        private String name;

        /// <summary>
        /// The number of rows in the table.
        /// </summary>
        public int RowCount
        {
            get
            {
                return rowCount;
            }
            set
            {
                rowCount = value;
            }
        }
        private int rowCount;

        /// <summary>
        /// The column definitions for the table.
        /// </summary>
        public DataColumnCollection Columns
        {
            get
            {
                return columns;
            }
            set
            {
                columns = value;
            }
        }
        private DataColumnCollection columns;

        /// <summary>
        /// Constructor.
        /// </summary>
        /// <param name="row">The row definition.</param>
        /// <param name="name">The table name.</param>
        /// <param name="rowCount">The number of rows in the table.</param>
        /// <param name="columns">Information on the column tables.</param>
        public DatabaseTableInfo(DataRow row, string name, int rowCount,
                       DataColumnCollection columns)
        {
            Name = name;
            Data = row;
            RowCount = rowCount;
            Columns = columns;
        } // DatabaseTableInfo
    } // class DatabaseTableInfo

    #endregion DatabaseTableInfo

    #region DatabaseRowInfo

    /// <summary>
    /// Contains information specific to an individual table row.
    /// Analogous to the FileInfo class.
    /// </summary>
    public class DatabaseRowInfo
    {
        /// <summary>
        /// Row data information.
        /// </summary>
        public DataRow Data
        {
            get
            {
                return data;
            }
            set
            {
                data = value;
            }
        }
        private DataRow data;

        /// <summary>
        /// The row index.
        /// </summary>
        public string RowNumber
        {
            get
            {
                return rowNumber;
            }
            set
            {
                rowNumber = value;
            }
        }
        private string rowNumber;

        /// <summary>
        /// Constructor.
        /// </summary>
        /// <param name="row">The row information.</param>
        /// <param name="name">The row index.</param>
        public DatabaseRowInfo(DataRow row, string name)
        {
            RowNumber = name;
            Data = row;
        } // DatabaseRowInfo
    } // class DatabaseRowInfo

    #endregion DatabaseRowInfo

    #region AccessDBContentReader

    /// <summary>
    /// Content reader used to retrieve data from this provider.
    /// </summary>
    public class AccessDBContentReader : IContentReader
    {
        // A provider instance is required so as to get "content"
        private AccessDBProvider provider;
        private string path;
        private long currentOffset;

        internal AccessDBContentReader(string path, AccessDBProvider provider)
        {
            this.path = path;
            this.provider = provider;
        }

        /// <summary>
        /// Read the specified number of rows from the source.
        /// </summary>
        /// <param name="readCount">The number of items to 
        /// return.</param>
        /// <returns>An array of elements read.</returns>
        public IList Read(long readCount)
        {
            // Read the number of rows specified by readCount and increment
            // offset
            string tableName;
            int rowNumber;
            PathType type = provider.GetNamesFromPath(path, out tableName, out rowNumber);

            Collection<DatabaseRowInfo> rows =
                provider.GetRows(tableName);
            Collection<DataRow> results = new Collection<DataRow>();

            if (currentOffset < 0 || currentOffset >= rows.Count)
            {
                return null;
            }

            int rowsRead = 0;

            while (rowsRead < readCount && currentOffset < rows.Count)
            {
                results.Add(rows[(int)currentOffset].Data);
                rowsRead++;
                currentOffset++;
            }

            return results;
        } // Read

        /// <summary>
        /// Moves the content reader specified number of rows from the 
        /// origin
        /// </summary>
        /// <param name="offset">Number of rows to offset</param>
        /// <param name="origin">Starting row from which to offset</param>
        public void Seek(long offset, System.IO.SeekOrigin origin)
        {
            // get the number of rows in the table which will help in
            // calculating current position
            string tableName;
            int rowNumber;

            PathType type = provider.GetNamesFromPath(path, out tableName, out rowNumber);

            if (type == PathType.Invalid)
            {
                throw new ArgumentException("Path specified must represent a table or a row :" + path);
            }

            if (type == PathType.Table)
            {
                Collection<DatabaseRowInfo> rows = provider.GetRows(tableName);

                int numRows = rows.Count;

                if (offset > rows.Count)
                {
                    throw new
                           ArgumentException(
                               "Offset cannot be greater than the number of rows available"
                                            );
                }

                if (origin == System.IO.SeekOrigin.Begin)
                {
                    // starting from Beginning with an index 0, the current offset
                    // has to be advanced to offset - 1
                    currentOffset = offset - 1;
                }
                else if (origin == System.IO.SeekOrigin.End)
                {
                    // starting from the end which is numRows - 1, the current
                    // offset is so much less than numRows - 1
                    currentOffset = numRows - 1 - offset;
                }
                else
                {
                    // calculate from the previous value of current offset
                    // advancing forward always
                    currentOffset += offset;
                }
            } // if (type...
            else
            {
                // for row, the offset will always be set to 0
                currentOffset = 0;
            }

        } // Seek

        /// <summary>
        /// Closes the content reader, so all members are reset
        /// </summary>
        public void Close()
        {
            Dispose();
        } // Close

        /// <summary>
        /// Dispose any resources being used
        /// </summary>
        public void Dispose()
        {
            Seek(0, System.IO.SeekOrigin.Begin);
            
            GC.SuppressFinalize(this);
        } // Dispose
    } // AccessDBContentReader

    #endregion AccessDBContentReader

    #region AccessDBContentWriter

    /// <summary>
    /// Content writer used to write data in this provider.
    /// </summary>
    public class AccessDBContentWriter : IContentWriter
    {
        // A provider instance is required so as to get "content"
        private AccessDBProvider provider;
        private string path;
        private long currentOffset;

        internal AccessDBContentWriter(string path, AccessDBProvider provider)
        {
            this.path = path;
            this.provider = provider;
        }

        /// <summary>
        /// Write the specified row contents in the source
        /// </summary>
        /// <param name="content"> The contents to be written to the source.
        /// </param>
        /// <returns>An array of elements which were successfully written to 
        /// the source</returns>
        /// 
        public IList Write(IList content)
        {
            if (content == null)
            {
                return null;
            }

            // Get the total number of rows currently available it will 
            // determine how much to overwrite and how much to append at
            // the end
            string tableName;
            int rowNumber;
            PathType type = provider.GetNamesFromPath(path, out tableName, out rowNumber);

            if (type == PathType.Table)
            {
                OdbcDataAdapter da = provider.GetAdapterForTable(tableName);
                if (da == null)
                {
                    return null;
                }

                DataSet ds = provider.GetDataSetForTable(da, tableName);
                DataTable table = provider.GetDataTable(ds, tableName);

                string[] colValues = (content[0] as string).Split(',');

                // set the specified row
                DataRow row = table.NewRow();

                for (int i = 0; i < colValues.Length; i++)
                {
                    if (!String.IsNullOrEmpty(colValues[i]))
                    {
                        row[i] = colValues[i];
                    }
                }

                //table.Rows.InsertAt(row, rowNumber);
                // Update the table
                table.Rows.Add(row);
                da.Update(ds, tableName);
                
            }
            else 
            {
                throw new InvalidOperationException("Operation not supported. Content can be added only for tables");
            }

            return null;
        } // Write

        /// <summary>
        /// Moves the content reader specified number of rows from the 
        /// origin
        /// </summary>
        /// <param name="offset">Number of rows to offset</param>
        /// <param name="origin">Starting row from which to offset</param>
        public void Seek(long offset, System.IO.SeekOrigin origin)
        {
            // get the number of rows in the table which will help in
            // calculating current position
            string tableName;
            int rowNumber;

            PathType type = provider.GetNamesFromPath(path, out tableName, out rowNumber);

            if (type == PathType.Invalid)
            {
                throw new ArgumentException("Path specified should represent either a table or a row : " + path);
            }

            Collection<DatabaseRowInfo> rows =
                   provider.GetRows(tableName);

            int numRows = rows.Count;

            if (offset > rows.Count)
            {
                throw new
                       ArgumentException(
                           "Offset cannot be greater than the number of rows available"
                                               );
            }

            if (origin == System.IO.SeekOrigin.Begin)
            {
                // starting from Beginning with an index 0, the current offset
                // has to be advanced to offset - 1
                currentOffset = offset - 1;
            }
            else if (origin == System.IO.SeekOrigin.End)
            {
                // starting from the end which is numRows - 1, the current
                // offset is so much less than numRows - 1
                currentOffset = numRows - 1 - offset;
            }
            else
            {
                // calculate from the previous value of current offset
                // advancing forward always
                currentOffset += offset;
            }

        } // Seek

        /// <summary>
        /// Closes the content reader, so all members are reset
        /// </summary>
        public void Close()
        {
            Dispose();
        } // Close

        /// <summary>
        /// Dispose any resources being used
        /// </summary>
        public void Dispose()
        {
            Seek(0, System.IO.SeekOrigin.Begin);

            GC.SuppressFinalize(this);
        } // Dispose
    } // AccessDBContentWriter

    #endregion AccessDBContentWriter

    #endregion Helper Classes
} // namespace Microsoft.Samples.PowerShell.Providers


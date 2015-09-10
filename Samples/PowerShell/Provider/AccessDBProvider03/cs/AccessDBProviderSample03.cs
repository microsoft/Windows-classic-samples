// <copyright file="AccessDBProviderSample03.cs" company="Microsoft Corporation">
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
using System.Collections.ObjectModel;
using System.Text;
using System.Diagnostics;
using System.Text.RegularExpressions;
using System.Management.Automation;
using System.Management.Automation.Provider;
using System.ComponentModel;
using System.Globalization;

namespace Microsoft.Samples.PowerShell.Providers
{
   #region AccessDBProvider

    /// <summary>
    /// A PowerShell Provider which acts upon a access database.
    /// </summary>
    /// <remarks>
    /// This example implements the item overloads.
    /// </remarks>
   [CmdletProvider("AccessDB", ProviderCapabilities.None)]

   public class AccessDBProvider : ItemCmdletProvider
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
      /// Chunks the path and returns the table name and the row number 
      /// from the path
      /// </summary>
      /// <param name="path">Path to chunk and obtain information</param>
      /// <param name="tableName">Name of the table as represented in the 
      /// path</param>
      /// <param name="rowNumber">Row number obtained from the path</param>
      /// <returns>what the path represents</returns>
      private PathType GetNamesFromPath(string path, out string tableName, out int rowNumber)
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
      private Collection<DatabaseTableInfo> GetTables()
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
      private Collection<DatabaseRowInfo> GetRows(string tableName)
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
      /// Obtain a data adapter for the specified Table
      /// </summary>
      /// <param name="tableName">Name of the table to obtain the 
      /// adapter for</param>
      /// <returns>Adapter object for the specified table</returns>
      /// <remarks>An adapter serves as a bridge between a DataSet (in memory
      /// representation of table) and the data source</remarks>
      private OdbcDataAdapter GetAdapterForTable(string tableName)
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
      private DataSet GetDataSetForTable(OdbcDataAdapter adapter, string tableName)
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
      private DataTable GetDataTable(DataSet ds, string tableName)
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
      } // SafeConvertRowNumber

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

      #endregion Helper Methods

      #region Private Properties

      private string pathSeparator = "\\";
      private static string pattern = @"^[a-z]+[0-9]*_*$";

      private enum PathType { Database, Table, Row, Invalid };

      #endregion Private Properties
  }

   #endregion AccessDBProvider

   #region Helper Classes

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

   #endregion Helper Classes
}


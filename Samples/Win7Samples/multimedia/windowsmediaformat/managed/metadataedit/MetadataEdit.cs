//*****************************************************************************
//
// Microsoft Windows Media
// Copyright ( C) Microsoft Corporation. All rights reserved.
//
// FileName:            main.cpp
//
// Abstract:            Sample to edit and view metadata from a media file.
//
//*****************************************************************************


using System;
using WMFSDKWrapper;

namespace WMFSDKSample
{
    class MetadataEdit
    {
        //------------------------------------------------------------------------------
        // Name: EditorOpenFile()
        // Desc: Creates a metadata editor and opens the file.
        //------------------------------------------------------------------------------
        bool EditorOpenFile( string pwszInFile, out IWMMetadataEditor ppEditor )            
        {
            ppEditor = null;

            try
            {
                WMFSDKFunctions.WMCreateEditor( out ppEditor );

                ppEditor.Open( pwszInFile ) ;
            }
            catch( System.Runtime.InteropServices.COMException e )
            {
                Console.WriteLine( e.Message );
                return( false );
            }

            return( true );
        }

        //------------------------------------------------------------------------------
        // Name: PrintListHeader()
        // Desc: Displays column headings.
        //------------------------------------------------------------------------------
        void PrintListHeader()
        {
            Console.WriteLine( "*" );
            Console.WriteLine( "* Idx  Name                   Stream Language Type  Value" );
            Console.WriteLine( "* ---  ----                   ------ -------- ----  -----" ) ;
        }

        //------------------------------------------------------------------------------
        // Name: PrintAttribute()
        // Desc: Displays the specified attribute.
        //------------------------------------------------------------------------------
        void PrintAttribute( ushort wIndex,
                             ushort wStream,
                             string pwszName,
                             WMT_ATTR_DATATYPE AttribDataType,
                             ushort wLangID,
                             byte[] pbValue,
                             uint dwValueLen )
        {
            string pwszValue = String.Empty;

            //
            // Make the data type string
            //
            string pwszType = "Unknown";
            string[] pTypes = { "DWORD", "STRING", "BINARY", "BOOL", "QWORD", "WORD", "GUID" };

            if( pTypes.Length > Convert.ToInt32(AttribDataType) )
            {
                pwszType = pTypes[Convert.ToInt32(AttribDataType)];
            }

            //
            // The attribute value.
            //
            switch ( AttribDataType )
            {
                // String
                case WMFSDKWrapper.WMT_ATTR_DATATYPE.WMT_TYPE_STRING:
                
                    if ( 0 == dwValueLen )
                    {
                        pwszValue = "***** NULL *****";
                    }
                    else
                    {
                        if ( ( 0xFE == Convert.ToInt16( pbValue[0] ) ) &&
                             ( 0xFF == Convert.ToInt16( pbValue[1] ) ) )
                        {
                            pwszValue = "\"UTF-16LE BOM+\"";
                            
                            if( 4 <= dwValueLen )
                            {
                                for ( int i = 0; i < pbValue.Length - 2; i += 2 )
                                {
                                    pwszValue += Convert.ToString( BitConverter.ToChar( pbValue, i ) );
                                }
                            }

                            pwszValue = pwszValue + "\"" ;
                        }
                        else if ( ( 0xFF == Convert.ToInt16( pbValue[0] ) ) &&
                                  ( 0xFE == Convert.ToInt16( pbValue[1] ) ) )
                        {
                            pwszValue = "\"UTF-16BE BOM+\""; 
                            if( 4 <= dwValueLen )
                            {
                                for ( int i = 0; i < pbValue.Length - 2; i += 2 )
                                {
                                    pwszValue += Convert.ToString( BitConverter.ToChar( pbValue, i ) );
                                }
                            }

                            pwszValue = pwszValue + "\"" ;
                        }
                        else
                        {
                            pwszValue = "\"";
                            if( 2 <= dwValueLen )
                            {
                                for ( int i = 0; i < pbValue.Length - 2; i += 2 )
                                {
                                    pwszValue += Convert.ToString( BitConverter.ToChar( pbValue, i ) );
                                }
                            }

                            pwszValue += "\"" ;
                        }
                    }
                    break;

                // Binary
                case WMFSDKWrapper.WMT_ATTR_DATATYPE.WMT_TYPE_BINARY:

                    pwszValue = "[" + dwValueLen.ToString() + " bytes]";
                    break;

                // Boolean
                case WMFSDKWrapper.WMT_ATTR_DATATYPE.WMT_TYPE_BOOL:

                    if ( BitConverter.ToBoolean( pbValue, 0 ) )
                    {
                        pwszValue = "True";
                    }
                    else
                    {
                        pwszValue = "False";
                    }
                    break;

                // DWORD
                case WMFSDKWrapper.WMT_ATTR_DATATYPE.WMT_TYPE_DWORD:    
                    
                    uint dwValue = BitConverter.ToUInt32( pbValue, 0 );
                    pwszValue = dwValue.ToString( );
                    break;

                // QWORD
                case WMFSDKWrapper.WMT_ATTR_DATATYPE.WMT_TYPE_QWORD:

                    ulong qwValue = BitConverter.ToUInt64( pbValue, 0 );
                    pwszValue = qwValue.ToString( );
                    break;

                // WORD
                case WMFSDKWrapper.WMT_ATTR_DATATYPE.WMT_TYPE_WORD:
                    
                    uint wValue = BitConverter.ToUInt16( pbValue, 0 );
                    pwszValue = wValue.ToString( );
                    break;

                // GUID
                case WMFSDKWrapper.WMT_ATTR_DATATYPE.WMT_TYPE_GUID:

                    pwszValue = BitConverter.ToString( pbValue, 0, pbValue.Length );
                    break;

                default:

                    break;
            }
    
            //
            // Dump the string to the screen.
            //  
            Console.WriteLine( "* {0, 3}  {1, -25} {2, 3}  {3, 3}  {4, 7}  {5}", 
                wIndex, pwszName, wStream, wLangID, pwszType, pwszValue );
        }

        //------------------------------------------------------------------------------
        // Name: ShowAttributes()
        // Desc: Displays all attributes for the specified stream.
        //------------------------------------------------------------------------------
        bool ShowAttributes( string pwszFileName, ushort wStreamNum )
        {
            try
            {
                IWMMetadataEditor   MetadataEditor;
                IWMHeaderInfo3      HeaderInfo3;
                ushort              wAttributeCount;

                WMFSDKFunctions.WMCreateEditor( out MetadataEditor );

                MetadataEditor.Open( pwszFileName ) ;

                HeaderInfo3 = ( IWMHeaderInfo3 )MetadataEditor;

                HeaderInfo3.GetAttributeCount( wStreamNum, out wAttributeCount );

                PrintListHeader();

                for( ushort wAttribIndex = 0; wAttribIndex < wAttributeCount; wAttribIndex++)
                {
                    WMT_ATTR_DATATYPE   wAttribType;
                    string              pwszAttribName = null;
                    byte[]              pbAttribValue = null;
                    ushort              wAttribNameLen = 0;
                    ushort              wAttribValueLen = 0;

                    HeaderInfo3.GetAttributeByIndex( wAttribIndex,
                                                     ref wStreamNum,
                                                     pwszAttribName,
                                                     ref wAttribNameLen,
                                                     out wAttribType,
                                                     pbAttribValue,
                                                     ref wAttribValueLen );

                    pbAttribValue = new byte[ wAttribValueLen ];
                    pwszAttribName = new String( (char)0, wAttribNameLen );

                    HeaderInfo3.GetAttributeByIndex( wAttribIndex,
                                                     ref wStreamNum,
                                                     pwszAttribName,
                                                     ref wAttribNameLen,
                                                     out wAttribType,
                                                     pbAttribValue,
                                                     ref wAttribValueLen );

                    PrintAttribute( wAttribIndex, wStreamNum, pwszAttribName, wAttribType, 0, pbAttribValue, wAttribValueLen );
                }
            }
            catch( Exception e )
            {
                Console.WriteLine( e.Message );
                return( false );
            }

            return( true );
        }

        //------------------------------------------------------------------------------
        // Name: ShowAttributes3()
        // Desc: Displays all attributes for the specified stream, with support
        //       for GetAttributeByIndexEx.
        //------------------------------------------------------------------------------
        bool ShowAttributes3( string pwszFileName, ushort wStreamNum )
        {
            try
            {
                IWMMetadataEditor   MetadataEditor;
                IWMHeaderInfo3      HeaderInfo3;
                ushort wAttributeCount = 0;

                WMFSDKFunctions.WMCreateEditor( out MetadataEditor );

                MetadataEditor.Open( pwszFileName ) ;

                HeaderInfo3 = ( IWMHeaderInfo3 )MetadataEditor;
                
                HeaderInfo3.GetAttributeCountEx( wStreamNum, out wAttributeCount );

                PrintListHeader();

                for( ushort wAttribIndex = 0; wAttribIndex < wAttributeCount; wAttribIndex++)
                {
                    WMT_ATTR_DATATYPE   wAttribType;
                    ushort              wLangIndex = 0;
                    string              pwszAttribName = null;
                    byte[]              pbAttribValue = null;
                    ushort              wAttribNameLen = 0;
                    uint                dwAttribValueLen = 0;

                    HeaderInfo3.GetAttributeByIndexEx( wStreamNum,
                                                       wAttribIndex,
                                                       pwszAttribName,
                                                       ref wAttribNameLen,
                                                       out wAttribType,
                                                       out wLangIndex,
                                                       pbAttribValue,
                                                       ref dwAttribValueLen);
                
                    pwszAttribName = new String( ( char )0, wAttribNameLen );
                    pbAttribValue = new byte[ dwAttribValueLen ];
                                    
                    HeaderInfo3.GetAttributeByIndexEx( wStreamNum,
                                                       wAttribIndex,
                                                       pwszAttribName,
                                                       ref wAttribNameLen,
                                                       out wAttribType,
                                                       out wLangIndex,
                                                       pbAttribValue,
                                                       ref dwAttribValueLen );

                    PrintAttribute( wAttribIndex, wStreamNum, pwszAttribName, wAttribType, 0, pbAttribValue, dwAttribValueLen );
                }
            }
            catch( Exception e )
            {
                Console.WriteLine( e.Message );
                return( false );
            }

            return( true );
        }

        //------------------------------------------------------------------------------
        // Name: DeleteAttrib()
        // Desc: Delete the attribute at the specified index.
        //------------------------------------------------------------------------------
        bool DeleteAttrib( string pwszFileName, ushort wStreamNum, ushort wAttribIndex )
        {
            try
            {
                IWMMetadataEditor   MetadataEditor;
                IWMHeaderInfo3      HeaderInfo3;

                WMFSDKFunctions.WMCreateEditor( out MetadataEditor );

                MetadataEditor.Open( pwszFileName ) ;

                HeaderInfo3 = ( IWMHeaderInfo3 )MetadataEditor;

                HeaderInfo3.DeleteAttribute( wStreamNum, wAttribIndex );

                MetadataEditor.Flush();

                MetadataEditor.Close() ;
            }
            catch( Exception e )
            {
                Console.WriteLine( e.Message );
                return( false );
            }

            return( true );
        }

        //------------------------------------------------------------------------------
        // Name: TranslateAttrib()
        // Desc: Converts attributes to byte arrays.
        //------------------------------------------------------------------------------
        bool TranslateAttrib( WMT_ATTR_DATATYPE AttribDataType, string pwszValue, out byte [] pbValue, out int nValueLength )
        {
            switch( AttribDataType )
            {
                case WMFSDKWrapper.WMT_ATTR_DATATYPE.WMT_TYPE_DWORD:

                    nValueLength = 4;
                    uint[] pdwAttribValue = new uint[1] { Convert.ToUInt32( pwszValue ) };

                    pbValue = new Byte[nValueLength];
                    Buffer.BlockCopy( pdwAttribValue, 0, pbValue, 0, nValueLength );
                    
                    return( true );

                case WMFSDKWrapper.WMT_ATTR_DATATYPE.WMT_TYPE_WORD:

                    nValueLength = 2;
                    ushort[] pwAttribValue = new ushort[1] { Convert.ToUInt16( pwszValue ) };

                    pbValue = new Byte[nValueLength];
                    Buffer.BlockCopy( pwAttribValue, 0, pbValue, 0, nValueLength );

                    return( true );

                case WMFSDKWrapper.WMT_ATTR_DATATYPE.WMT_TYPE_QWORD:

                    nValueLength = 8;
                    ulong[] pqwAttribValue = new ulong[1] { Convert.ToUInt64( pwszValue ) };

                    pbValue = new Byte[nValueLength];
                    Buffer.BlockCopy( pqwAttribValue, 0, pbValue, 0, nValueLength );

                    return( true );
                            
                case WMFSDKWrapper.WMT_ATTR_DATATYPE.WMT_TYPE_STRING:

                    nValueLength = ( ushort )( ( pwszValue.Length + 1 ) * 2 );
                    pbValue = new Byte[ nValueLength ];
                    
                    Buffer.BlockCopy( pwszValue.ToCharArray(), 0, pbValue, 0, pwszValue.Length * 2 );
                    pbValue[ nValueLength - 2 ] = 0;
                    pbValue[ nValueLength - 1 ] = 0;

                    return( true );

                case WMFSDKWrapper.WMT_ATTR_DATATYPE.WMT_TYPE_BOOL:

                    nValueLength = 4;
                    pdwAttribValue = new uint[1] { Convert.ToUInt32( pwszValue ) };
                    if ( pdwAttribValue[0] != 0 )
                    {
                        pdwAttribValue[0] = 1;
                    }

                    pbValue = new Byte[nValueLength];
                    Buffer.BlockCopy( pdwAttribValue, 0, pbValue, 0, nValueLength );
                    
                    return( true );

                default:

                    pbValue = null;
                    nValueLength = 0;
                    Console.WriteLine( "Unsupported data type." );

                    return( false );
            }
        }

        //------------------------------------------------------------------------------
        // Name: SetAttrib()
        // Desc: Set the specified attribute.
        //------------------------------------------------------------------------------
        bool SetAttrib( string pwszFileName, ushort wStreamNum, string pwszAttribName, 
                        ushort wAttribType, string pwszAttribValue )
        {
            try
            {
                IWMMetadataEditor   MetadataEditor;
                IWMHeaderInfo3      HeaderInfo3;
                byte[]              pbAttribValue;
                int                 nAttribValueLen;
                WMT_ATTR_DATATYPE   AttribDataType = ( WMT_ATTR_DATATYPE )wAttribType;

                if ( !TranslateAttrib( AttribDataType, pwszAttribValue, out pbAttribValue, out nAttribValueLen ) )
                {
                    return false;
                }

                WMFSDKFunctions.WMCreateEditor( out MetadataEditor );

                MetadataEditor.Open( pwszFileName ) ;

                HeaderInfo3 = ( IWMHeaderInfo3 )MetadataEditor;
                
                HeaderInfo3.SetAttribute( wStreamNum,
                                          pwszAttribName,
                                          AttribDataType,                                        
                                          pbAttribValue,
                                          (ushort)nAttribValueLen );

                MetadataEditor.Flush();

                MetadataEditor.Close() ;
            }
            catch( Exception e )
            {
                Console.WriteLine( e.Message );
                return( false );
            }

            return( true );
        }

        //------------------------------------------------------------------------------
        // Name: AddAttrib()
        // Desc: Add an attribute with the specifed language index.
        //------------------------------------------------------------------------------
        bool AddAttrib( string pwszFileName, ushort wStreamNum, string pwszAttribName, 
                        ushort wAttribType, string pwszAttribValue, ushort wLangIndex )
        {
            try
            {
                IWMMetadataEditor   MetadataEditor;
                IWMHeaderInfo3      HeaderInfo3;
                byte[]              pbAttribValue;
                int                 nAttribValueLen;
                WMT_ATTR_DATATYPE   AttribDataType = ( WMT_ATTR_DATATYPE ) wAttribType;
                ushort              wAttribIndex = 0;

                if ( !TranslateAttrib( AttribDataType, pwszAttribValue, out pbAttribValue, out nAttribValueLen ) )
                {
                    return false;
                }

                WMFSDKFunctions.WMCreateEditor( out MetadataEditor );

                MetadataEditor.Open( pwszFileName ) ;

                HeaderInfo3 = ( IWMHeaderInfo3 )MetadataEditor;

                HeaderInfo3.AddAttribute( wStreamNum,
                                          pwszAttribName,
                                          out wAttribIndex,
                                          AttribDataType, 
                                          wLangIndex,
                                          pbAttribValue,
                                          (uint)nAttribValueLen);
                                                                                
                MetadataEditor.Flush();

                MetadataEditor.Close();
            }
            catch( Exception e )
            {
                Console.WriteLine( e.Message );
                return( false );
            }

            return( true );
        }

        //------------------------------------------------------------------------------
        // Name: ModifyAttrib()
        // Desc: Modifies the value of the specified attribute.
        //------------------------------------------------------------------------------
        bool ModifyAttrib( string pwszFileName, ushort wStreamNum, ushort wAttribIndex, 
                           ushort wAttribType, string pwszAttribValue, ushort wLangIndex )
        {
            try
            {
                IWMMetadataEditor   MetadataEditor;
                IWMHeaderInfo3      HeaderInfo3;
                byte[]              pbAttribValue;
                int                 nAttribValueLen;
                WMT_ATTR_DATATYPE   AttribDataType = ( WMT_ATTR_DATATYPE ) wAttribType;

                if ( !TranslateAttrib( AttribDataType, pwszAttribValue, out pbAttribValue, out nAttribValueLen ) )
                {
                    return false;
                }

                WMFSDKFunctions.WMCreateEditor( out MetadataEditor );

                MetadataEditor.Open( pwszFileName ) ;

                HeaderInfo3 = ( IWMHeaderInfo3 )MetadataEditor;

                HeaderInfo3.ModifyAttribute( wStreamNum,
                                             wAttribIndex,
                                             AttribDataType,
                                             wLangIndex,
                                             pbAttribValue,
                                             (uint)nAttribValueLen );

                MetadataEditor.Flush();

                MetadataEditor.Close();
            }
            catch( Exception e )
            {
                Console.WriteLine( e.Message );
                return( false );
            }

            return( true );
        }

        //------------------------------------------------------------------------------
        // Name: Usage()
        // Desc: Show the correct command-line usage.
        //------------------------------------------------------------------------------
        void Usage()
        {
            Console.WriteLine( "MetadataEdit\t <filename> show <stream number>"  );
            Console.WriteLine( "\t\t <filename> show3 <stream number>" );
            Console.WriteLine( "\t\t <filename> delete <stream number> <attrib index>" );
            Console.WriteLine( "\t\t <filename> set <stream number> <attrib name> <attrib type> <attrib value>" ) ;
            Console.WriteLine( "\t\t <filename> add <stream number> <attrib name> <attrib type> <attrib value> <attrib language>" ) ;
            Console.WriteLine( "\t\t <filename> modify <stream number> <attrib index> <attrib type> <attrib value> <attrib language>" ) ;

            Console.WriteLine( "\n Attrib Type can have one of the following values:" );
            Console.WriteLine( "\t 0 - WMT_TYPE_DWORD" );
            Console.WriteLine( "\t 1 - WMT_TYPE_STRING" );
            Console.WriteLine( "\t 2 - WMT_TYPE_BINARY" );
            Console.WriteLine( "\t 3 - WMT_TYPE_BOOL" );
            Console.WriteLine( "\t 4 - WMT_TYPE_QWORD" );
            Console.WriteLine( "\t 5 - WMT_TYPE_WORD" );
            Console.WriteLine( "\t 6 - WMT_TYPE_GUID" );
        }

        //------------------------------------------------------------------------------
        // Name: Main()
        // Desc: Creates the MetadataEdit object and parses the command line.
        //------------------------------------------------------------------------------
        [STAThread]
        static void Main( string[] args )
        {
            try
            {
                MetadataEdit Editor = new MetadataEdit();

                if ( args.Length < 3 )
                {
                    Editor.Usage();
                    return;
                }

                if ( args[1].Equals( "show" ) )
                {
                    if ( 3 != args.Length )
                    {
                        Editor.Usage( );
                        return;
                    }

                    ushort wStreamNum = Convert.ToUInt16( args[2] );

                    if ( !Editor.ShowAttributes( args[0], wStreamNum ) )
                    {
                        Console.WriteLine( "ShowAttributes failed." );
                        return;
                    }
                }
                else if ( args[1].Equals( "show3" ) )
                {
                    if ( 3 != args.Length )
                    {
                        Editor.Usage( );
                        return;
                    }

                    ushort wStreamNum = Convert.ToUInt16( args[2] );

                    if ( !Editor.ShowAttributes3( args[0], wStreamNum ) )
                    {
                        Console.WriteLine( "ShowAttributes3 failed." );
                        return;
                    }
                }   
                else if ( args[1].Equals( "delete" ) )
                {
                    if ( 4 != args.Length )
                    {
                        Editor.Usage( );
                        return;
                    }

                    ushort wStreamNum = Convert.ToUInt16( args[2] );
                    ushort wAttribIndex = Convert.ToUInt16( args[3] );
                    
                    if ( !Editor.DeleteAttrib( args[0], wStreamNum, wAttribIndex ) )
                    {
                        Console.WriteLine( "DeleteAttrib failed." );
                        return;
                    }   
            
                    if ( !Editor.ShowAttributes3( args[0], wStreamNum ) )
                    {
                        Console.WriteLine( "ShowAttributes3 failed." );
                        return;
                    }
                }
                else if ( args[1].Equals( "set" ) )
                {
                    if ( 6 != args.Length )
                    {
                        Editor.Usage();
                        return;
                    }


                    ushort wStreamNum = Convert.ToUInt16( args[2] );
                    ushort wAttribType = Convert.ToUInt16( args[4] );
                
                    if ( !Editor.SetAttrib( args[0], wStreamNum, args[3], wAttribType, args[5] ) )
                    {
                        Console.WriteLine( "SetAttrib failed." );
                        return;
                    }

                    if ( !Editor.ShowAttributes( args[0], wStreamNum ) )
                    {
                        Console.WriteLine( "ShowAttributes failed." );
                        return;
                    }
                }
                else if( args[1].Equals( "add" ) )
                {
                    if ( 7 != args.Length )
                    {
                        Editor.Usage();
                        return;
                    }

                    ushort wStreamNum = Convert.ToUInt16( args[2] );
                    ushort wAttribType = Convert.ToUInt16( args[4] );
                    ushort wLangIndex = Convert.ToUInt16( args[6] );

                    if ( !Editor.AddAttrib( args[0], wStreamNum, args[3], wAttribType, args[5], wLangIndex ) )
                    {
                       Console.WriteLine( "AddAttrib failed." );
                       return;
                    }

                    if ( !Editor.ShowAttributes3( args[0], wStreamNum ) )
                    {
                        Console.WriteLine( "ShowAttributes3 failed." );
                        return;
                    }
                }
                else if( args[1].Equals( "modify" ) )
                {
                    if ( 7 != args.Length )
                    {
                        Editor.Usage();
                        return;
                    }

                    ushort wStreamNum = Convert.ToUInt16( args[2] );
                    ushort wAttribIndex = Convert.ToUInt16( args[3] );
                    ushort wAttribType = Convert.ToUInt16( args[4] );
                    ushort wLangIndex = Convert.ToUInt16( args[6] );

                    if ( !Editor.ModifyAttrib( args[0], wStreamNum, wAttribIndex, wAttribType, args[5], wLangIndex ) )
                    {
                        Console.WriteLine( "ModifyAttrib failed." );
                        return;
                    }

                    if ( !Editor.ShowAttributes3( args[0], wStreamNum ) )
                    {
                        Console.WriteLine( "ShowAttributes3 failed." );
                        return;
                    }
                }
                else
                {
                    Editor.Usage( );
                    return;
                }

                String Footer = new String( '*', 70 );
                
                Console.WriteLine( Footer );
            }
            catch ( Exception e )
            {
                Console.WriteLine( e.Message );
                Console.WriteLine( "Invalid argument was found" );
            }
        }
    }
}
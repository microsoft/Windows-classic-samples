//------------------------------------------------------------------------------
//
// Microsoft Windows Media
// Copyright (C) Microsoft Corporation. All rights reserved.
//
// PluginAdmin
//
//  Inherits auto-generated cacheproxysampleplugin.cs (class PersistedConfiguration)
//  Exposes admin interface
//  Implements loading from file and persisting to file
// *******************NOTE*****************************
//THIS COMPONENT IS IMPLEMENTED FOR DEMONSTRATION PURPOSES ONLY.
// CURRENT PLUGIN IMPLEMENTATION DOESN'T USE ANY OF ITS CONFIGURATION SETTINGS
// THOUGH IT SHOWS HOW EASILY THE PLUGIN ADMIN CAN BE USED
//------------------------------------------------------------------------------



using System.Xml.Serialization;
using System;
using System.IO;
using System.Xml;
using Microsoft.WindowsMediaServices.Interop;
using System.Runtime.InteropServices;
using System.Diagnostics;

[Guid("b70d2b4d-3ff5-4ac0-8d23-0fb974618b05")]
public interface IWMSCacheAdmin
{
    int DiskQuota { get; set; }
    string CacheDirectoryPath { get; set; }
    bool ProxyOnDemandCacheMiss { get; set; }
    string PreferredProtocol { get; set; }
    bool CacheOnDemandCacheMiss { get; set; }
    int ArchiveQuotaPerStream { get; set; }
    string ProxyRedirectURL { get; set; }
    string BackendServer { get; set; }
}

/// <remarks/>
[ComVisible( true ) ]
[ClassInterface( ClassInterfaceType.AutoDual ) ]
[IDispatchImpl( System.Runtime.InteropServices.IDispatchImplType.CompatibleImpl)]
public class CachePluginAdmin : IWMSCacheAdmin
{
    private PersistedConfiguration SerializedValues;
    private string strPathToConfigDoc = "";
    private WMSNamedValues g_NamedValues = null;
    public delegate void ModifySettingsEventHandler( string strName, object objValue );
    private ModifySettingsEventHandler RegisteredChangeHandler = null;

    public CachePluginAdmin()
    {
        SerializedValues = new PersistedConfiguration();
        SerializedValues.diskQuota = -1;
        SerializedValues.archiveQuotaPerStream = -1;
        SerializedValues.reverseProxyRedirectURL = "";
        SerializedValues.backendServer = "";
        SerializedValues.enableCaching = false;
        SerializedValues.enableProxy = false;
        SerializedValues.proxyOnDemandCacheMiss = false;
        SerializedValues.cacheOnDemandCacheMiss = false;
        SerializedValues.handleUpstreamCacheRequests = false;
        SerializedValues.protocol = protocolEnum.clientprotocol;
        RegisteredChangeHandler = null;
    }

    public WMSNamedValues NamedValues
    {
        set 
        {
            if( null == g_NamedValues )
            {
                Console.WriteLine( "Named values now set" );
                g_NamedValues = value;
            }
        }
    }

    public void LoadSettingsFromServerNamespace()
    {
        if( null == g_NamedValues )
        {
            return;
        }
        if( null == SerializedValues )
        {
            SerializedValues = new PersistedConfiguration();
            FetchNameValFromServerNamespace( "ArchiveQuotaPerStream", SerializedValues.archiveQuotaPerStream );
            FetchNameValFromServerNamespace( "BackendServer", SerializedValues.backendServer );
            FetchNameValFromServerNamespace( "CacheDirectoryPath", SerializedValues.cacheDirectoryPath );
            FetchNameValFromServerNamespace( "CacheOnDemandCacheMiss", SerializedValues.cacheOnDemandCacheMiss );
            FetchNameValFromServerNamespace( "DiskQuota", SerializedValues.diskQuota );
            FetchNameValFromServerNamespace( "EnableCaching", SerializedValues.enableCaching );
            FetchNameValFromServerNamespace( "EnableProxy", SerializedValues.enableProxy );
            FetchNameValFromServerNamespace( "HandleUpstreamCacheRequests", SerializedValues.handleUpstreamCacheRequests );
            FetchNameValFromServerNamespace( "PreferredProtocol", SerializedValues.protocol );
            FetchNameValFromServerNamespace( "ProxyOnDemandCacheMiss", SerializedValues.proxyOnDemandCacheMiss );
            FetchNameValFromServerNamespace( "ReverseProxyRedirectURL", SerializedValues.reverseProxyRedirectURL );
        }
    }

    public bool LoadSettingsFromXmlFile( string strPath )
    {
        FileStream fs = null;
        bool bSucceeded = false;
        try
        {
            // A FileStream is needed to read the XML document.
            fs = new FileStream( strPath, FileMode.Open);
        }
        catch( Exception e )
        {
            Console.WriteLine( "Exception: {0}\n{1}", e.Message, e.StackTrace );
            return( false );
        }

        XmlTextReader reader = new XmlTextReader(fs);

        // Create an instance of the XmlSerializer specifying type and namespace.
        XmlSerializer serializer = new XmlSerializer( typeof( PersistedConfiguration ) );

        PersistedConfiguration tmpSettings = null;

        try
        {
            tmpSettings = (PersistedConfiguration) serializer.Deserialize( reader );
        }
        catch( Exception e )
        {
            Console.WriteLine( "Exception: {0}\n{1}", e.Message, e.StackTrace );
        }
        if( null != tmpSettings )
        {
            SerializedValues = null;
            SerializedValues = tmpSettings;
            tmpSettings = null;
            bSucceeded = true;
        }

        serializer = null;
        reader.Close();
        reader = null;
        strPathToConfigDoc = strPath;
        return( bSucceeded );
    }

    public void PersistChangesToXmlFile( string strPath )
    {
        XmlTextWriter writer = new XmlTextWriter( strPath, System.Text.Encoding.UTF8 );

        // Create an instance of the XmlSerializer specifying type and namespace.
        XmlSerializer serializer = new XmlSerializer( typeof( PersistedConfiguration ) );

        serializer.Serialize( writer, ( object ) SerializedValues );
        strPathToConfigDoc = strPath;

        writer.Flush();
        writer.Close();
        writer = null;
        serializer = null;
    }

    public void PersistChangesToXmlFile()
    {
        if( 0 < strPathToConfigDoc.Length )
        {
            PersistChangesToXmlFile( strPathToConfigDoc );
        }
    }

    public void RegisterChangeHandler( ModifySettingsEventHandler theHandler )
    {
        RegisteredChangeHandler = theHandler;
    }

    public object FetchNameValFromServerNamespace( string strKey, object objValue )
    {
        if( null == g_NamedValues )
        {
            return( null );
        }
        foreach( IWMSNamedValue NamedVal in g_NamedValues )
        {
            if( NamedVal.Name == strKey )
            {
                return( NamedVal.Value );
            }
        }
        return( null );
    }

    public void PersistNameValToServerNamespace( string strKey, object objValue )
    {
        if( null == g_NamedValues )
        {
            return;
        }
        bool bPersisted = false;
        foreach( IWMSNamedValue NamedVal in g_NamedValues )
        {
            if( NamedVal.Name == strKey )
            {
                // Console.WriteLine( "Persisting {0} to the server namespace: {1}", strKey, objValue );
                NamedVal.Value = objValue;
                bPersisted = false;
                break;
            }
        }
        if( ! bPersisted )
        {
            g_NamedValues.Add( strKey, objValue );
        }
    }

    public void RegisterChange( string strName, object objValue )
    {
        // invoke callback
        if( null != RegisteredChangeHandler )
        {
            // persist the changes back to disk
            PersistChangesToXmlFile();
            RegisteredChangeHandler( strName, objValue );
        }
    }

    //-------------------------------------------------- 
    //
    //  IWMSCacheAdmin interface
    //
    //-------------------------------------------------- 
    int IWMSCacheAdmin.DiskQuota
    {
        // [DispIdAttribute( 1 )]
        get { return( SerializedValues.diskQuota ); }
        // [DispIdAttribute( 1 )]
        set 
        { 
            if( value != SerializedValues.diskQuota )
            {
                SerializedValues.diskQuota = value; 
                RegisterChange( "DiskQuota", value );
            }
        }
    }

    string IWMSCacheAdmin.CacheDirectoryPath
    {
        // [DispIdAttribute( 2 )]
        get { return( SerializedValues.cacheDirectoryPath ); }
        // [DispIdAttribute( 2 )]
        set 
        { 
            if( value != SerializedValues.cacheDirectoryPath )
            {
                SerializedValues.cacheDirectoryPath = value; 
                RegisterChange( "CacheDirectoryPath", value );
            }
        }
    }

    bool IWMSCacheAdmin.ProxyOnDemandCacheMiss
    {
        // [DispIdAttribute( 3 )]
        get { return( SerializedValues.proxyOnDemandCacheMiss ); }
        // [DispIdAttribute( 3 )]
        set 
        { 
            if( value != SerializedValues.proxyOnDemandCacheMiss )
            {
                SerializedValues.proxyOnDemandCacheMiss = value; 
                RegisterChange( "ProxyOnDemandCacheMiss", value );
            }
        }
    }

    string IWMSCacheAdmin.PreferredProtocol
    {
        // [DispIdAttribute( 4 )]
        get 
        {
            string strRetVal = "";
            switch( SerializedValues.protocol )
            {
                case protocolEnum.clientprotocol:
                    strRetVal = "clientprotocol";
                    break;
                case protocolEnum.HTTP:
                    strRetVal = "HTTP";
                    break;
                case protocolEnum.RTSPT:
                    strRetVal = "RTSPT";
                    break;
                case protocolEnum.RTSPU:
                    strRetVal = "RTSPU";
                    break;
            }
            return( strRetVal );
        }
        // [DispIdAttribute( 4 )]
        set
        {
            protocolEnum enumProtocol;
            switch( value )
            {
                case "clientprotocol":
                    enumProtocol = protocolEnum.clientprotocol;
                    break;
                case "HTTP":
                    enumProtocol = protocolEnum.HTTP;
                    break;
                case "RTSPT":
                    enumProtocol = protocolEnum.RTSPT;
                    break;
                case "RTSPU":
                    enumProtocol = protocolEnum.RTSPU;
                    break;
                default:
                    enumProtocol = protocolEnum.clientprotocol;
                    break;
            }
            if( enumProtocol != SerializedValues.protocol )
            {
                RegisterChange( "PreferredProtocol", value );
            }
        }
    }

    bool IWMSCacheAdmin.CacheOnDemandCacheMiss
    {
        // [DispIdAttribute( 5 )]
        get { return( SerializedValues.cacheOnDemandCacheMiss ); }
        // [DispIdAttribute( 5 )]
        set 
        { 
            if( value != SerializedValues.cacheOnDemandCacheMiss )
            {
                SerializedValues.cacheOnDemandCacheMiss = value; 
                RegisterChange( "CacheOnDemandCacheMiss", value );
            }
        }
    }

    int IWMSCacheAdmin.ArchiveQuotaPerStream
    {
        // [DispIdAttribute( 6 )]
        get { return( SerializedValues.archiveQuotaPerStream ); }
        // [DispIdAttribute( 6 )]
        set 
        { 
            if( value != SerializedValues.archiveQuotaPerStream )
            {
                SerializedValues.archiveQuotaPerStream = value; 
                RegisterChange( "ArchiveQuotaPerStream", value );
            }
        }
    }

    string IWMSCacheAdmin.ProxyRedirectURL
    {
        // [DispIdAttribute( 7 )]
        get { return( SerializedValues.reverseProxyRedirectURL ); }
        // [DispIdAttribute( 7 )]
        set 
        { 
            if( value != SerializedValues.reverseProxyRedirectURL )
            {
                SerializedValues.reverseProxyRedirectURL = value; 
                RegisterChange( "ProxyRedirectURL", value );
            }
        }
    }

    string IWMSCacheAdmin.BackendServer
    {
        // [DispIdAttribute( 8 )]
        get { return( SerializedValues.backendServer ); }
        // [DispIdAttribute( 8 )]
        set 
        { 
            if( value != SerializedValues.backendServer )
            {
                SerializedValues.backendServer = value; 
                RegisterChange( "BackendServer", value );
            }
        }
    }

    //-------------------------------------------------- 
    //
    //  IDispatch interface
    //
    //-------------------------------------------------- 
    public int DiskQuota
    {
        [DispIdAttribute( 1 )]
        get { return( SerializedValues.diskQuota ); }
        [DispIdAttribute( 1 )]
        set 
        { 
            if( value != SerializedValues.diskQuota )
            {
                SerializedValues.diskQuota = value; 
                RegisterChange( "DiskQuota", value );
            }
        }
    }

    public string CacheDirectoryPath
    {
        [DispIdAttribute( 2 )]
        get { return( SerializedValues.cacheDirectoryPath ); }
        [DispIdAttribute( 2 )]
        set 
        { 
            if( value != SerializedValues.cacheDirectoryPath )
            {
                SerializedValues.cacheDirectoryPath = value; 
                RegisterChange( "CacheDirectoryPath", value );
            }
        }
    }

    public bool ProxyOnDemandCacheMiss
    {
        [DispIdAttribute( 3 )]
        get { return( SerializedValues.proxyOnDemandCacheMiss ); }
        [DispIdAttribute( 3 )]
        set 
        { 
            if( value != SerializedValues.proxyOnDemandCacheMiss )
            {
                SerializedValues.proxyOnDemandCacheMiss = value; 
                RegisterChange( "ProxyOnDemandCacheMiss", value );
            }
        }
    }

    public string PreferredProtocol
    {
        [DispIdAttribute( 4 )]
        get 
        {
            string strRetVal;
            switch( SerializedValues.protocol )
            {
                case protocolEnum.HTTP:
                    strRetVal = "HTTP";
                    break;
                case protocolEnum.RTSPT:
                    strRetVal = "RTSPT";
                    break;
                case protocolEnum.RTSPU:
                    strRetVal = "RTSPU";
                    break;
                case protocolEnum.clientprotocol:
                    strRetVal = "clientprotocol";
                    break;
                default:
                    strRetVal = "clientprotocol";
                    break;
            }
            return( strRetVal );
        }
        [DispIdAttribute( 4 )]
        set
        {
            protocolEnum enumProtocol;
            switch( value )
            {
                case "HTTP":
                    enumProtocol = protocolEnum.HTTP;
                    break;
                case "RTSPT":
                    enumProtocol = protocolEnum.RTSPT;
                    break;
                case "RTSP":
                    enumProtocol = protocolEnum.RTSPU;
                    break;
                case "RTSPU":
                    enumProtocol = protocolEnum.RTSPU;
                    break;
                case "clientprotocol":
                    enumProtocol = protocolEnum.clientprotocol;
                    break;
                default:
                    Console.WriteLine( "*** BAD VALUE DETECTED: {0}", value );
                    enumProtocol = protocolEnum.clientprotocol;
                    break;
            }
            if( enumProtocol != SerializedValues.protocol )
            {
                SerializedValues.protocol = enumProtocol;
                RegisterChange( "PreferredProtocol", value );
            }
        }
    }

    public bool CacheOnDemandCacheMiss
    {
        [DispIdAttribute( 5 )]
        get { return( SerializedValues.cacheOnDemandCacheMiss ); }
        [DispIdAttribute( 5 )]
        set 
        { 
            if( value != SerializedValues.cacheOnDemandCacheMiss )
            {
                SerializedValues.cacheOnDemandCacheMiss = value; 
                RegisterChange( "CacheOnDemandCacheMiss", value );
            }
        }
    }

    public int ArchiveQuotaPerStream
    {
        [DispIdAttribute( 6 )]
        get { return( SerializedValues.archiveQuotaPerStream ); }
        [DispIdAttribute( 6 )]
        set 
        { 
            if( value != SerializedValues.archiveQuotaPerStream )
            {
                SerializedValues.archiveQuotaPerStream = value; 
                RegisterChange( "ArchiveQuotaPerStream", value );
            }
        }
    }

    public string ProxyRedirectURL
    {
        [DispIdAttribute( 7 )]
        get { return( SerializedValues.reverseProxyRedirectURL ); }
        [DispIdAttribute( 7 )]
        set 
        { 
            if( value != SerializedValues.reverseProxyRedirectURL )
            {
                SerializedValues.reverseProxyRedirectURL = value; 
                RegisterChange( "ProxyRedirectURL", value );
            }
        }
    }

    public string BackendServer
    {
        [DispIdAttribute( 8 )]
        get { return( SerializedValues.backendServer ); }
        [DispIdAttribute( 8 )]
        set 
        { 
            if( value != SerializedValues.backendServer )
            {
                SerializedValues.backendServer = value; 
                RegisterChange( "BackendServer", value );
            }
        }
    }
}
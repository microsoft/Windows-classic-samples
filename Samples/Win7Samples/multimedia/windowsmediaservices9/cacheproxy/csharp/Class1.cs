//------------------------------------------------------------------------------
//
// Microsoft Windows Media
// Copyright (C) Microsoft Corporation. All rights reserved.
//
// Class1.cs
//
//------------------------------------------------------------------------------
using System;
using System.Runtime.InteropServices;
using Microsoft.Win32;
using System.Threading;
using System.Data;
using System.Xml;
using System.Reflection;
using System.Diagnostics;
using System.Collections;
using System.IO;
using Microsoft.WindowsMediaServices.Interop;

using System.Xml.Serialization;

namespace CacheProxySamplePlugin
{
    class PlaylistItem
    {
        public string	CacheUrl;
        public string	CDLData;
		public int		CacheID;
    };
    public class ContentInfo
    {
        public ContentInfo(string o, string c)
        {
            Debug.WriteLine("ContentInfo::ContentInfo entered");
            OriginUrl = o;
            CacheUrl = c;
            CacheFlags=0;
            ContentSize = 0;
            SubscriptionFlag=0;
            ContentType=0;
            EntityTags = new ArrayList();
            LastModified = System.DateTime.Now;
            ExpirationTime = System.DateTime.Now;
            CacheProxyCallback = null;
            varContext = null;
            lExpiration = 0;
            CDL = new ArrayList();
        }
        public string   OriginUrl, CacheUrl;
        public int      SubscriptionFlag;
        public int      ContentType;
        public DateTime LastModified;
        public DateTime ExpirationTime;
        public int      CacheFlags;
        public System.Int64 ContentSize;
        public ArrayList    EntityTags;
        public string   CDLData;
        public IWMSCacheProxyCallback   CacheProxyCallback;
        public object       varContext;
        public int          lExpiration; // needed for prestuffing
        public ArrayList    CDL; //array of strings for playlist

    };
    public class INSSBufferImpl : INSSBuffer
    {
        public byte[]       Buffer;
        public uint         Length;

        public INSSBufferImpl()
        {
			Debug.WriteLine("INSSBufferImpl::INSSBufferImpl entered");
			Length = 0;
        }

        void INSSBuffer.GetBufferAndLength( IntPtr ppbBuffer, out uint pdwLength)
        {
            Debug.WriteLine("INSSBufferImpl::GetBufferAndLength entered");
            pdwLength=Length;
            INSSBuffer pBuf = this as INSSBuffer;
            pBuf.GetBuffer( ppbBuffer);

        }
        void INSSBuffer.GetBuffer( IntPtr ppbBuffer)
        {
            Debug.WriteLine("INSSBufferImpl::GetBuffer entered");   
            ppbBuffer = Marshal.AllocCoTaskMem((int)Length);
            try
            {
                Marshal.Copy(Buffer,0,ppbBuffer,(int)Length);
            }
            catch(Exception e)
            {
                Debug.WriteLine(e);
            }
        }

        void INSSBuffer.GetLength(out uint pdwLength)
        {
            Debug.WriteLine("INSSBufferImpl::GetLength entered");   
            pdwLength=Length;
        }

        void INSSBuffer.GetMaxLength(out uint pdwLength)
        {
            Debug.WriteLine("INSSBufferImpl::GetMaxLength entered");   
            pdwLength=Length;
        }

        void INSSBuffer.SetLength(uint dwLength)
        {
            Debug.WriteLine("INSSBufferImpl::SetLength entered");   
            Length = dwLength;
        }

        public void SetBuffer(string Buf)
        {
            Debug.WriteLine("INSSBufferImpl::SetBuffer entered");   
            if(Buf!=null)
            {
                try
                {
                    Buffer = Convert.FromBase64String(Buf);
					Length = (uint)Buffer.Length;
                }
                catch(Exception e)
                {
                    Debug.WriteLine(e);
                }
                
            }
			Debug.WriteLine("INSSBufferImpl::SetBuffer ended");  
        }

    }


	class IWMSCacheItemDescriptorImpl : IWMSCacheItemDescriptor
	{
		public IWMSCacheItemDescriptorImpl(IWMSCacheItemCollectionImpl collection,DataRow row)
		{
			Row = row;
			Collection = collection;
		}

		void IWMSCacheItemDescriptor.GetCacheUrl(out string url)
		{
			url = null;
			try
			{
				url = Convert.ToString(Row["CacheUrl"]);
			}
			catch(Exception e)
			{
				Debug.WriteLine(e);
				throw new COMException("NO ERROR",1);
			}
		}

		void IWMSCacheItemDescriptor.GetContentInformation(out IWMSContext contentinfo)
		{
			contentinfo=null;
			try
			{
				ContentInfo ci;
				Collection.Plugin.GetContentInfo(Convert.ToString(Row["OriginUrl"]),out ci);
				Collection.Plugin.GetContentInfoContext(ci,out contentinfo);
			}
			catch(Exception e)
			{
				Debug.WriteLine(e);
				throw new COMException("NO ERROR",1);
			}
		}

		void IWMSCacheItemDescriptor.GetContentSize(out int sizeLow, out int sizeHigh)
		{
			sizeLow = sizeHigh = 0;
			try
			{
				sizeLow = sizeHigh = 0;
			}
			catch(Exception e)
			{
				Debug.WriteLine(e);
				throw new COMException("NO ERROR",1);
			}
		}

		void IWMSCacheItemDescriptor.GetOriginUrl(out string url)
		{
			url = null;
			try
			{
				url = Convert.ToString(Row["OriginUrl"]);
			}
			catch(Exception e)
			{
				Debug.WriteLine(e);
				throw new COMException("NO ERROR",1);
			}
		}
		
		DataRow Row;
		IWMSCacheItemCollectionImpl Collection;
	}

	class IWMSCacheItemCollectionImpl : IWMSCacheItemCollection
	{
		static string strSel = "ContentType%2 = 0";
		public IWMSCacheItemCollectionImpl(CacheProxyPlugin plugin,DataTable dt)
		{
			Plugin = plugin;
			DT = dt;
		}

		void IWMSCacheItemCollection.GetCount(out int count)
		{
			// return the count of the Cache Table entries
			// we have to return the count of only the Cache downloads and have to exclude the 
			// Broadcast specific cache information
			try
			{
				DataRow[] rows = DT.Select(strSel);
				count = rows.Length;
			}
			catch(Exception e)
			{
				count = 0;
				Debug.WriteLine(e);
				throw new COMException("NO ERROR",1);
			}
		}

		void IWMSCacheItemCollection.GetItem(int index, out IWMSCacheItemDescriptor cacheItem)
		{
			cacheItem = null;
			try
			{
				DataRow[] rows = DT.Select(strSel);
				if((index <0)||(index >= rows.Length))
				{
					throw new COMException("NO ERROR",1);
				}
				cacheItem = new IWMSCacheItemDescriptorImpl(this,rows[index]) as IWMSCacheItemDescriptor;
			}
			catch(Exception e)
			{
				Debug.WriteLine(e);
				throw new COMException("NO ERROR",1);
			}
		}

		DataTable DT;
		public CacheProxyPlugin Plugin;
	}

    [Guid("1976C73A-4B3A-4738-8B84-1C5545CE80EC")]
    /// <summary>
    /// Summary description for Class1.
    /// </summary>
    
    public class CacheProxyPlugin : IWMSBasicPlugin, IWMSCacheProxy, IWMSCacheProxyServerCallback, IWMSProxyContext
    {
        static readonly string strGuid = "{1976C73A-4B3A-4738-8B84-1C5545CE80EC}";
        static readonly string strSubKey1 = "SOFTWARE\\Microsoft\\Windows Media\\Server\\RegisteredPlugins\\Cache Proxy\\" + strGuid;
        static readonly string strSubKey2 = "CLSID\\" + strGuid + "\\Properties";
        static readonly string strPluginName = "WMS SDK Sample C# Cache Proxy Plug-in";

        IWMSClassObject         ClassObject;
        IWMSCacheProxyServer    CacheProxyServer;

        // the xml that stores all the configuration details as well as the cache database 
        string                  ConfigPath;
        string                  DatabasePath; // xml file that saves the cache database

        CachePluginAdmin        AdminSettings;
        DataSet                 DS;

        static string GetStringFromNSSBuffer( INSSBuffer NsBuffer)
        {
			Debug.WriteLine("CacheProxyPlugin::GetStringFromNSSBuffer entered");   
            uint bufSize;
            IntPtr pBuf = IntPtr.Zero;
            NsBuffer.GetBufferAndLength(  pBuf, out bufSize );
            byte[] Buf = new byte[bufSize];
            Marshal.Copy(pBuf,Buf,0,(int)bufSize);
            /*
            string s = Marshal.PtrToStringUni( pBuf, (int) bufSize / 2 );
            return s;
            */
            return Convert.ToBase64String(Buf,0,(int)bufSize);
            
        }

        void CreateCacheTable()
        {
			Debug.WriteLine("CacheProxyPlugin::CreateCacheTable entered");   
            // create a table and add columns for it under CacheDatabase Node
            DataTable dt = new DataTable("CachedItems");
			DataColumn col = dt.Columns.Add("OriginUrl",typeof(string));
			dt.Columns.Add("CacheUrl",typeof(string));
            dt.Columns.Add("ContentSize",typeof(System.Int64));
            dt.Columns.Add("ContentType",typeof(int));
            dt.Columns.Add("CacheFlags",typeof(int));
            dt.Columns.Add("ExpirationTime",typeof(System.DateTime));
            dt.Columns.Add("LastModified",typeof(System.DateTime));
            dt.Columns.Add("SubscriptionFlag",typeof(int));
            dt.Columns.Add("EntityTags",typeof(string));
            dt.Columns.Add("CDLData",typeof(string));

			col.Unique = true;
			dt.PrimaryKey = new DataColumn[]{col};
            
            DS = new DataSet("SampleCachePlugin");
            DS.Tables.Add(dt);

			// we add a dummy row to this file so that 
			// next inserts won't get confused by mismatched columns etc
			object[] obj = {"test","test",0,0,0,DateTime.Now,DateTime.Now,0,"test","test"};
			DataRow dr = dt.NewRow();
			dr.ItemArray = obj;
        }

		void CreateReverseProxyTable()
		{
			Debug.WriteLine("CacheProxyPlugin::CreateReverseProxyTable entered");   
			// create a table and add columns for it under CacheDatabase Node
			DataTable dt = new DataTable("ReverseProxyEntries");
			DataColumn col = dt.Columns.Add("OriginUrl",typeof(string));
			dt.Columns.Add("CacheUrl",typeof(string));
			dt.Columns.Add("ContentSize",typeof(System.Int64));
			dt.Columns.Add("ContentType",typeof(int));
			dt.Columns.Add("CacheFlags",typeof(int));
			dt.Columns.Add("ExpirationTime",typeof(System.DateTime));
			dt.Columns.Add("LastModified",typeof(System.DateTime));
			dt.Columns.Add("SubscriptionFlag",typeof(int));
			dt.Columns.Add("EntityTags",typeof(string));
			dt.Columns.Add("CDLData",typeof(string));

			col.Unique = true;
			dt.PrimaryKey = new DataColumn[]{col};
            
			DS = new DataSet("SampleCachePlugin");
			DS.Tables.Add(dt);

			// we add a dummy row to this file so that 
			// next inserts won't get confused by mismatched columns etc
			object[] obj = {"test","test",0,0,0,DateTime.Now,DateTime.Now,0,"test","test"};
			DataRow dr = dt.NewRow();
			dr.ItemArray = obj;
			dt.Rows.Add(dr);	   
		}
        
        void CreatePlaylistTable()
        {
			Debug.WriteLine("CacheProxyPlugin::CreatePlaylistTable entered");   
            DataTable dt2 = new DataTable("PlaylistEntries");
            DataColumn col2 = dt2.Columns.Add("OriginUrl",typeof(string));
            DataColumn col3 = dt2.Columns.Add("CacheID",typeof(string));
            dt2.Columns.Add("CDLData",typeof(string));
            dt2.Columns.Add("CacheUrl",typeof(string));

			dt2.PrimaryKey = new DataColumn[] {col2,col3};

            if(DS==null)
            {
                DS = new DataSet("SampleCachePlugin");
            }
            DS.Tables.Add(dt2);

			object[] obj = {"test","test","test","test"};
			DataRow dr = dt2.NewRow();
			dr.ItemArray = obj;
        }

        // creates the config document if it doesn't exist
        // otherwise loads it from disk
        void LoadConfig()
        {
			Debug.WriteLine("CacheProxyPlugin::LoadConfig entered");   
            Console.WriteLine( "LoadConfig entered" );

            // get the module path and replace extension with xml
            string Path = Assembly.GetExecutingAssembly().Location;
            string Dir = Path.Substring(0,Path.LastIndexOf('.'));
            ConfigPath = Dir + ".xml";
            DatabasePath = Dir+"database"+".xml";

            try
            {
                Console.WriteLine( "Attempting to load config file {0}", ConfigPath );

                // Load configuration from file

                AdminSettings = new CachePluginAdmin();

                // Choose one or the other means of persistence
                if( ! AdminSettings.LoadSettingsFromXmlFile( ConfigPath ) )
                {
                    Console.WriteLine( "Failed to read XML file.  Reading from server namespace." );
                    AdminSettings.LoadSettingsFromServerNamespace();
                }

                IWMSCacheAdmin cacheAdmin = ( IWMSCacheAdmin ) AdminSettings;
                Console.WriteLine( "AdminSettings loaded: diskquota: " + cacheAdmin.DiskQuota );

                MyChangeEventHandler = new CachePluginAdmin.ModifySettingsEventHandler( PersistSettings );
                AdminSettings.RegisterChangeHandler( MyChangeEventHandler );
            }
            catch(Exception e)
            {
                Console.WriteLine( "ASSERT1: " + e.Message + e.StackTrace );
                //load failed, we will treat this as a new file and save it

            }

            try
            {
                DS = new DataSet("SampleCachePlugin");
                DS.ReadXml(DatabasePath, XmlReadMode.ReadSchema);
            }
            catch(Exception e)
            {
                Console.WriteLine( "ASSERT2: " + e.Message + e.StackTrace );
				CreateCacheTable();
				CreatePlaylistTable();
            }
			File.Delete(DatabasePath);
        }
        
        public CacheProxyPlugin()
        {
			Debug.WriteLine("CacheProxyPlugin::CacheProxyPlugin entered");
        }

        [ComRegisterFunctionAttribute]
        public static void RegisterFunction(Type t)
        {
            Debug.WriteLine("CacheProxyPlugin::RegisterFunction entered");   
            try
            {
                RegistryKey regHKLM = Registry.LocalMachine;
                regHKLM = regHKLM.CreateSubKey( strSubKey1 );
                regHKLM.SetValue(null, strPluginName);

                RegistryKey regHKCR = Registry.ClassesRoot;
                regHKCR = regHKCR.CreateSubKey( strSubKey2 );
                regHKCR.SetValue("Name", strPluginName);
                regHKCR.SetValue("Author", "Microsoft Corporation");
                regHKCR.SetValue("CopyRight", "Copyright (C) Microsoft Corporation. All rights reserved.");
				regHKCR.SetValue("Description", "Enables you to configure Cache/Proxy");
                regHKCR.SetValue("UnsupportedLoadTypes", 0x02);
            }
            catch(Exception error)
            {
                Console.WriteLine( "Error Registering DLL. Error " + error.Message );
            }
        }

        [ComUnregisterFunctionAttribute]
        public static void UnRegisterFunction(Type t)
        {
			Debug.WriteLine("CacheProxyPlugin::UnRegisterFunction entered");   
            try
            {
                RegistryKey regHKLM = Registry.LocalMachine;
                regHKLM.DeleteSubKey( strSubKey1 );
            }
            catch( Exception )
            {
                // ignore error for deleting sub key
            }

            try
            {
                RegistryKey regHKCR = Registry.ClassesRoot;
                regHKCR.DeleteSubKeyTree( strSubKey2 );
            }
            catch( Exception )
            {
                // ignore error for deleting sub key
            }
        }

        void IWMSBasicPlugin.InitializePlugin(IWMSContext pServerContext, 
            WMSNamedValues pNamedValues, 
            IWMSClassObject pClassFactory)
        {
			Debug.WriteLine("CacheProxyPlugin::InitializePlugin entered");   
            Console.WriteLine( "IWMSBasicPlugin.InitializePlugin entered" );
            ClassObject = pClassFactory;
            Type t = typeof(IWMSCacheProxyServer);
            Guid guid = t.GUID;
            Object obj;
            pServerContext.GetAndQueryIUnknownValue(WMSDefines.WMS_SERVER_CACHE_MANAGER,WMSDefines.WMS_SERVER_CACHE_MANAGER_ID,
                ref guid,out obj ,0);
            CacheProxyServer = (IWMSCacheProxyServer)obj;

			//load config file
            LoadConfig();
            if( null != AdminSettings )
            {
                AdminSettings.NamedValues = pNamedValues;
            }
        }
        
        void IWMSBasicPlugin.EnablePlugin(ref int flags,ref int heartbeat )
        {       
			Debug.WriteLine("WMSBasicPlugin.EnablePlugin entered");                
        }

        void IWMSBasicPlugin.DisablePlugin()
        {
			Debug.WriteLine("WMSBasicPlugin.DisablePlugin entered");                            
        }

        CachePluginAdmin.ModifySettingsEventHandler MyChangeEventHandler;

        public void PersistSettings( string strName, object objValue )
        {
            if( null != AdminSettings )
            {
                AdminSettings.PersistNameValToServerNamespace( strName, objValue );
            }
        }

        object IWMSBasicPlugin.GetCustomAdminInterface()
        {
            Debug.WriteLine("IWMSBasicPlugin.GetCustomAdminInterface entered");
            IWMSCacheAdmin CacheAdmin = null;
            try
            {
                CacheAdmin = ( IWMSCacheAdmin ) AdminSettings;
            }
            catch( Exception e )
            {
                Console.WriteLine( "EXCEPTION: " + e.Message + e.StackTrace );
                AdminSettings = null;
            }
            finally
            {
                CacheAdmin = null;
            }
            return( ( object ) AdminSettings );
        }

        void IWMSBasicPlugin.OnHeartbeat()
        {
			Debug.WriteLine("IWMSBasicPlugin.OnHeartbeat entered");
        }

        void IWMSBasicPlugin.ShutdownPlugin()
        {
			Debug.WriteLine("IWMSBasicPlugin.ShutdownPlugin entered");
			File.Delete(DatabasePath);
			DS.WriteXml(DatabasePath,XmlWriteMode.WriteSchema);
        }
        
        //
        // IWMSCacheProxy
        //
        void IWMSCacheProxy.QueryCache(string bstrOriginUrl , 
            IWMSContext pUserContext , 
            IWMSCommandContext pCommandContext , 
            IWMSContext pPresentationContext , 
            int lQueryType , 
            IWMSCacheProxyCallback pCallback , 
            object varContext )
        {
			Debug.WriteLine("IWMSCacheProxy.QueryCache entered");
            // we simply return a hard coded URL for the request WMS_CACHE_QUERY_OPEN for now
            int nFlag = (int)WMS_CACHE_QUERY_TYPE_FLAGS.WMS_CACHE_QUERY_OPEN;
            int nOpen = lQueryType & nFlag;
            int nGCI = lQueryType & ((int)WMS_CACHE_QUERY_TYPE_FLAGS.WMS_CACHE_QUERY_GET_CONTENT_INFO);
			int nReverseProxy = lQueryType & ((int)WMS_CACHE_QUERY_TYPE_FLAGS.WMS_CACHE_QUERY_REVERSE_PROXY);
            
            // either open or GCI is called
            // for GCI we don't care about CompareContentInformation
            // if not upto date, we treat as a miss
			if((nOpen!=0)||(nGCI!=0)) 
			{
				// allocate ContentInfoContext and DataContainerObject
				// stuff it with the information
				// and call back
				IWMSContext Context;
				WMS_CACHE_QUERY_RESPONSE Response = WMS_CACHE_QUERY_RESPONSE.WMS_CACHE_QUERY_MISS;
                
				ContentInfo ci;
				//gets allocated here
				GetContentInfo(bstrOriginUrl,out ci);
				ci.CacheProxyCallback = pCallback;
				ci.varContext = varContext;
				GetContentInfoContext(ci,out Context);

				bool bQueryCache = true;
				bool bOnDemand = true;
				if((ci.CacheUrl!=null) && (nReverseProxy==0)) // there is something in the cache
				{
					// if content is not expired, this is a hit
					DateTime now = DateTime.Now;
					Debug.WriteLine(string.Format("Current local time={0}",now));
					//covert to UTC time
					now = now.ToUniversalTime();
					Debug.WriteLine(string.Format("Current UTC time={0}",now));

					Debug.WriteLine(string.Format("Expiration time={0}",ci.ExpirationTime));
					if(ci.ExpirationTime > now)
					{
						if((ci.ContentType & 1 )!=0) // it's a brodcast content
						{
							Response = WMS_CACHE_QUERY_RESPONSE.WMS_CACHE_QUERY_HIT_PLAY_BROADCAST;
							bOnDemand = false;
						}
						else
						{
							Response = WMS_CACHE_QUERY_RESPONSE.WMS_CACHE_QUERY_HIT_PLAY_ON_DEMAND;
							bOnDemand = true;
						}
					}
					else // content appears expired, we will have to call CompareContentInformation
					{
						if(nOpen!=0) // only for open queries
						{
							bQueryCache = false;
						}
					}
				}
                
				if(bQueryCache)
				{
					string CacheUrl = ci.CacheUrl;
					if(bOnDemand)
					{
						CacheUrl = string.Format("file://{0}",ci.CacheUrl);
					}
					pCallback.OnQueryCache( 0,
						Response,
						CacheUrl,
						Context,
						null,
						varContext);
				}
				else
				{
					CacheProxyServer.CompareContentInformation(bstrOriginUrl,Context,pPresentationContext,
						this,null,this,(object)ci);
				}
			}
			else
			{
				// see of this is for event propagation
				int nCacheEvent = lQueryType & ((int)WMS_CACHE_QUERY_TYPE_FLAGS.WMS_CACHE_QUERY_CACHE_EVENT);
				int nLocalEvent = lQueryType & ((int)WMS_CACHE_QUERY_TYPE_FLAGS.WMS_CACHE_QUERY_LOCAL_EVENT);
				if((nCacheEvent | nLocalEvent)!=0)
				{
					// we declare it as a miss
					// and on QCP, we ask the cachemanager to forward it
					WMS_CACHE_QUERY_RESPONSE Response = WMS_CACHE_QUERY_RESPONSE.WMS_CACHE_QUERY_MISS;
					pCallback.OnQueryCache( 0,
						Response,
						bstrOriginUrl,
						null,
						null,
						varContext);
				}

			}
            
            return;
        }

        void IWMSCacheProxy.QueryCacheMissPolicy ( string bstrOriginUrl , 
            IWMSContext pUserContext , 
            IWMSCommandContext pCommandContext , 
            IWMSContext pPresentationContext , 
            object pCachePluginContext , 
            int lQueryType , 
            IWMSCacheProxyCallback pCallback , 
            object varContext )
        {
			Debug.WriteLine("IWMSCacheProxy.QueryCacheMissPolicy entered");
            int nOpenFlag = (int)WMS_CACHE_QUERY_TYPE_FLAGS.WMS_CACHE_QUERY_OPEN;
            int nGCI = lQueryType & ((int)WMS_CACHE_QUERY_TYPE_FLAGS.WMS_CACHE_QUERY_GET_CONTENT_INFO);
			int nReverseProxy = lQueryType & ((int)WMS_CACHE_QUERY_TYPE_FLAGS.WMS_CACHE_QUERY_REVERSE_PROXY);
            ContentInfo ci = new ContentInfo(bstrOriginUrl,null);
            if((nOpenFlag & lQueryType)!=0) // open query
            {
				if(nReverseProxy==0) //normal mode
				{
					// get content information
					ci.CacheProxyCallback = pCallback;
					ci.varContext = varContext;
					CacheProxyServer.GetContentInformation(bstrOriginUrl,pPresentationContext,null,
						null,this,ci);
				}
				else // it's a reverse proxy mode
				{
					// we simply look up our table to see if there is a mapping between the requested
					//Url and the RP url and return it
					// we distinguish the table entry if it's a reverse proxy or not by
					// checking the CDL data
					// we store a special string "ReverseProxy" there for the distinction
					// one can simply add another entry in the table to be safer
					// as there might be some CDL data written as "ReverseProxy" !!
					ContentInfo ciRP = null;
					GetContentInfo(bstrOriginUrl,out ciRP);
					WMS_CACHE_QUERY_MISS_RESPONSE Response = WMS_CACHE_QUERY_MISS_RESPONSE.WMS_CACHE_QUERY_MISS_PLAY_ON_DEMAND;
					if((ciRP.ContentType & 1 )!=0) // it's a brodcast content
					{
						Response = WMS_CACHE_QUERY_MISS_RESPONSE.WMS_CACHE_QUERY_MISS_PLAY_BROADCAST;
					}
					else
					{
						Response = WMS_CACHE_QUERY_MISS_RESPONSE.WMS_CACHE_QUERY_MISS_PLAY_ON_DEMAND;
					}

					IWMSContext ContentInfoContext = null;
					GetContentInfoContext(ci,out ContentInfoContext);
					pCallback.OnQueryCacheMissPolicy(0,Response,ciRP.CacheUrl,null,ContentInfoContext,varContext);

				}

            }
            if((nGCI & lQueryType)!=0) // GCI query from downstream server
            {
                WMS_CACHE_QUERY_MISS_RESPONSE Response = WMS_CACHE_QUERY_MISS_RESPONSE.WMS_CACHE_QUERY_MISS_FORWARD_REQUEST;
                IWMSContext ContentInfoContext = null;
                GetContentInfoContext(ci,out ContentInfoContext);
                pCallback.OnQueryCacheMissPolicy(0,Response,bstrOriginUrl,null,ContentInfoContext,varContext);
            }

            
            if((lQueryType & (int)WMS_CACHE_QUERY_TYPE_FLAGS.WMS_CACHE_QUERY_CACHE_EVENT)!=0)
            {
                pCallback.OnQueryCacheMissPolicy(0,WMS_CACHE_QUERY_MISS_RESPONSE.WMS_CACHE_QUERY_MISS_FORWARD_REQUEST,
					null,this,null,varContext);
            }
            
            return;
        }

        void IWMSCacheProxy.RemoveCacheItem( 
            string stringOriginUrl,
            IWMSCacheProxyCallback pCallback,
            object varContext
            )
        {
			Debug.WriteLine("IWMSCacheProxy.RemoveCacheItem entered");
            RemoveEntryFromDatabase(stringOriginUrl,true);
            pCallback.OnRemoveCacheItem(0,varContext);
            return;
        }

        void IWMSCacheProxy.RemoveAllCacheItems( 
            IWMSCacheProxyCallback pCallback,
            object varContext
            )
        {
			Debug.WriteLine("IWMSCacheProxy.RemoveAllCacheItems entered");
            DataTable dt = DS.Tables["CachedItems"];
            DataRow[] drows = dt.Select();
            foreach(DataRow row in drows)
            {
                string File = Convert.ToString(row["OriginUrl"]);
                RemoveEntryFromDatabase(File,true);
            }
            pCallback.OnRemoveAllCacheItems(0,varContext);
            return;
        }

        void IWMSCacheProxy.AddCacheItem( string bstrOriginUrl , 
            string bstrPrestuffUrl , 
            int lExpiration , 
            int lBandwidth ,                                                                                                                    
            int lRemoteEventFlags ,                                                                                                                                                                          
            IWMSCacheProxyCallback pCallback ,                                                                                                                                                                                              
            object varContext )
        {
			Debug.WriteLine("IWMSCacheProxy.AddCacheItem entered");
            //first remove the entry from database
            ContentInfo ci = new ContentInfo(bstrOriginUrl,bstrPrestuffUrl);
            ci.lExpiration = lExpiration;
            
            IWMSContext pPresentationContext=null;
            Type t = typeof(IWMSContext);
            Guid guid = t.GUID;
            System.IntPtr punk;
            ClassObject.AllocIWMSContext(ref guid,WMS_CONTEXT_TYPE.WMS_PRESENTATION_CONTEXT_TYPE,null,
                out punk);
            pPresentationContext = (IWMSContext)Marshal.GetObjectForIUnknown(punk);

            CacheProxyServer.GetContentInformation(bstrOriginUrl,pPresentationContext,null,
                null,this,ci);
            

            return;
        }

        void IWMSCacheProxy.QuerySpaceForCacheItem( int lContentSizeLow , 
            int lContentSizeHigh , 
            out System.Boolean pvarfSpaceAvail )
        {
			Debug.WriteLine("IWMSCacheProxy.QuerySpaceForCacheItem entered");
            pvarfSpaceAvail = true;
            return;
        }

        void IWMSCacheProxy.FindCacheItem( string bstrOriginUrl , 
            out IWMSCacheItemDescriptor ppCacheItemDescriptor )
        {
			Debug.WriteLine("IWMSCacheProxy.FindCacheItem entered");
            ppCacheItemDescriptor = null;
            
            return;
        }

        void IWMSCacheProxy.CreateCacheItemCollection( out IWMSCacheItemCollection ppCacheItemCollection )
        {
			Debug.WriteLine("IWMSCacheProxy.CreateCacheItemCollection entered");
            IWMSCacheItemCollectionImpl obj = new IWMSCacheItemCollectionImpl(this,DS.Tables["CachedItems"]);
			ppCacheItemCollection = obj as IWMSCacheItemCollection;
            return;
        }

        void IWMSCacheProxy.OnCacheClientClose( int resultHr , 
            IWMSContext pUserContext , 
            IWMSContext pPresentationContext )
        {
			Debug.WriteLine("IWMSCacheProxy.OnCacheClientClose entered");
            return;
        }

		void GetContentInfoFromContext(IWMSContext pContentInfo, ref ContentInfo ci)
        {
			Debug.WriteLine("CacheProxyPlugin::GetContentInfoFromContext entered");
            int nFlag=0;
            int lCacheFlags=0;
            IWMSDataContainerVersion DCV = null;
            Type t = typeof(IWMSDataContainerVersion);
            Guid guid = t.GUID;
            object odcv = (object)DCV;
            pContentInfo.GetAndQueryIUnknownValue(WMSDefines.WMS_CACHE_CONTENT_INFORMATION_DATA_CONTAINER_VERSION,
				WMSDefines.WMS_CACHE_CONTENT_INFORMATION_DATA_CONTAINER_VERSION_ID,ref guid,out odcv, 0);
            DCV = (IWMSDataContainerVersion)odcv;
            DCV.GetCacheFlags(out lCacheFlags);
            nFlag = lCacheFlags & (int)WMS_DATA_CONTAINER_VERSION_CACHE_FLAGS.WMS_DATA_CONTAINER_VERSION_ALLOW_PROXY_CACHING;
                
            ci.CacheFlags = lCacheFlags;
            //content size
            int nLowVal=0, nHighVal=0;
            DCV.GetContentSize(out nLowVal,out nHighVal);
            ci.ContentSize = nLowVal;

            //content type
            int nContentType = 0;
            pContentInfo.GetLongValue(WMSDefines.WMS_CACHE_CONTENT_INFORMATION_CONTENT_TYPE,
				WMSDefines.WMS_CACHE_CONTENT_INFORMATION_CONTENT_TYPE_ID,out nContentType,0);
            ci.ContentType = nContentType;

            //expiration time
            DateTime time = new DateTime(0);

			//last modified
			DCV.GetLastModifiedTime(out time);
			ci.LastModified = time;
			
			//following call throws exception
			// when this is a bpp sourcing from encoder
			// so we don't call it based on the lastmodified time
			
			if(((ci.ContentType &1)!=0) && time.Year==1899)
			{
			}
			else
			{
				DCV.GetExpirationTime(out time);
				try
				{
					ci.ExpirationTime = time;
				}
				catch(Exception e)
				{
					//if this call fails, we just ignore
					Debug.WriteLine(e);
				}
			}
            //Subscription flags
            int nSubFlag=0;
            pContentInfo.GetLongValue(WMSDefines.WMS_CACHE_CONTENT_INFORMATION_EVENT_SUBSCRIPTIONS
				,WMSDefines.WMS_CACHE_CONTENT_INFORMATION_EVENT_SUBSCRIPTIONS_ID,out nSubFlag,0);
            ci.SubscriptionFlag = nSubFlag;

			//clear previous entries
			ci.EntityTags.Clear();
            int nTagCount = 0;
            DCV.GetEntityTagCount(out nTagCount);
            for(int i=0;i<nTagCount;i++)
            {
                string tag;
                DCV.GetEntityTag(i,out tag);
                ci.EntityTags.Insert(i,tag);
            }
        }

        //
        // IWMSCacheProxyServerCallback
        //
        void IWMSCacheProxyServerCallback.OnGetContentInformation(
            int lHr,
            IWMSContext pContentInfo,
            object varContext
            )
        {
			Debug.WriteLine("IWMSCacheProxyServerCallback::OnGetContentInformation entered");
            int nFlag=0;
            ContentInfo ci = (ContentInfo)varContext;
			bool bAddCacheItem = false;
			int nRetHr = lHr;
            // if we are called from the result of AddCacheItem, the lExpiration variable won't be 0
            // and we would simply call DownloadContent
            if(ci.lExpiration!=0) // Call result of AddCacheIrtem
            {
                bAddCacheItem=true;
                lHr=0;
            }
            // check the flags and if allowed
            // download the content and save it locally
			if(lHr==0) // would be E_ACCESSDENIED or something
				// if we didn't get content info
			{
				// if we are not allowed to cache the content
				// we simply set up proxy on demand datapath
				// else we store the information and ask cache manager
				// to download the content
				GetContentInfoFromContext(pContentInfo, ref ci);
				nFlag = ci.CacheFlags & (int)WMS_DATA_CONTAINER_VERSION_CACHE_FLAGS.WMS_DATA_CONTAINER_VERSION_ALLOW_PROXY_CACHING;
                
				if((nFlag!=0)||(bAddCacheItem))
				{
					// proxying is allowed
					// let's get all the content information
					// and ask the cache manager to download the content
					if((!bAddCacheItem)||(bAddCacheItem && (ci.CacheUrl==null)))
					{
						ci.CacheUrl = "c:\\WMSCache\\test";
					}
                    
					//we check first from the database if the dowloading is already in progress 
					// and cll for downloading the content only if there is no active
					// downloading going on
					if(!IsDownloadInProgress(ci))
					{
						// add to the database that we are downloading
						AddForDownload(ci);
						CacheProxyServer.DownloadContent(ci.OriginUrl,ci.CacheUrl,0,0,0,0,this,
							null,this,ci);
					}
					else
					{
						Debug.WriteLine("download arlready in progress");
					}
				}
				// if this is a broadcast content, we are not going to download it
				// so we simply store the content information in the database with 
				// empty CacheUrl, so when next request comes, we get a hit so 
				// we can reply accordingly
				if((ci.ContentType & 1)!=0)
				{
					UpdateTable(ci);
				}
			}
			else
			{
				Debug.WriteLine(string.Format("HRESULT for CacheProxyPlugin::IWMSCacheProxyServerCallback.OnGetContentInformation = {0}",
					lHr.ToString("X")));
				if(lHr==-2147024891)
					nRetHr=0;
			}

            // check if stream splitting is allowed
            // if so we ask cache manager to set up a brodcast connection with the player
            // otherwise we go for OD
            WMS_CACHE_QUERY_MISS_RESPONSE MissResponse = WMS_CACHE_QUERY_MISS_RESPONSE.WMS_CACHE_QUERY_MISS_PLAY_ON_DEMAND;
            nFlag = ci.CacheFlags & (int)WMS_DATA_CONTAINER_VERSION_CACHE_FLAGS.WMS_DATA_CONTAINER_VERSION_ALLOW_STREAM_SPLITTING;
            if(nFlag!=0)
            {
                MissResponse = WMS_CACHE_QUERY_MISS_RESPONSE.WMS_CACHE_QUERY_MISS_PLAY_BROADCAST;
            }

            // ask the server to setup a proxy on-demand
            if(ci.CacheProxyCallback!=null)
            {
                ci.CacheProxyCallback.OnQueryCacheMissPolicy((int)nRetHr,MissResponse
                    ,ci.OriginUrl,null,pContentInfo,ci.varContext);
            }
            return;
        }


		// query the database to see if there is any row matching the OriginUrl
		bool IsDownloadInProgress(ContentInfo ci)
		{
			Debug.WriteLine("CacheProxyPlugin::IsDownloadInProgress entered");
			try
			{
				DataTable dt = DS.Tables["CachedItems"];
				string filexpr = string.Format("OriginUrl = '{0}'",ci.OriginUrl);
				DataRow[] drows = dt.Select(filexpr);
				return (drows.Length!=0);
			}
			catch(Exception e)
			{
				Debug.WriteLine(e);
				return false;
			}
		}

		void AddForDownload(ContentInfo ci)
		{
			Debug.WriteLine("CacheProxyPlugin::AddForDownload entered");
			// update the database
            object[] obj = {ci.OriginUrl,"Downloading",null,null,null,
                               null,null,null,null,null
                           };
			try
			{
				DataTable dt = DS.Tables["CachedItems"];
				DataRow dr = dt.NewRow();
				dr.ItemArray = obj;
				dt.Rows.Add(dr);
			}
			catch(Exception e)
			{
				Debug.WriteLine(e);
			}
		}

        void RemoveEntryFromDatabase(ContentInfo ci,bool bDeleteFile)
        {
			Debug.WriteLine("CacheProxyPlugin::RemoveEntryFromDatabase entered");
			try
			{
				DataTable dt = DS.Tables["CachedItems"];
				string filexpr = string.Format("OriginUrl = '{0}'",ci.OriginUrl);
				DataRow[] drows = dt.Select(filexpr);
				if(drows.Length==0)
				{
					return;
				}
				// should ideally be only one row
				foreach(DataRow row in drows)
				{
					if(bDeleteFile)
					{
						string file = Convert.ToString(row["CacheUrl"]);
						File.Delete(file);
					}
					dt.Rows.Remove(row);
				}

				// now delete all playlist entries if available
				DataTable dtPlaylist = DS.Tables["PlaylistEntries"];
				string filter = string.Format("OriginUrl='{0}'",ci.OriginUrl);
				DataRow[] rows = dtPlaylist.Select(filter);
				foreach(DataRow row in rows)
				{
					if(bDeleteFile)
					{
						string file = Convert.ToString(row["CacheUrl"]);
						File.Delete(file);
					}
					dt.Rows.Remove(row);
				}
			}
			catch(Exception e)
			{
				Debug.WriteLine(e);
			}
        }
        void RemoveEntryFromDatabase(string OriginUrl,bool bDeleteFile)
        {
			ContentInfo ci = new ContentInfo(OriginUrl,null);
			RemoveEntryFromDatabase(ci,bDeleteFile);
        }

		void SavePlaylist(ContentInfo ci)
		{
			Debug.WriteLine("CacheProxyPlugin::SavePlaylist entered");
			//create a wsx file with filename same as first cache entry of the playlist file
			// create wsx with following format
			//<?wsx version="1.0"?>
			//<smil>
			//  <media src="{filename1}" />
			//  <media src="{filename2}" />
			//</smil>
			if(ci==null)
			{
				return;
			}
			if(ci.CDL.Count==0)
			{
				return;
			}
			string filename=string.Format("{0}.wsx",((PlaylistItem)ci.CDL[0]).CacheUrl);
			XmlDocument dom = new XmlDocument();
			//create an empty playlist
			dom.LoadXml("<?wsx version=\"1.0\"?><smil/>");
			//get the smil node
			XmlElement smilnode = dom.DocumentElement;

			int i=0;
			foreach(Object obj in ci.CDL)
			{
				((PlaylistItem)obj).CacheID = i;
				string file = ((PlaylistItem)obj).CacheUrl;
				
				XmlElement elem = dom.CreateElement("media");
				XmlAttribute attr = dom.CreateAttribute("src");
				attr.Value = file;
				elem.Attributes.Append(attr);

				attr = dom.CreateAttribute("CacheID");
				attr.Value = i.ToString();
				elem.Attributes.Append(attr);

				smilnode.AppendChild(elem);
				i++;
			}
			
			// now save the file
			string filesaved = string.Format("{0}.wsx",((PlaylistItem)ci.CDL[0]).CacheUrl);
			dom.Save(filesaved);

			// set the cache URL to this file for cache hit
			ci.CacheUrl = filesaved;
		}

		// based on the response
		// we reply as a hit or miss
		// and update database accordingly

		void IWMSCacheProxyServerCallback.OnCompareContentInformation(
			int lHr,
			WMS_CACHE_VERSION_COMPARE_RESPONSE CompareResponse,
			IWMSContext pNewContentInfo,
			object varContext
			)
		{
			Debug.WriteLine("CacheProxyPlugin::IWMSCacheProxyServerCallback.OnCompareContentInformation entered");
			WMS_CACHE_QUERY_RESPONSE Response = WMS_CACHE_QUERY_RESPONSE.WMS_CACHE_QUERY_MISS;
			ContentInfo ci = (ContentInfo)varContext;
			// we use this content info  to call back by default
			// it changes if the CI in our database is valid
			IWMSContext pContenInfo = pNewContentInfo;
			// if the call failed, we have to go for protocol rollover etc
			if(lHr==0)
			{
				switch (CompareResponse)
				{
					case WMS_CACHE_VERSION_COMPARE_RESPONSE.WMS_CACHE_VERSION_CACHE_STALE:
					{
						RemoveEntryFromDatabase(ci,true);
					}
						break;

					case WMS_CACHE_VERSION_COMPARE_RESPONSE.WMS_CACHE_VERSION_CACHE_UP_TO_DATE:
					{
						if((ci.ContentType & 1 )!=0) // it's a brodcast content
						{
							Response = WMS_CACHE_QUERY_RESPONSE.WMS_CACHE_QUERY_HIT_PLAY_BROADCAST;
						}
						else
						{
							Response = WMS_CACHE_QUERY_RESPONSE.WMS_CACHE_QUERY_HIT_PLAY_ON_DEMAND;
						}
						// update the database with the new information
						RemoveEntryFromDatabase(ci,false);
						GetContentInfoFromContext(pNewContentInfo,ref ci);
						UpdateTable(ci);

						GetContentInfoContext(ci,out pContenInfo);
					}
						break;
					case WMS_CACHE_VERSION_COMPARE_RESPONSE.WMS_CACHE_VERSION_FAIL_TO_CHECK_VERSION:
					{
					}
						break;

					default:
						break;
				}

				Debug.WriteLine("CacheProxyPlugin::IWMSCacheProxyServerCallback.OnCompareContentInformation -> calling OnQueryCache");

				// crude test...look up if we have :// embedded in the URL, if not prefix it with file://
				// otherwise just let it go
				int nIndex = ci.CacheUrl.IndexOf("://");
				string strCacheUrl = ci.CacheUrl;
				if(nIndex==-1)
				{
					strCacheUrl = string.Format("file://{0}",ci.CacheUrl);
				}
				
				ci.CacheProxyCallback.OnQueryCache(0,Response,strCacheUrl,pContenInfo,
					null,ci.varContext);

			}

			return;
		}

		void AddCDLToContext(IWMSContext pContentInfo,ContentInfo ci)
		{
			IWMSDataContainerVersion DCV = null;
			Type t = typeof(IWMSDataContainerVersion);
			Guid guid = t.GUID;
			object odcv = (object)DCV;
			/*
			pContentInfo.GetAndQueryIUnknownValue("WMS_CACHE_CONTENT_INFORMATION_DATA_CONTAINER_VERSION",
				3,ref guid,out odcv, 0);
			*/
			pContentInfo.GetAndQueryIUnknownValue(WMSDefines.WMS_CACHE_CONTENT_INFORMATION_DATA_CONTAINER_VERSION,
				WMSDefines.WMS_CACHE_CONTENT_INFORMATION_DATA_CONTAINER_VERSION_ID,ref guid,out odcv, 0);
			DCV = (IWMSDataContainerVersion)odcv;

			if(ci.EntityTags!=null)
			{
				foreach(string str in ci.EntityTags)
				{
					DCV.SetEntityTag(str);
				}
			}
		}

		void IWMSCacheProxyServerCallback.OnDownloadContentProgress(
			int lHr,
			WMS_RECORD_PROGRESS_OPCODE opCode,
			IWMSContext pArchiveContext,
			object varContext
			)
		{
			Debug.WriteLine("CacheProxyPlugin::IWMSCacheProxyServerCallback.OnDownloadContentProgress entered");
			if(lHr==0)
			{
				if(opCode==WMS_RECORD_PROGRESS_OPCODE.WMS_RECORD_PROGRESS_ARCHIVE_STARTED)
				{
					INSSBuffer NSBuffer=null;
					Guid guid = typeof(INSSBuffer).GUID;
					Object obj;
					pArchiveContext.GetAndQueryIUnknownValue(WMSDefines.WMS_ARCHIVE_CONTENT_DESCRIPTION_LIST_BUFFER,
						WMSDefines.WMS_ARCHIVE_CONTENT_DESCRIPTION_LIST_BUFFER_ID,ref guid,out obj ,0);
					NSBuffer = (INSSBuffer)obj;

					// get the file name
					string CacheFile;
					pArchiveContext.GetStringValue(WMSDefines.WMS_ARCHIVE_FILENAME,
						WMSDefines.WMS_ARCHIVE_FILENAME_ID,out CacheFile,0);

					ContentInfo ci = (ContentInfo)varContext;
                    
					string s = GetStringFromNSSBuffer(NSBuffer);
					if((ci.ContentType & 2)==0) // not a playlist
					{
						ci.CDLData = s;
						ci.CacheUrl = CacheFile;
					}
					else
					{
						PlaylistItem Item = new PlaylistItem();
						Item.CacheUrl = CacheFile;
						Item.CDLData = s;
						ci.CDL.Add((object)Item);
					}
				}
			}
			return;
		}

		void IWMSCacheProxyServerCallback.OnDownloadContentFinished(
			int lHr,
			object[] psaArchiveContexts,
			object varContext
			)
		{
			Debug.WriteLine("CacheProxyPlugin::IWMSCacheProxyServerCallback.OnDownloadContentFinished entered");
			if(lHr==0) // download succeeded
			{
				ContentInfo ci = (ContentInfo)varContext;
				// server might have renamed the specified file if there is a name collision
				// so we get the actual name of the file from archive context that we get
				if(psaArchiveContexts.Length!=0) 
				{
					if((ci.ContentType & 2)==0) // it's not a playlist
					{
						IWMSContext ArchiveContext = (IWMSContext)psaArchiveContexts.GetValue(0);
						// get the file name
						string CacheFile;
						ArchiveContext.GetStringValue(WMSDefines.WMS_ARCHIVE_FILENAME,
							WMSDefines.WMS_ARCHIVE_FILENAME_ID,out CacheFile,0);
						ci.CacheUrl = CacheFile;
						
					}
					else
					{
						SavePlaylist(ci);
					}
					UpdateTable(ci);
				}
				
			}
			else
			{
				Debug.WriteLine(string.Format("HRESULT for CacheProxyPlugin::IWMSCacheProxyServerCallback.OnDownloadContentFinished = {0}",
					lHr.ToString("X")));
			}
			return;
		}

        void UpdateTable(ContentInfo ci)
        {
			Debug.WriteLine("CacheProxyPlugin::UpdateTable entered");
            string strTags=null;
            for(int i=0;(ci.EntityTags!=null)&&(i<ci.EntityTags.Count);i++)
            {
                strTags += ci.EntityTags[i];
                strTags += "#";
            }

            if(strTags==null)
            {
                strTags="Test";
            }
            // update the database
            object[] obj = {ci.OriginUrl,ci.CacheUrl,ci.ContentSize,ci.ContentType,ci.CacheFlags,
                               ci.ExpirationTime,ci.LastModified,ci.SubscriptionFlag
                               ,strTags,ci.CDLData
                           };
            try
            {
                DataTable dt = DS.Tables["CachedItems"];
				DataRow dr = dt.NewRow();
                dr.ItemArray = obj;

                
				// we remove the previous entry which would most likely be due to the Add For Download call
				RemoveEntryFromDatabase(ci,false);
                dt.Rows.Add(dr);

                // if this is a playlist file, we also need to persist the CDL information for the playlist
                DataTable dt2 = DS.Tables["PlaylistEntries"];
                
                foreach(PlaylistItem Item in ci.CDL)
                {
                    //add a row for each CDL data
                    DataRow dr2 = dt2.NewRow();
                    object[] objPlaylist = 
                        {
                            ci.OriginUrl,Item.CacheID ,Item.CDLData,Item.CacheUrl
                        };
                    dr2.ItemArray = objPlaylist;
                    dt2.Rows.Add(dr2);
                }

            }
            catch(System.Exception e)
            {
                Debug.WriteLine(e);
            }
        }

        void IWMSCacheProxyServerCallback.OnCancelDownloadContent(
            int lHr,
            object varContext
            )
        {
			Debug.WriteLine("CacheProxyPlugin::IWMSCacheProxyServerCallback.OnCancelDownloadContent entered");
            return;
        }
        
        //[PreserveSig] can be used as well
        void IWMSProxyContext.FindProxyForURL(IWMSContext pUserContext, string bstrUrl, out string pbstrProxyServer, out uint pdwProxyPort)
        {
			Debug.WriteLine("CacheProxyPlugin::IWMSProxyContext.FindProxyForURL entered");
            throw new COMException("NO ERROR",1);
        }

        //[PreserveSig] can be used as well
        void IWMSProxyContext.GetCredentials(IWMSContext pUserContext, string bstrRealm, string bstrUrl, out string pbstrName, out string pbstrPassword)
        {
			Debug.WriteLine("CacheProxyPlugin::IWMSProxyContext.GetCredentials entered");
            pbstrName="";
            pbstrPassword="";
            throw new COMException("NO ERROR",1);
        }

        // selects the row for OriginUrl
        // creates a ContentInfo structure based on it
        public void GetContentInfo(string OriginUrl, out ContentInfo ci)
        {
			Debug.WriteLine("CacheProxyPlugin::GetContentInfo entered");
			ci = new ContentInfo(OriginUrl,null);
			try
			{
				DataTable dt = DS.Tables["CachedItems"];
				if(dt==null)
				{
					return;
				}
            
				string filexpr = string.Format("OriginUrl = '{0}'",OriginUrl);
             
				DataRow[] drows = dt.Select(filexpr);
				if(drows.Length==0)
				{
					return;
				}

				// FOR REVERSE PROXY DATA, WE HAVE TO HAVE THE TYPE OF CONTENT
				// STORED HERE AS WELL FOR THE APPROPRIATE RESPONSE

				ci.ContentType = Convert.ToInt32(drows[0]["ContentType"]);
				if((ci.ContentType & 1)!=0)
				{
					ci.CacheUrl = OriginUrl;
				}
				else
				{
					ci.CacheUrl = Convert.ToString(drows[0]["CacheUrl"]);
				}

				// if this string is ReverseProxy, then that means we are looking at a ReverseProxy
				// entry, so we should really return from here to force a cache miss
				ci.CDLData = Convert.ToString((drows[0]["CDLData"]));
				if(ci.CDLData=="ReverseProxy")
				{
					return;
				}
            
				// we simply assume there will be only one row returned as we enforce
				// primary key constraint

				ci.SubscriptionFlag = Convert.ToInt32((drows[0]["SubscriptionFlag"]));
				ci.LastModified = Convert.ToDateTime((drows[0]["LastModified"]));
				ci.ExpirationTime = Convert.ToDateTime(drows[0]["ExpirationTime"]);
				ci.ContentSize = Convert.ToInt64(drows[0]["ContentSize"]);
				
				// get the entity tags

				string strTags=Convert.ToString(drows[0]["EntityTags"]);
				int i=0, j=0;
				while(true)
				{
					int k = strTags.IndexOf('#',i);
					if(k<0)
					{
						break;
					}
					string str = strTags.Substring(i,k);
					ci.EntityTags.Insert(j,str);
					i=k+1;
					j++;
				}
			}
			catch(Exception e)
			{
				Debug.WriteLine(e);
				
			}
        }

        
		public void GetContentInfoContext(ContentInfo ci, out IWMSContext Context)
        {
			Debug.WriteLine("CacheProxyPlugin::GetContentInfoContext entered");
            System.IntPtr punk;
            Type t = typeof(IWMSContext);
            System.Guid guid = t.GUID;
            ClassObject.AllocIWMSContext(ref guid,
                WMS_CONTEXT_TYPE.WMS_CACHE_CONTENT_INFORMATION_CONTEXT_TYPE
                ,null, out punk);
            Context = (IWMSContext)Marshal.GetObjectForIUnknown(punk);
                
            t = typeof(IWMSDataContainerVersion);
            guid = t.GUID;
            ClassObject.CreateInstance(ref guid, out punk);
            IWMSDataContainerVersion DataContainer = 
                (IWMSDataContainerVersion)Marshal.GetObjectForIUnknown(punk);
            Context.SetIUnknownValue(WMSDefines.WMS_CACHE_CONTENT_INFORMATION_DATA_CONTAINER_VERSION,
				WMSDefines.WMS_CACHE_CONTENT_INFORMATION_DATA_CONTAINER_VERSION_ID,DataContainer,0);

            Context.SetLongValue(WMSDefines.WMS_CACHE_CONTENT_INFORMATION_CONTENT_TYPE,
				WMSDefines.WMS_CACHE_CONTENT_INFORMATION_CONTENT_TYPE_ID,ci.ContentType,0);
            Context.SetLongValue(WMSDefines.WMS_CACHE_CONTENT_INFORMATION_EVENT_SUBSCRIPTIONS,
				WMSDefines.WMS_CACHE_CONTENT_INFORMATION_EVENT_SUBSCRIPTIONS_ID,ci.SubscriptionFlag,0);      
            DataContainer.SetLastModifiedTime(ci.LastModified);
            DataContainer.SetExpirationTime(ci.ExpirationTime);
            DataContainer.SetCacheFlags(ci.CacheFlags);
            DataContainer.SetContentSize((int)ci.ContentSize,0);

            if(ci.EntityTags!=null)
            {
                foreach(string str in ci.EntityTags)
                {
                    DataContainer.SetEntityTag(str);
                }
            }

			guid = typeof(IWMSContext).GUID;
			if((ci.ContentType & 2) == 0) // not a playlist
			{
				//create a CDL context and make it point to the INSSBuffer that we persisted
				ClassObject.AllocIWMSContext(ref guid,
					WMS_CONTEXT_TYPE.WMS_UNKNOWN_CONTEXT_TYPE
					,null, out punk);
				IWMSContext CDLContext = (IWMSContext)Marshal.GetObjectForIUnknown(punk);
				INSSBufferImpl Buffer = new INSSBufferImpl();
				Buffer.SetBuffer(ci.CDLData);
				INSSBuffer pBuffer = Buffer as INSSBuffer;
				CDLContext.SetIUnknownValue("CDL",0,(object)pBuffer,2);

				Context.SetIUnknownValue(WMSDefines.WMS_CACHE_CONTENT_INFORMATION_CONTENT_DESCRIPTION_LISTS,
					WMSDefines.WMS_CACHE_CONTENT_INFORMATION_CONTENT_DESCRIPTION_LISTS_ID,(object)CDLContext,0);
			}
			else
			{
				// create CDL for each file of the playlist
				// get the list of entries from PlaylistItem table corresponding to this Origin URL
				// for each of the entry
				// create CDL data and stuff it with the Context
				// query would be like select * from PlaylistEntries where OriginUrl=ci.OriginUrl
				DataTable dtPlaylist = DS.Tables["PlaylistEntries"];
				string filter = string.Format("OriginUrl='{0}'",ci.OriginUrl);
				DataRow[] rows = dtPlaylist.Select(filter);

				ClassObject.AllocIWMSContext(ref guid,
					WMS_CONTEXT_TYPE.WMS_UNKNOWN_CONTEXT_TYPE
					,null, out punk);
				IWMSContext CDLContext = (IWMSContext)Marshal.GetObjectForIUnknown(punk);
				foreach( DataRow row in rows)
				{
					
					INSSBufferImpl Buffer = new INSSBufferImpl();
					Buffer.SetBuffer(Convert.ToString(row["CDLData"]));
					INSSBuffer pBuffer = Buffer as INSSBuffer;
					CDLContext.SetIUnknownValue(Convert.ToString(row["CacheID"]),0,(object)pBuffer,2);
				}
				Context.SetIUnknownValue(WMSDefines.WMS_CACHE_CONTENT_INFORMATION_CONTENT_DESCRIPTION_LISTS
					,WMSDefines.WMS_CACHE_CONTENT_INFORMATION_CONTENT_DESCRIPTION_LISTS_ID
					,(object)CDLContext,0);

			}
        }
        bool IsContentCached(string Url, out string ReturnURL, out IWMSContext Context, out int ContentType)
        {
			Debug.WriteLine("CacheProxyPlugin::IsContentCached entered");
            ContentType=0;
            ReturnURL=null;

            System.IntPtr punk;
            Type t = typeof(IWMSContext);
            System.Guid guid = t.GUID;
            ClassObject.AllocIWMSContext(ref guid,
                WMS_CONTEXT_TYPE.WMS_CACHE_CONTENT_INFORMATION_CONTEXT_TYPE
                ,null, out punk);
            Context = (IWMSContext)Marshal.GetObjectForIUnknown(punk);
                
            t = typeof(IWMSDataContainerVersion);
            guid = t.GUID;
            ClassObject.CreateInstance(ref guid, out punk);
            IWMSDataContainerVersion DataContainer = 
                (IWMSDataContainerVersion)Marshal.GetObjectForIUnknown(punk);

            Context.SetIUnknownValue(WMSDefines.WMS_CACHE_CONTENT_INFORMATION_DATA_CONTAINER_VERSION,
				WMSDefines.WMS_CACHE_CONTENT_INFORMATION_DATA_CONTAINER_VERSION_ID,DataContainer,0);

            // search the database if the specified URL exists
            DataTable dt = DS.Tables["CachedItems"];
            if(dt==null)
            {
                return false;
            }
            
            string filexpr = string.Format("OriginUrl = '{0}'",Url);
             
            DataRow[] drows = dt.Select(filexpr);
            if(drows.Length==0)
            {
                return false;
            }
            
            // we simply assume there will be only one row returned as we enforce
            // primary key constraint

            object ob = (drows[0]["ContentType"]);
            int nVal = Convert.ToInt32(ob);
            Context.SetLongValue(WMSDefines.WMS_CACHE_CONTENT_INFORMATION_CONTENT_TYPE,
				WMSDefines.WMS_CACHE_CONTENT_INFORMATION_CONTENT_TYPE_ID,nVal,0);
            ContentType = nVal;
            
            

            nVal = Convert.ToInt32((drows[0]["SubscriptionFlag"]));
            Context.SetLongValue(WMSDefines.WMS_CACHE_CONTENT_INFORMATION_EVENT_SUBSCRIPTIONS,
				WMSDefines.WMS_CACHE_CONTENT_INFORMATION_EVENT_SUBSCRIPTIONS_ID,nVal,0);     

            System.DateTime date = Convert.ToDateTime((drows[0]["LastModified"]));
            DataContainer.SetLastModifiedTime(date);

            date = Convert.ToDateTime(drows[0]["ExpirationTime"]);
            DataContainer.SetExpirationTime(date);

            nVal = Convert.ToInt32((drows[0]["SubscriptionFlag"]));
            DataContainer.SetCacheFlags(nVal);

            System.Int64 n64Val = Convert.ToInt64(drows[0]["ContentSize"]);
            DataContainer.SetContentSize((int)n64Val,0);
            /*
                        string tag = Convert.ToString(drows[0]["EntityTags"]);
                        DataContainer.SetEntityTag(tag);
            */

            string cacheurl=null;
            try
            {
                cacheurl = Convert.ToString(drows[0]["CacheUrl"]);
            }
            catch(Exception e)
            {
				Debug.WriteLine(e);
            }

            // there is no CacheUrl for broadcast content, so we should reply with 
            // the OriginUrl
            if((ContentType & 1 )!=0) // it's a brodcast content
            {
                ReturnURL = Url;
            }
            else
            {
                ReturnURL = string.Format("file://{0}",cacheurl);
            }
            
            return true;
            
        }

    }
}

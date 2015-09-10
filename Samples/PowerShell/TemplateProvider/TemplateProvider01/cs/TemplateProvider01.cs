// <copyright file="TemplateProvider.cs" company="Microsoft Corporation">
// Copyright (c) 2009 Microsoft Corporation. All rights reserved.
// </copyright>
// DISCLAIMER OF WARRANTY: The software is licensed “as-is.” You 
// bear the risk of using it. Microsoft gives no express warranties, 
// guarantees or conditions. You may have additional consumer rights 
// under your local laws which this agreement cannot change. To the extent 
// permitted under your local laws, Microsoft excludes the implied warranties 
// of merchantability, fitness for a particular purpose and non-infringement.

using System;
using System.Collections.ObjectModel;
using System.Security.AccessControl;
using System.Management.Automation;
using System.Management.Automation.Provider;

namespace Microsoft.Samples.Management.Automation
{
    /// <summary>
    /// This is a template for a provider that hooks into the Windows
    /// PowerShell namespaces.  It contains all possible provider
    /// overrides and interfaces.  A provider developer should be able to copy
    /// this file, change its name, delete those interfaces and methods the 
    /// provider doesn't need to implement/override and be on their way.
    ///
    /// The ProviderDeclaration attribute signifies to the Microsoft Command
    /// Shell that this class implements a CmdletProvider.  The first
    /// parameter is the default friendly name for the provider. The second
    /// parameter is the provider supported capabilities.
    /// </summary>
    [CmdletProvider( "Template", ProviderCapabilities.None )]
    public class TemplateProvider :
        NavigationCmdletProvider,
        IPropertyCmdletProvider,
        IContentCmdletProvider,
        IDynamicPropertyCmdletProvider,
        ISecurityDescriptorCmdletProvider
    {
        // CmdletProvider is a base class for a provider that hooks into the
        // namespace. This class is at the top of the hirerchy of all 
        // classes that can be used to derive from to create a new 
        // Windows PowerShell provider. Although it is possible to create a 
        // CmdletProvider by deriving from this class, developers are strongly 
        // encouraged to derive from either of ItemCmdletProvider, 
        // ContainerCmdletProvider or NavigationCmdletProvider. 

        #region CmdletProvider Overrides

        /// <summary>
        /// Gives the provider the opportunity to initialize itself.
        /// </summary>
        /// 
        /// <param name="providerInfo">
        /// The information about the provider that is being started.
        /// </param>
        /// 
        /// <returns>
        /// Either the providerInfo that was passed or a derived class
        /// of ProviderInfo that was initialized with the provider information
        /// that was passed.
        /// </returns>
        /// 
        /// <remarks>
        /// The default implementation returns the ProviderInfo instance that
        /// was passed.To have session state maintain persisted data on behalf
        /// of the provider, the provider should derive from 
        /// <see cref="System.Management.Automation.ProviderInfo"/>
        /// and add any properties or methods for the data it wishes to persist.
        /// When Start gets called the provider should construct an instance of 
        /// its derived ProviderInfo using the providerInfo that is passed in 
        /// and return that new instance.
        /// </remarks>
        protected override ProviderInfo Start( ProviderInfo providerInfo )
        {
            return providerInfo;
        } // Start

        /// <summary>
        /// Gets an object that defines the additional parameters for the
        /// Start implementation for a provider.
        /// </summary>
        /// 
        /// <returns>
        /// Overrides of this method should return an object that has properties
        /// and fields decorated with parsing attributes similar to a cmdlet 
        /// class or a 
        /// <see cref="System.Management.Automation.PseudoParameterDictionary"/>.
        /// The default implemenation returns null.
        /// </returns>
        protected override object StartDynamicParameters()
        {
            return null;
        } //StartDynamicParameters

        /// <summary>
        /// Uninitialize the provider. Called by Session State when the provider
        /// is being removed.
        /// </summary>
        /// 
        /// <remarks>
        /// This is the time to free up any resources that the provider
        /// was using. The default implementation in CmdletProvider does 
        /// nothing.
        /// </remarks>
        protected override void Stop()
        {
        } //Stop

        #endregion CmdletProvider Overrides

        // DriveCmdletProvider is a base class for a provider that hooks into
        // the Windows PowerShell namespace and that can be exposed
        // through Windows PowerShell drives. A drive name can be a valid sequence of 
        // alpha numneric characters. Although it is possible to create a
        // CmdletProvider that can be exposed through a drive by directly
        // deriving from this class, developers are strongly encouraged to
        // derive from either of ItemCmdletProvider, ContainerCmdletProvider
        // or NavigationCmdletProvider.

        #region DriveCmdletProvider Overrides

        /// <summary>
        /// Gives the provider an opportunity to validate the drive that is
        /// being added.  It also allows the provider to modify parts of the
        /// PSDriveInfo object.  This may be done for performance or
        /// reliability reasons or to provide extra data to all calls using
        /// the Drive.
        /// </summary>
        /// 
        /// <param name="drive">
        /// The proposed new drive.
        /// </param>
        /// 
        /// <returns>
        /// The new drive that is to be added to the Windows PowerShell namespace.  This
        /// can either be the same <paramref name="drive"/> object that
        /// was passed in or a modified version of it. The default 
        /// implementation returns the drive that was passed.
        /// </returns>
        /// 
        /// <remarks>
        /// This method gives the provider an opportunity to associate 
        /// provider specific information with a drive. This is done by 
        /// deriving a new class from 
        /// <see cref="System.Management.Automation.PSDriveInfo"/>
        /// and adding any properties, methods, or fields that are necessary. 
        /// When this method gets called, the override should create an instance
        /// of the derived PSDriveInfo using the passed in PSDriveInfo. The derived
        /// PSDriveInfo should then be returned. Each subsequent call into the provider
        /// that uses this drive will have access to the derived PSDriveInfo via the
        /// PSDriveInfo property provided by the base class. Implementers of this 
        /// method should verify that the root exists and that a connection to 
        /// the data store (if there is one) can be made.  Any failures should 
        /// be sent to the
        /// <see cref="System.Management.Automation.Provider.CmdletProvider.WriteError(ErrorRecord)"/>
        /// method and null should be returned.
        /// </remarks>
        protected override PSDriveInfo NewDrive( PSDriveInfo drive )
        {
            return drive;
        }// NewDrive

        /// <summary>
        /// Allows the provider to attach additional parameters to the 
        /// New-PSDrive cmdlet.
        /// </summary>
        /// 
        /// <returns>
        /// Implementors of this method should return an object that has 
        /// properties and fields decorated with parsing attributes similar 
        /// to a cmdlet class or a 
        /// <see cref="System.Management.Automation.PseudoParameterDictionary"/>.
        /// The default implemenation returns null.
        /// </returns>
        protected override object NewDriveDynamicParameters()
        {
            return null;
        }// NewDriveDynamicParameters

        /// <summary>
        /// Cleans up provider specific data for a drive before it is  
        /// removed. This method gets called before a drive gets removed. 
        /// </summary>
        /// 
        /// <param name="drive">
        /// The Drive object that represents the mounted drive.
        /// </param>
        /// 
        /// <returns>
        /// If the drive can be removed then the drive that was passed in is 
        /// returned. If the drive cannot be removed, null should be returned
        /// and an exception is written to the 
        /// <see cref="System.Management.Automation.Provider.CmdletProvider.WriteError(ErrorRecord)"/>
        /// method. The default implementation returns the drive that was 
        /// passed.
        /// </returns>
        /// 
        /// <remarks>
        /// An implementer has to ensure that this method is overridden to free 
        /// any resources that may be associated with the drive being removed.
        /// </remarks>
        /// 
        protected override PSDriveInfo RemoveDrive( PSDriveInfo drive )
        {
            return drive;
        }// RemoveDrive

        /// <summary>
        /// Allows the provider to map drives after initialization.
        /// </summary>
        /// 
        /// <returns>
        /// A drive collection with the drives that the provider wants to be 
        /// added to the session upon initialization. The default 
        /// implementation returns an empty 
        /// <see cref="System.Management.Automation.PSDriveInfo"/> collection.
        /// </returns>
        /// 
        /// <remarks>
        /// After the Start method is called on a provider, the
        /// InitializeDefaultDrives method is called. This is an opportunity
        /// for the provider to mount drives that are important to it. For
        /// instance, the Active Directory provider might mount a drive for
        /// the defaultNamingContext if the machine is joined to a domain.
        /// 
        /// All providers should mount a root drive to help the user with
        /// discoverability. This root drive might contain a listing of a set
        /// of locations that would be interesting as roots for other mounted
        /// drives. For instance, the Active Directory provider may create a
        /// drive that lists the naming contexts found in the namingContext
        /// attributes on the RootDSE. This will help users discover
        /// interesting mount points for other drives.
        /// </remarks>
        protected override Collection<PSDriveInfo> InitializeDefaultDrives()
        {
            Collection<PSDriveInfo> drives = new Collection<PSDriveInfo>();
            return drives;
        }// InitializeDefaultDrives

        #endregion DriveCmdletProvider Overrides

        // ItemCmdletProvider is a base class for a provider of a single item, 
        // that hooks into the Windows PowerShell namespace and which 
        // exposes the item as an Windows PowerShell path. When inherited from this 
        // class, the provider inherits a set of methods that allows the Windows 
        // PowerShell engine to provide a core set of commands for getting 
        // and setting of data on one or more items. A provider should derive 
        // from this class if they want to take advantage of the item core 
        // commands that are already implemented by the engine. This allows 
        // users to have common commands and semantics across multiple 
        // providers.

        #region ItemCmdletProvider Overrides

        /// <summary>
        /// Determines if the specified path is syntactically and semantically
        /// valid.
        /// </summary>
        /// 
        /// <param name="path">
        /// The path to validate.
        /// </param>
        /// 
        /// <returns>
        /// True if the path is syntactically and semantically valid for the 
        /// provider, false otherwise. The default implementation checks if 
        /// the path is not null or empty.
        /// </returns>
        /// 
        /// <remarks>
        /// This test should not verify the existance of the item at the path. 
        /// It should only perform syntactic and semantic validation of the 
        /// path. For instance, for the file system provider, that path should
        /// be canonicalized, syntactically verified, and ensure that the path
        /// does not refer to a device.
        /// </remarks>
        protected override bool IsValidPath( string path )
        {
            bool result = true;

            if ( String.IsNullOrEmpty( path ) )
            {
                result = false;
            }

            // Here, for example, you could check to make sure that there
            // are no empty segments in the path you are processing:
            // 
            // string pathSeparator = "\\/";
            // path = path.TrimEnd( pathSeparator.ToCharArray() );
            // path = path.TrimStart( pathSeparator.ToCharArray() );
            // string[] pathChunks = path.Split( pathSeparator.ToCharArray() );
            // foreach ( string pathChunk in pathChunks )
            // { 
            //     if ( pathChunk.Length == 0 )
            //     {
            //         result = false;
            //         break;
            //     }
            // }

            return result;

        } // IsValidPath

        /// <summary>
        /// Determines if an item exists at the specified path.
        /// </summary>
        /// <param name="path">
        /// The path to the item to see if it exists. 
        /// </param>
        /// <returns>
        /// True if the item exists, false otherwise.
        /// </returns>
        /// 
        /// <remarks>
        /// Providers can override this method to give the user the ability to 
        /// check for the existence of provider objects using the test-path 
        /// cmdlet. The default implementation returns false.
        /// 
        /// Providers that declare 
        /// <see cref="System.Management.Automation.Provider.ProviderCapabilities"/>
        /// of ExpandWildcards, Filter, Include, or Exclude should ensure that 
        /// the path passed meets those requirements by accessing the 
        /// appropriate property from the base class.
        /// 
        /// The implemenation of this method should take into account any form 
        /// of access to the object that may make it visible to the user.  For 
        /// instance, if a user has write access to a file in the file system 
        /// provider but not read access, the file still exists and the method
        /// should return true.  Sometimes this may require checking the parent
        /// to see if the child can be enumerated.
        /// </remarks>
        protected override bool ItemExists(string path)
        {
            return false;
        } // ItemExists

        /// <summary>
        /// Allows the provider to attach additional parameters to the 
        /// test-path cmdlet.
        /// </summary>
        /// 
        /// <param name="path">
        /// If the path was specified on the command line, this is the path to
        /// the item to get the dynamic parameters for.
        /// </param>
        /// 
        /// <returns>
        /// Overrides of this method should return an object that has 
        /// properties and fields decorated with parsing attributes similar to
        /// a cmdlet class or a 
        /// <see cref="System.Management.Automation.PseudoParameterDictionary"/>.
        /// 
        /// The default implemenation returns null. 
        /// </returns>
        protected override object ItemExistsDynamicParameters(string path)
        {
            return null;
        } // ItemExistsDynamicParameters

        /// <summary>
        /// Gets the item at the specified path. 
        /// </summary>
        /// 
        /// <param name="path">
        /// The path to the item to retrieve.
        /// </param>
        /// 
        /// <returns>
        /// Nothing is returned, but all objects will be written to the 
        /// WriteItemObject method.
        /// </returns>
        /// 
        /// <remarks>
        /// Providers can override this method to give the user access to the 
        /// provider objects using the get-item and get-childitem cmdlets. All
        /// objects should be written to the 
        /// <see cref="System.Management.Automation.Provider.CmdletProvider.WriteItemObject"/>
        /// method.
        /// 
        /// Providers that declare 
        /// <see cref="System.Management.Automation.Provider.ProviderCapabilities"/>
        /// of ExpandWildcards, Filter, Include, or Exclude should ensure that 
        /// the path passed meets those requirements by accessing the 
        /// appropriate property from the base class.
        /// 
        /// By default overrides of this method should not write objects that 
        /// are generally hidden from the user unless the Force property is set
        /// to true. For instance, the FileSystem provider should not call 
        /// WriteItemObject for hidden or system files unless the Force 
        /// property is set to true. 
        /// 
        /// The default implementation of this method does nothing
        /// </remarks>    
        protected override void GetItem( string path )
        {
            // Get the item at the specified path and write it using
            // WriteItemObject( item, path, isContainer );

        } // GetItem

        /// <summary>
        /// Allows the provider an opportunity to attach additional parameters 
        /// to the get-item cmdlet.
        /// </summary>
        /// 
        /// <param name="path">
        /// If the path was specified on the command line, this is the path
        /// to the item to get the dynamic parameters for.
        /// </param>
        /// 
        /// <returns>
        /// Overrides of this method should return an object that has 
        /// properties and fields decorated with parsing attributes similar 
        /// to a cmdlet class or a 
        /// <see cref="System.Management.Automation.PseudoParameterDictionary"/>.
        /// The default implemenation returns null. (no additional parameters)
        /// </returns>
        protected override object GetItemDynamicParameters(string path)
        {
            return null;
        } // GetItemDynamicParameters

        /// <summary>
        /// Sets the item specified by the path. 
        /// </summary>
        /// 
        /// <param name="path">
        /// The path to the item to set.
        /// </param>
        /// 
        /// <param name="value">
        /// The value of the item specified by the path.
        /// </param>
        /// 
        /// <remarks>
        /// The item that is set should be written to the 
        /// <see cref="System.Management.Automation.Providers.CmdletProvider.WriteItemObject"/> 
        /// method.
        /// 
        /// Providers can override this method to give the user the ability to 
        /// modify provider objects using the set-item cmdlet.
        /// 
        /// Providers that declare 
        /// <see cref="System.Management.Automation.Provider.ProviderCapabilities"/>
        /// of ExpandWildcards, Filter, Include, or Exclude should ensure that
        /// the path passed meets those requirements by accessing the 
        /// appropriate property from the base class.
        /// 
        /// By default, overrides of this method should not set or write 
        /// objects that are generally hidden from the user unless the Force 
        /// property is set to true. An error should be sent to the 
        /// <see cref="System.Management.Automation.Provider.CmdletProvider.WriteError(ErrorRecord)"/>
        /// method if the path represents an item that is hidden from the user 
        /// and Force is set to false.
        /// 
        /// The default implementation of this method throws an 
        /// <see cref="System.Management.Automation.PSNotSupportedException"/>.
        /// This method should call ShouldProcess and check its return value 
        /// before making any changes to the store this provider is working upon.
        /// </remarks>     
        protected override void SetItem( string path, object value )
        {
            // Modify the item at the specified path and write
            // it using
            // WriteItemObject( item, path, isContainer );

            // Example
            // if (ShouldProcess(path, "ClearItem"))
            // {
            //      // Set the item and then call WriteItemObject
            //      WriteItemObject(item, path, isContainer);
            // }

        } //SetItem

        /// <summary>
        /// Allows the provider to attach additional parameters to the set-item
        /// cmdlet.
        /// </summary>
        /// 
        /// <param name="path">
        /// If the path was specified on the command line, this is the path to
        /// the item to get the dynamic parameters for.
        /// </param>
        /// 
        /// <param name="value">
        /// The value of the item specified by the path.
        /// </param>
        /// 
        /// <returns>
        /// Overrides of this method should return an object that has 
        /// properties and fields decorated with parsing attributes similar to
        /// a cmdlet class or a 
        /// <see cref="System.Management.Automation.PseudoParameterDictionary"/>.
        /// 
        /// The default implemenation returns null. 
        /// </returns>
        protected override object SetItemDynamicParameters(string path, object value)
        {
            return null;
        } // SetItemDynamicParameters

        /// <summary>
        /// Clears the item specified by the path. 
        /// </summary>
        /// 
        /// <param name="path">
        /// The path to the item to clear.
        /// </param>
        /// 
        /// <remarks>
        /// Providers override this method to give the user the ability to 
        /// clear provider objects using the clear-item cmdlet.
        /// 
        /// The item that is cleared should be written to the 
        /// <see cref="System.Management.Automation.Providers.CmdletProvider.WriteItemObject"/> 
        /// method.
        /// 
        /// This method should call 
        /// <see cref="System.Management.Automation.Providers.CmdletProvider.ShouldProcess"/> 
        /// and check its return value before making any changes to the store 
        /// this provider is working upon.
        /// 
        /// Providers that declare 
        /// <see cref="System.Management.Automation.Provider.ProviderCapabilities"/>
        /// of ExpandWildcards, Filter, Include, or Exclude should ensure that 
        /// the path passed meets those requirements by accessing the 
        /// appropriate property from the base class.
        /// 
        /// By default overrides of this method should not clear or write 
        /// objects that are generally hidden from the user unless the Force 
        /// property is set to true. An error should be sent to the 
        /// <see cref="System.Management.Automation.Provider.CmdletProvider.WriteError(ErrorRecord)"/> 
        /// method if the path represents an item that is hidden from the user 
        /// and Force is set to false.
        /// </remarks>        
        protected override void ClearItem(string path)
        {         
            // clear the item at the specified path and write
            // it using
            // WriteItemObject(item, path, isContainer);

            // Example             
            //
            // if (ShouldProcess(path, "ClearItem"))
            // {
            //   //Clear the item and call WriteItemObject
            //   WriteItemObject(item, path, IsContainer);
            // }

        }// ClearItem

        /// <summary>
        /// Allows the provider to attach additional parameters to the 
        /// clear-item cmdlet.
        /// </summary>
        /// 
        /// <param name="path">
        /// If the path was specified on the command line, this is the path to
        /// the item to get the dynamic parameters for.
        /// </param>
        /// 
        /// <returns>
        /// Overrides of this method should return an object that has 
        /// properties and fields decorated with parsing attributes similar to 
        /// a cmdlet class or a 
        /// <see cref="System.Management.Automation.PseudoParameterDictionary"/>.
        /// 
        /// The default implemenation returns null. 
        /// </returns>
        protected override object ClearItemDynamicParameters(string path)
        {
            return null;
        } // ClearItemDynamicParameters

        /// <summary>
        /// Invokes the default action on the specified item. 
        /// </summary>
        /// 
        /// <param name="path">
        /// The path to the item to perform the default action on.
        /// </param>
        /// 
        /// <remarks>
        /// Providers can override this method to give the user the ability to 
        /// invoke provider objects using the invoke-item cmdlet. Think of the 
        /// invocation as a double click in the Windows Shell. This method 
        /// provides a default action based on the path that was passed. For 
        /// example, the File System provider may call ShellExecute on the
        /// path that is passed in.The default implementation does nothing.
        /// 
        /// Providers that declare 
        /// <see cref="System.Management.Automation.Provider.ProviderCapabilities"/>
        /// of ExpandWildcards, Filter, Include, or Exclude should ensure that 
        /// the path passed meets those requirements by accessing the 
        /// appropriate property from the base class.
        /// 
        /// By default, overrides of this method should not invoke objects that 
        /// are generally hidden from the user unless the Force property is set 
        /// to true. An error should be sent to the 
        /// <see cref="System.Management.Automation.Provider.CmdletProvider.WriteError(ErrorRecord)"/>
        /// method if the path represents an item that is hidden from the user 
        /// and Force is set to false.
        /// </remarks>
        protected override void InvokeDefaultAction( string path )
        {

        } // InvokeDefaultAction

        /// <summary>
        /// Gives the provider an opportunity to attach additional parameters to
        /// the invoke-item cmdlet.
        /// </summary>
        /// 
        /// <param name="path">
        /// If the path was specified on the command line, this is the path
        /// to the item to get the dynamic parameters for.
        /// </param>
        /// 
        /// <returns>
        /// Overrides of this method should return an object that has 
        /// properties and fields decorated with parsing attributes similar 
        /// to a cmdlet class or a 
        /// <see cref="System.Management.Automation.PseudoParameterDictionary"/>.
        /// 
        /// The default implemenation returns null. 
        /// </returns>
        protected override object InvokeDefaultActionDynamicParameters(string path)
        {
            return null;
        } // InvokeDefaultActionDynamicParameters

        #endregion ItemCmdletProvider Overrides

        // ContainerCmdletProvider is a base class for a provider of a container
        // of items, that hooks into the Windows PowerShell namespace. 
        // This provider base class has methods which can be overriden to allow 
        // the use of a set of core commands against the objects that the 
        // provider gives access to. By deriving from this class users can take 
        // advantage of all the features of the ItemCmdletProvider as well as
        // use of wildcards on objects and the following commands:
        //     get-childitem
        //     rename-item
        //     new-item
        //     remove-item
        //     set-location
        //     push-location
        //     pop-location
        //     get-location -stack

        #region ContainerCmdletProvider Overrides

        /// <summary>
        /// Gets the children of the item at the specified path.
        /// </summary>
        /// 
        /// <param name="path">
        /// The path (or name in a flat namespace) to the item from which to
        /// retrieve the children.
        /// </param>
        /// 
        /// <param name="recurse">
        /// True if all children in a subtree should be retrieved, false if
        /// only a single level of children should be retrieved.  
        /// </param>
        /// 
        /// <remarks>
        /// Providers override this method to give the user access to the 
        /// provider objects using the get-childitem cmdlets.
        /// 
        /// Providers that declare 
        /// <see cref="System.Management.Automation.Provider.ProviderCapabilities"/>
        /// of ExpandWildcards, Filter, Include, or Exclude should ensure that 
        /// the path passed meets those requirements by accessing the 
        /// appropriate property from the base class.
        /// 
        /// By default, overrides of this method should not write objects that 
        /// are generally hidden from the user unless the Force property is set
        /// to true. For instance, the FileSystem provider should
        /// not call WriteItemObject for hidden or system files unless the Force
        /// property is set to true. 
        /// 
        /// The provider implementation is responsible for preventing infinite 
        /// recursion when there are circular links and the like. An 
        /// appropriate terminating exception should be thrown if this
        /// situation occurs.
        /// </remarks>
        protected override void GetChildItems( string path, bool recurse )
        {
            // WriteItemObject( item, path, isContainer );
            //
            // If more than one item is the norm and the operation may take
            // some time try writing each individual item to the
            // WriteItemObject method one at a time. This allows the user to
            // get a streaming behavior.

        } // GetChildItems

        /// <summary>
        /// Allows the provider to attach additional parameters to the 
        /// get-childitem cmdlet.
        /// </summary>
        /// 
        /// <param name="path">
        /// If the path was specified on the command line, this is the path
        /// to the item to get the dynamic parameters for.
        /// </param>
        /// 
        /// <param name="recurse">
        /// True if all children in a subtree should be retrieved, false if 
        /// only a single level of children should be retrieved. In either 
        /// case, only leaf items that match the filter should be returned.
        /// </param>
        /// 
        /// <returns>
        /// Overrides of this method should return an object that has 
        /// properties and fields decorated with parsing attributes similar to
        /// a cmdlet class or a 
        /// <see cref="System.Management.Automation.PseudoParameterDictionary"/>.
        /// 
        /// The default implemenation returns null. 
        /// </returns>
        protected override object GetChildItemsDynamicParameters(string path,
                                        bool recurse)
        {
            return null;
        } // GetChildItemsDynamicParameters

        /// <summary>
        /// Gets names of the children of the specified path.
        /// </summary>
        /// 
        /// <param name="path">
        /// The path to the item from which to retrieve the child names.
        /// </param>
        /// 
        /// <param name="returnAllContainers">
        /// If true, the provider should return all containers, even if they
        /// don't match the filter.  If false, the provider should only return
        /// containers that do match the filter.
        /// </param>
        /// 
        /// <remarks>
        /// Providers override this method to give the user access to the 
        /// provider objects using the get-childitem -name cmdlet.
        /// 
        /// Providers that declare 
        /// <see cref="System.Management.Automation.Provider.ProviderCapabilities"/>
        /// of ExpandWildcards, Filter, Include, or Exclude should ensure that
        /// the path passed meets those requirements by accessing the 
        /// appropriate property from the base class. The exception to this is
        /// if <paramref name="returnAllContainers"/> is true, then any child 
        /// name for a container should be returned even if it doesn't match 
        /// the Filter, Include, or Exclude.
        /// 
        /// By default overrides of this method should not write the names of
        /// objects that are generally hidden from the user unless the Force 
        /// property is set to true. For instance, the FileSystem provider 
        /// should not call WriteItemObject for hidden or system files unless 
        /// the Force property is set to true. 
        /// 
        /// The provider implementation is responsible for preventing infinite 
        /// recursion when there are circular links and the like. An 
        /// appropriate terminating exception should be thrown if this
        /// situation occurs.
        /// 
        /// The child names are the leaf portion of the path. Example, for the
        /// file system the name for the path c:\windows\system32\foo.dll would
        /// be foo.dll or for the directory c:\windows\system32 would be
        /// system32.  For Active Directory the child names would be RDN values
        /// of the child objects of the container.
        /// 
        /// All names should be written to the WriteItemObject method.
        /// </remarks>
        protected override void GetChildNames( string path,
                                    ReturnContainers returnAllContainers )
        {
            // Get the names of children at the specified path
            // and writing them 
            // WriteItemObject( item, path, isContainer );

        } // GetChildNames

        /// <summary>
        /// Allows the provider to attach additional parameters to the 
        /// get-childitem -name cmdlet.
        /// </summary>
        /// If the path was specified on the command line, this is the path
        /// to the item to get the dynamic parameters for.
        /// </param>
        /// <returns>
        /// Overrides of this method should return an object that has 
        /// properties and fields decorated with parsing attributes similar to 
        /// a cmdlet class or a 
        /// <see cref="System.Management.Automation.PseudoParameterDictionary"/>.
        /// 
        /// The default implemenation returns null. 
        /// </returns>        
        protected override object GetChildNamesDynamicParameters(string path)
        {
            return null;
        } // GetChildNamesDynamicParameters

        /// <summary>
        /// Renames the item at the specified path to the new name provided.
        /// </summary>
        /// 
        /// <param name="path">
        /// The path to the item to rename.
        /// </param>
        /// 
        /// <param name="newName">
        /// The name to which the item should be renamed.  This name should
        /// always be relative to the parent container.
        /// </param>
        /// 
        /// <remarks>
        /// Providers override this method to give the user the ability to 
        /// rename provider objects using the rename-item cmdlet.
        /// 
        /// Providers that declare 
        /// <see cref="System.Management.Automation.Provider.ProviderCapabilities"/>
        /// of ExpandWildcards, Filter, Include, or Exclude should ensure that 
        /// the path passed meets those requirements by accessing the 
        /// appropriate property from the base class.
        /// 
        /// By default overrides of this method should not allow renaming 
        /// objects that are generally hidden from the user unless the Force 
        /// property is set to true. For instance, the FileSystem provider 
        /// should not allow renaming of a hidden or system file unless the 
        /// Force property is set to true. 
        /// 
        /// This method is intended for the modification of the item's name 
        /// only and not for Move operations. An error should be written to 
        /// <see cref="CmdletProvider.WriteError"/> if the 
        /// <paramref name="newName"/> parameter contains path separators or 
        /// would cause the item to change its parent location.
        /// All renamed items should be written to the WriteItemObject.
        /// 
        /// This method should call ShouldProcess and check its return value
        /// before making any changes to the store this provider is working 
        /// upon.
        /// </remarks>
        protected override void RenameItem( string path, string newName )
        {
            // Rename the item at the specified after
            // necessary validations here.
            //
            // WriteItemObject(newName, path, false);

            // Example 
            //
            // if (ShouldProcess(path, "rename"))
            // {
            //      //Rename the item and then call WriteItemObject
            //      WriteItemObject(newName, path, false);
            // }

        } // RenameItem

        /// <summary>
        /// Allows the provider to attach additional parameters to the 
        /// rename-item cmdlet.
        /// </summary>
        /// 
        /// <param name="path">
        /// If the path was specified on the command line, this is the path
        /// to the item to get the dynamic parameters for.
        /// </param>
        /// 
        /// <param name="newName">
        /// The name to which the item should be renamed. This name should 
        /// always be relative to the parent container.
        /// </param>
        /// 
        /// <returns>
        /// Overrides of this method should return an object that has 
        /// properties and fields decorated with parsing attributes similar to 
        /// a cmdlet class or a 
        /// <see cref="System.Management.Automation.PseudoParameterDictionary"/>.
        /// 
        /// The default implemenation returns null. 
        /// </returns>
        protected override object RenameItemDynamicParameters(string path,
                                        string newName)
        {
            return null;
        } // RenameItemDynamicParameters

        /// <summary>
        /// Creates a new item at the specified path.
        /// </summary>
        /// 
        /// <param name="path">
        /// The path to the item to create.
        /// </param>
        /// 
        /// <param name="type">
        /// The provider defined type for the object to create.
        /// </param>
        /// 
        /// <param name="newItemValue">
        /// This is a provider specific type that the provider can use to
        /// create a new instance of an item at the specified path.
        /// </param>
        /// 
        /// <remarks>
        /// Providers override this method to give the user the ability to 
        /// create new provider objects using the new-item cmdlet.
        /// 
        /// The <paramref name="type"/> parameter is a provider specific 
        /// string that the user specifies to tell the provider what type of 
        /// object to create.  For instance, in the FileSystem provider the 
        /// <paramref name="type"/> parameter can take a value of "file" or 
        /// "directory". The comparison of this string should be 
        /// case-insensitive and you should also allow for least ambiguous 
        /// matches. So if the provider allows for the types "file" and 
        /// "directory", only the first letter is required to disambiguate.
        /// If <paramref name="type"/> refers to a type the provider cannot 
        /// create, the provider should produce an 
        /// <see cref="ArgumentException"/> with a message indicating the 
        /// types the provider can create.
        /// 
        /// The <paramref name="newItemValue"/> parameter can be any type of 
        /// object that the provider can use to create the item. It is 
        /// recommended that the provider accept at a minimum strings, and an 
        /// instance of the type of object that would be returned from 
        /// GetItem() for this path. 
        /// <see cref="LanguagePrimitives.ConvertTo(System.Object, System.Type)"/>
        /// can be used to convert some types to the desired type.
        /// 
        /// This method should call 
        /// <see cref="System.Management.Automation.CmdletProvider.ShouldProcess"/>
        /// and check its return value before making any changes to the store 
        /// this provider is working upon.
        /// </remarks>
        protected override void NewItem( string path, string type, 
                                    object newItemValue )
        {
            // Create the new item here after
            // performing necessary validations
            //
            // WriteItemObject(newItemValue, path, false);

            // Example 
            //
            // if (ShouldProcess(path, "new item"))
            // {
            //      // Create a new item and then call WriteObject
            //      WriteObject(newItemValue, path, false);
            // }

        } // NewItem

        /// <summary>
        /// Allows the provider to attach additional parameters to the
        /// new-item cmdlet.
        /// </summary>
        /// 
        /// <param name="path">
        /// If the path was specified on the command line, this is the path
        /// to the item to get the dynamic parameters for.
        /// </param>
        /// 
        /// <param name="type">
        /// The provider defined type of the item to create.
        /// </param>
        /// 
        /// <param name="newItemValue">
        /// This is a provider specific type that the provider can use to 
        /// create a new instance of an item at the specified path.
        /// </param>
        /// 
        /// <returns>
        /// Overrides of this method should return an object that has 
        /// properties and fields decorated with parsing attributes similar to 
        /// a cmdlet class or a 
        /// <see cref="System.Management.Automation.PseudoParameterDictionary"/>.
        /// 
        /// The default implemenation returns null. 
        /// </returns>
        protected override object NewItemDynamicParameters(string path,
                                        string type, object newItemValue)
        {
            return null;
        } // NewItemDynamicParameters

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
        /// the item at the specified path should be removed. 
        /// </param>
        /// 
        /// <remarks>
        /// Providers override this method to allow the user the ability to 
        /// remove provider objects using the remove-item cmdlet.
        /// 
        /// Providers that declare 
        /// <see cref="System.Management.Automation.Provider.ProviderCapabilities"/>
        /// of ExpandWildcards, Filter, Include, or Exclude should ensure that 
        /// the path passed meets those requirements by accessing the 
        /// appropriate property from the base class.
        /// 
        /// By default, overrides of this method should not remove objects that
        /// are generally hidden from the user unless the Force property is set 
        /// to true. For instance, the FileSystem provider should not remove a 
        /// hidden or system file unless the Force property is set to true. 
        /// 
        /// The provider implementation is responsible for preventing infinite 
        /// recursion when there are circular links and the like. An 
        /// appropriate terminating exception should be thrown if this 
        /// situation occurs.
        /// 
        /// This method should call 
        /// <see cref="System.Management.Automation.CmdletProvider.ShouldProcess"/> 
        /// and check its return value before making any changes to the store 
        /// this provider is working upon.
        /// </remarks>
        protected override void RemoveItem( string path, bool recurse )
        {
            // Example 
            //
            // if (ShouldProcess(path, "delete"))
            // {
            //      // delete the item
            // }

        } // RemoveItem

        /// <summary>
        /// Allows the provider to attach additional parameters to 
        /// the remove-item cmdlet.
        /// </summary>
        /// 
        /// <param name="path">
        /// If the path was specified on the command line, this is the path
        /// to the item to get the dynamic parameters for.
        /// </param>
        /// 
        /// <param name="recurse">
        /// True if all children in a subtree should be removed, false if only 
        /// a single level of children should be removed. 
        /// </param>
        /// 
        /// <returns>
        /// Overrides of this method should return an object that has 
        /// properties and fields decorated with parsing attributes similar to 
        /// a cmdlet class or a 
        /// <see cref="System.Management.Automation.PseudoParameterDictionary"/>.
        /// 
        /// The default implemenation returns null. 
        /// </returns>
        protected override object RemoveItemDynamicParameters(string path,
                                        bool recurse)
        {
            return null;
        } // RemoveItemDynamicParameters

        /// <summary>
        /// Determines if the item at the specified path has children.
        /// </summary>
        /// 
        /// <param name="path">
        /// The path to the item to see if it has children.
        /// </param>
        /// 
        /// <returns>
        /// True if the item has children, false otherwise.
        /// </returns>
        /// 
        /// <remarks>
        /// Providers override this method to give the provider infrastructure 
        /// the ability to determine if a particular provider object has 
        /// children without having to retrieve all the child items.
        /// 
        /// For providers that are derived from 
        /// <see cref="ContainerCmdletProvider"/> class, if a null or empty 
        /// path is passed, the provider should consider any items in the data 
        /// store to be children and return true.
        /// 
        /// If this provider exposes a root that contains interesting mount
        /// points (as described in InitializeDefaultDrives) it should return
        /// true when null or String.Empty is passed. 
        /// </remarks>
        protected override bool HasChildItems( string path )
        {
            return false;
        } // HasChildItems

        /// <summary>
        /// Copies an item at the specified path to an item at the
        /// <paramref name="copyPath" />.
        /// </summary>
        /// 
        /// <param name="path">
        /// The path of the item to copy.
        /// </param>
        /// 
        /// <param name="copyPath">
        /// The path of the item to copy to.
        /// </param>
        /// 
        /// <param name="recurse">
        /// Tells the provider to recurse sub-containers when copying.
        /// </param>
        /// 
        /// <remarks>
        /// Providers override this method to give the user the ability to copy
        /// provider objects using the copy-item cmdlet.
        /// 
        /// Providers that declare 
        /// <see cref="System.Management.Automation.Provider.ProviderCapabilities"/>
        /// of ExpandWildcards, Filter, Include, or Exclude should ensure that 
        /// the path and items being copied meets those requirements by 
        /// accessing the appropriate property from the base class.
        /// 
        /// By default, overrides of this method should not copy objects over 
        /// existing items unless the Force property is set to true. For 
        /// instance, the FileSystem provider should not copy c:\temp\foo.txt 
        /// over c:\bar.txt if c:\bar.txt already exists unless the Force 
        /// parameter is true.
        /// 
        /// If <paramref name="copyPath"/> exists and is a container then Force
        /// isn't required and <paramref name="path"/> should be copied into 
        /// the <paramref name="copyPath"/> container as a child.
        /// 
        /// If <paramref name="recurse"/> is true, the provider implementation 
        /// is responsible for preventing infinite recursion when there are 
        /// circular links and the like. An appropriate terminating exception 
        /// should be thrown if this situation occurs.
        /// 
        /// This method should call 
        /// <see cref="System.Management.Automation.CmdletProvider.ShouldProcess"/>  
        /// and check its return value before making any changes to the store 
        /// this provider is working upon.
        /// </remarks>
        protected override void CopyItem( string path, string copyPath,
                                    bool recurse )
        {
            // Code for Copying item after performing necessary 
            // validations here
            //
            // WriteItemObject(item, path, true);

            // Example 
            //
            // if (ShouldProcess(path, "copy"))
            // {
            //      // Delete the item and call WriteItemObject
            //      WriteItemObject(item, path, true);
            // }
        } // CopyItem

        /// <summary>
        /// Allows the provider to attach additional parameters to the 
        /// copy-item cmdlet.
        /// </summary>
        /// 
        /// <param name="path">
        /// If the path was specified on the command line, this is the path
        /// to the item to get the dynamic parameters for.
        /// </param>
        /// 
        /// <param name="destination">
        /// The path of the item to copy to.
        /// </param>
        /// 
        /// <param name="recurse">
        /// Tells the provider to recurse sub-containers when copying.
        /// </param>
        /// 
        /// <returns>
        /// Overrides of this method should return an object that has 
        /// properties and fields decorated with parsing attributes 
        /// similar to a cmdlet class or a 
        /// <see cref="System.Management.Automation.PseudoParameterDictionary"/>.
        /// </returns>
        /// 
        /// <remarks>
        /// The default implemenation returns null. 
        /// </remarks>
        protected override object CopyItemDynamicParameters( string path, 
                                        string destination, bool recurse )
        {
            return null;
        } // CopyItemDynamicParameters

        #endregion ContainerCmdletProvider Overrides

        // NavigationCmdletProvider is a base class for a provider of a tree of 
        // items, that hooks into the Windows PowerShell namespace. By 
        // deriving from this class users can take advantage the recursive 
        // commands, nested containers, and relative paths.

        #region NavigationCmdletProvider Overrides

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
        /// 
        /// <remarks>
        /// This method should use lexical joining of two path segments with a
        /// path separator character.  It should not validate the path as a
        /// legal fully qualified path in the provider namespace as each
        /// parameter could be only partial segments of a path and joined
        /// they may not generate a fully qualified path.
        /// Example: the file system provider may get "windows\system32" as
        /// the parent parameter and "foo.dll" as the child parameter. The
        /// method should join these with the "\" separator and return
        /// "windows\system32\foo.dll". Note that the returned path is not a
        /// fully qualified file system path.
        /// 
        /// Also beware that the path segments may contain characters that are
        /// illegal in the provider namespace. These characters are most
        /// likely used as wildcards and should not be removed by the
        /// implementation of this method.
        /// 
        /// The default implementation will take paths with '/' or '\' as the
        /// path separator and normalize the path separator to '\' and then
        /// join the child and parent with a '\'.
        /// </remarks>
        protected override string MakePath( string parent, string child )
        {
            return base.MakePath( parent, child ); // return combined path
        } // MakePath

        /// <summary>
        /// Removes the child segment of a path and returns the remaining
        /// parent portion.
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
        /// <returns>
        /// The path of the parent of the path parameter.
        /// </returns>
        /// 
        /// <remarks>
        /// This should be a lexical splitting of the path on the path
        /// separator character for the provider namespace.  For example, the
        /// file system provider should look for the last "\" and return
        /// everything to the left of the "\".
        /// 
        /// The default implementation accepts paths that have both '/' and
        /// '\' as the path separator. It first normalizes the path to have
        /// only '\' separators and then splits the parent path off at the
        /// last '\' and returns it.
        /// </remarks>
        protected override string GetParentPath( string path, string root )
        {
            return base.GetParentPath( path, root ); // return parentpath
        } // GetParentPath

        /// <summary>
        /// Gets the name of the leaf element in the specified path.
        /// </summary>
        /// 
        /// <param name="path">
        /// The full or partial provider specific path.
        /// </param>
        /// 
        /// <returns>
        /// The leaf element in the path.
        /// </returns>
        /// 
        /// <remarks>
        /// This should be implemented as a split on the path separator.  The
        /// characters in the fullPath may not be legal characters in the
        /// namespace but may be used for wildcards or regular expression
        /// matching.  If the path contains no path separators the path should
        /// be returned unmodified.
        /// 
        /// The default implementation accepts paths that have both '/' and
        /// '\' as the path separator. It first normalizes the path to have
        /// only '\' separators and then splits the parent path off at the
        /// last '\' and returns it.
        /// </remarks>
        protected override string GetChildName( string path )
        {
            return base.GetChildName( path ); // return childname
        } // GetChildName

        /// <summary>
        /// Determines if the specified object is a container
        /// </summary>
        /// 
        /// <param name="path">
        /// The path to the item to determine if it is a container.
        /// </param>
        /// 
        /// <returns>
        /// true if the item specified by path exists and is a container, 
        /// false otherwise.
        /// </returns>
        /// 
        /// <remarks>
        /// Providers override this method to give the user the ability to 
        /// check to see if a provider object is a container using the 
        /// test-path -container cmdlet.
        /// 
        /// Providers that declare 
        /// <see cref="System.Management.Automation.Provider.ProviderCapabilities"/>
        /// of ExpandWildcards, Filter, Include, or Exclude should ensure that 
        /// the path passed meets those requirements by accessing the 
        /// appropriate property from the base class.
        /// </remarks>
        protected override bool IsItemContainer( string path )
        {
            return false; 
        } // IsItemContainer

        /// <summary>
        /// Moves the item specified by path to the specified destination.
        /// </summary>
        /// 
        /// <param name="path">
        /// The path to the item to be moved.
        /// </param>
        /// 
        /// <param name="destination">
        /// The path of the destination container.
        /// </param>
        /// 
        /// <remarks>
        /// Providers override this method to give the user the ability to move
        /// provider objects using the move-item cmdlet.
        /// 
        /// Providers that declare 
        /// <see cref="System.Management.Automation.Provider.ProviderCapabilities"/>
        /// of ExpandWildcards, Filter, Include, or Exclude should ensure that 
        /// the path and items being moved meets those requirements by accessing
        /// the appropriate property from the base class.
        /// 
        /// By default overrides of this method should not move objects over 
        /// existing items unless the Force property is set to true. For 
        /// instance, the FileSystem provider should not move c:\temp\foo.txt 
        /// over c:\bar.txt if c:\bar.txt already exists unless the Force 
        /// parameter is true.
        /// 
        /// If <paramref name="destination"/> exists and is a container then 
        /// Force isn't required and <paramref name="path"/> should be moved 
        /// into the <paramref name="destination"/> container as a child.
        /// 
        /// All objects that are moved should be written to the 
        /// <see cref="System.Management.Automation.Provider.CmdletProvider.WriteItemObject"/>
        /// method.
        /// 
        /// This method should call 
        /// <see cref="System.Management.Automation.CmdletProvider.ShouldProcess"/> 
        /// and check its return value before making any changes to the store 
        /// this provider is working upon.
        /// </remarks>
        protected override void MoveItem( string path, string destination )
        {
            // Write code for moving the item after performing
            // necessary validations here
            //

            // Example 
            //
            // if (ShouldProcess(path, "move"))
            // {
            //      //Move item here
            // }
        } // MoveItem

        /// <summary>
        /// Allows the provider to attach additional parameters to
        /// the move-item cmdlet.
        /// </summary>
        /// 
        /// <param name="path">
        /// If the path was specified on the command line, this is the path
        /// to the item to get the dynamic parameters for.
        /// </param>
        /// 
        /// <param name="destination">
        /// The path of the destination container.
        /// </param>
        /// 
        /// <returns>
        /// Overrides of this method should return an object that has 
        /// properties and fields decorated with parsing attributes similar 
        /// to a cmdlet class or a 
        /// <see cref="System.Management.Automation.PseudoParameterDictionary"/>.
        /// 
        /// The default implemenation returns null. 
        /// </returns>
        protected override object MoveItemDynamicParameters(string path,
                                        string destination)
        {
            return null;
        } // MoveItemDynamicParameters

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
        /// <param name="basePath">
        /// The path that the return value should be relative to.
        /// </param>
        /// 
        /// <returns>
        /// A normalized path that is relative to the basePath that was
        /// passed.  The provider should parse the path parameter, normalize
        /// the path, and then return the normalized path relative to the
        /// basePath.
        /// </returns>
        ///
        /// <remarks>
        /// This method does not have to be purely syntactical parsing of the
        /// path.  It is encouraged that the provider actually use the path to
        /// lookup in its store and create a relative path that matches the
        /// casing, and standardized path syntax.
        /// 
        /// Note, the base class implemenation uses GetParentPath,
        /// GetChildName, and MakePath to normalize the path and then make it
        /// relative to basePath.  All string comparisons are done using
        /// StringComparison.InvariantCultureIgnoreCase.
        /// </remarks>
        protected override string NormalizeRelativePath( string path,
                                        string basePath )
        {
            return base.NormalizeRelativePath( path, basePath );
        }
   
        #endregion NavigationCmdletProvider Overrides

        // IContentProvider is an interface for a provider of content that hooks 
        // into the Windows PowerShell namespace and helps in exposing
        // contents of an item.
        //
        // An IContentCmdletProvider provider implements a set of methods that 
        // allows the use of a set of core commands against the data store that
        // the provider gives access to. By implementing this interface users 
        // can take advantage the commands that expose the contents of an item.
        //
        //     get-content
        //     set-content
        //     clear-content

        #region IContentCmdletProvider Overrides

        /// <summary>
        /// Gets an IContentReader from the provider for the item at the
        /// specified path.
        /// </summary>
        /// 
        /// <param name="path">
        /// The path to the object to be opened for reading content.
        /// </param>
        /// 
        /// <returns>
        /// An IContentReader for the specified content.
        /// </returns>
        /// 
        /// <remarks>
        /// Overrides of this method should return an 
        /// <see cref="System.Management.Automation.Provider.IContentReader"/>
        /// for the item specified by the path.
        /// 
        /// Providers that declare 
        /// <see cref="System.Management.Automation.Provider.ProviderCapabilities"/>
        /// of ExpandWildcards, Filter, Include, or Exclude should ensure that 
        /// the path passed meets those requirements by accessing the 
        /// appropriate property from the base class.
        /// 
        /// By default, overrides of this method should not return a content 
        /// reader for objects that are generally hidden from the user unless 
        /// the Force property is set to true. An error should be sent to the 
        /// <see cref="System.Management.Automation.CmdletProvider.WriteError"/>
        /// method if
        /// the path represents an item that is hidden from the user and Force 
        /// is set to false.
        /// </remarks>
        public IContentReader GetContentReader( string path )
        {
            return null;
        } // GetContentReader

        /// <summary>
        /// Allows the provider to attach additional parameters to the
        /// get-content cmdlet.
        /// </summary>
        /// 
        /// <param name="path">
        /// If the path was specified on the command line, this is the path
        /// to the item to get the dynamic parameters for.
        /// </param>
        /// 
        /// <returns>
        /// An object that has properties and fields decorated with
        /// parsing attributes similar to a cmdlet class or a 
        /// <see cref="System.Management.Automation.PseudoParameterDictionary"/>.
        /// </returns>
        /// 
        /// <remarks>
        /// The default implementation returns null.
        /// </remarks>
        public object GetContentReaderDynamicParameters( string path )
        {
            return null;
        } // GetContentReaderDynamicParameters

        /// <summary>
        /// Gets an IContentWriter from the provider for the content at the
        /// specified path.
        /// </summary>
        /// 
        /// <param name="path">
        /// The path to the object to be opened for writing content.
        /// </param>
        /// 
        /// <returns>
        /// An IContentWriter for the item at the specified path.
        /// </returns>
        /// 
        /// <remarks>
        /// Overrides of this method should return an 
        /// <see cref="System.Management.Automation.Provider.IContentWriter"/>
        /// for the item specified by the path.
        /// 
        /// Providers that declare 
        /// <see cref="System.Management.Automation.Provider.ProviderCapabilities"/>
        /// of ExpandWildcards, Filter, Include, or Exclude should ensure that 
        /// the path passed meets those requirements by accessing the 
        /// appropriate property from the base class.
        /// 
        /// By default, overrides of this method should not return a content 
        /// writer for objects that are generally hidden from 
        /// the user unless the Force property is set to true. An error should 
        /// be sent to the WriteError method if the path represents an item 
        /// that is hidden from the user and Force is set to false.
        /// </remarks>
        public IContentWriter GetContentWriter( string path )
        {
            return null;
        } // GetContentWriter

        /// <summary>
        /// Allows the provider to attach additional parameters to the
        /// set-content and add-content cmdlet.
        /// </summary>
        /// 
        /// <param name="path">
        /// If the path was specified on the command line, this is the path
        /// to the item to get the dynamic parameters for.
        /// </param>
        /// 
        /// <returns>
        /// An object that has properties and fields decorated with parsing
        /// attributes similar to a cmdlet class.
        /// </returns>
        /// 
        /// <remarks>
        /// The default implementation will returns null.
        /// </remarks>
        public object GetContentWriterDynamicParameters( string path )
        {
            return null;
        } // GetContentWriterDynamicParameters

        /// <summary>
        /// Clears the content from the specified item.
        /// </summary>
        /// 
        /// <param name="path">
        /// The path to the item to clear the content from.
        /// </param>
        ///
        /// <remarks>
        /// Overrides of this method should remove any content from the object 
        /// but not remove (delete) the object itself.
        /// 
        /// Providers that declare 
        /// <see cref="System.Management.Automation.Provider.ProviderCapabilities"/>
        /// of ExpandWildcards, Filter, Include, or Exclude should ensure that 
        /// the path passed meets those requirements by accessing the 
        /// appropriate property from the base class.
        /// 
        /// By default, overrides of this method should not clear or write 
        /// objects that are generally hidden from the user unless the Force 
        /// property is set to true. An error should be sent to the WriteError 
        /// method if the path represents an item that is hidden from the user 
        /// and Force is set to false.
        /// 
        /// This method should call 
        /// <see cref="System.Management.Automation.Providers.CmdletProvider.ShouldProcess"/>
        /// and check its return value before making any changes to the store 
        /// this provider is working upon.
        /// </remarks>
        public void ClearContent( string path )
        {
            // Write code here to clear contents
            // after performing necessary validations

            // Example 
            // 
            // if (ShouldProcess(path, "clear"))
            // {
            //      // Clear the contents and then call WriteItemObject
            //      WriteItemObject("", path, false);
            // }
        }//ClearContent

        /// <summary>
        /// Allows the provider to attach additional parameters to the
        /// clear-content cmdlet.
        /// </summary>
        /// 
        /// <param name="path">
        /// If the path was specified on the command line, this is the path
        /// to the item to get the dynamic parameters for.
        /// </param>
        /// 
        /// <returns>
        /// An object that has properties and fields decorated with
        /// parsing attributes similar to a cmdlet class or a 
        /// <see cref="System.Management.Automation.PseudoParameterDictionary"/>.
        /// </returns>
        /// 
        /// <remarks>
        /// The default implementation returns null.
        /// </remarks>
        public object ClearContentDynamicParameters( string path )
        {
            return null;
        } // ClearContentDynamicParameters


        #endregion IContentCmdletProvider Overrides

        // IPropertyProvider is an interface for a provider that hooks 
        // into the Windows PowerShell namespace and exposes
        // properties of an item.
        //
        // An IPropertyCmdletProvider provider implements a set of methods 
        // that allows the use of a set of core commands against the data 
        // store that the provider gives access to. By implementing this 
        // interface users can take advantage the commands that expose the 
        // contents of an item.
        //
        //     get-itemproperty
        //     set-itemproperty
        //     clear-itemproperty

        #region IPropertyCmdletProvider Overrides      

        /// <summary>
        /// Gets the properties of the item specified by the path.
        /// </summary>
        /// 
        /// <param name="path">
        /// The path to the item to retrieve properties from.
        /// </param>
        /// 
        /// <param name="providerSpecificPickList">
        /// A list of properties that should be retrieved. If this parameter is
        /// null or empty, all properties should be retrieved.
        /// </param>
        /// 
        /// <remarks>
        /// 
        /// Providers override this method to give the user the ability to  
        /// retrieve properties to provider objects using the get-itemproperty 
        /// cmdlet.
        /// 
        /// Providers that declare 
        /// <see cref="System.Management.Automation.Provider.ProviderCapabilities"/>
        /// of ExpandWildcards, Filter, Include, or Exclude should ensure that 
        /// the path passed meets those requirements by accessing the 
        /// appropriate property from the base class.
        /// 
        /// By default, overrides of this method should not retrieve properties
        /// from objects that are generally hidden from the user unless the 
        /// Force property is set to true. An error should be sent to the 
        /// <see cref="System.Management.Automation.CmdletProvier.WriteError"/>
        /// method if the path represents an item that is hidden from the user 
        /// and Force is set to false.
        /// 
        /// An <see cref="System.Management.Automation.PSObject"/> can be used
        /// as a property bag for the properties that need to be returned if 
        /// the <paramref name="providerSpecificPickList"/> contains multiple 
        /// properties to get.
        ///
        /// An instance of <see cref="System.Management.Automation.PSObject"/>
        /// representing the properties that were retrieved should be passed 
        /// to the 
        /// <see cref="System.Management.Automation.CmdletProvider.WritePropertyObject"/> 
        /// method. 
        /// 
        /// It is recommended that you support wildcards in property names 
        /// using <see cref="System.Management.Automation.WildcardPattern"/>.
        /// </remarks>
        public void GetProperty( string path,
                        Collection<string> providerSpecificPickList )
        {
            // PSObject psObject = new PSObject();
            // psObject.AddNote( propertyName, propertyValue );
            // WritePropertyObject( propertyValue, path );

        } // GetProperty

        /// <summary>
        /// Allows the provider to attach additional parameters to the
        /// get-itemproperty cmdlet.
        /// </summary>
        /// 
        /// <param name="path">
        /// If the path was specified on the command line, this is the path
        /// to the item to get the dynamic parameters for.
        /// </param>
        /// 
        /// <param name="providerSpecificPickList">
        /// A list of properties that were specified on the command line.
        /// This parameter may be empty or null if the properties are being
        /// piped into the command.
        /// </param>
        /// 
        /// <returns>
        /// An object that has properties and fields decorated with parsing
        /// attributes similar to a cmdlet class or a 
        /// <see cref="System.Management.Automation.PseudoParameterDictionary"/>.
        /// Null can be returned if no dynamic parameters are to be added.
        /// </returns>
        /// 
        /// <remarks>
        /// The default implementation returns null.
        /// </remarks>
        public object GetPropertyDynamicParameters( string path,
                            Collection<string> providerSpecificPickList )
        {
            return null;
        } // GetPropertyDynamicParameters

        /// <summary>
        /// Sets the specified properties of the item at the specified path.
        /// </summary>
        /// 
        /// <param name="path">
        /// The path to the item to set the properties on.
        /// </param>
        /// 
        /// <param name="property">
        /// An PSObject which contains a collection of the names and values
        /// of the properties to be set.
        /// </param>
        /// 
        /// <remarks>
        /// Providers override this method to give the user the ability to set 
        /// the value of provider object properties using the set-itemproperty 
        /// cmdlet.
        /// 
        /// Providers that declare 
        /// <see cref="System.Management.Automation.Provider.ProviderCapabilities"/>
        /// of ExpandWildcards, Filter, Include, or Exclude should ensure that 
        /// the path passed meets those requirements by accessing the 
        /// appropriate property from the base class.
        /// 
        /// By default, overrides of this method should not retrieve properties
        /// from objects that are generally hidden from the user unless the 
        /// Force property is set to true. An error should be sent to the 
        /// WriteError 
        /// method if the path represents an item that is hidden from the user 
        /// and Force is set to false.
        /// 
        /// 
        /// An instance of <see cref="System.Management.Automation.PSObject"/>
        /// representing the properties that were set should be passed 
        /// to the 
        /// <see cref="System.Management.Automation.CmdletProvider.WritePropertyObject"/> 
        /// method. 
        /// 
        /// <paramref name="propertyValue"/> is a property bag containing the 
        /// properties that should be set. See 
        /// <see cref="System.Management.Automation.PSObject"/> for more 
        /// information.
        /// 
        /// This method should call 
        /// <see cref="System.Management.Automation.CmdletProvider.ShouldProcess"/>
        /// and check its return value before making any changes to the store 
        /// this provider is working upon.
        /// </remarks>
        public void SetProperty( string path, PSObject propertyValue )
        {
            // Write code here to set the specified property
            // after performing the necessary validations
            //
            // WritePropertyObject(propertyValue, path);

            // Example 
            // 
            //      if (ShouldProcess(path, "set property"))
            //      {
            //          // set the property here and then call WritePropertyObject
            //          WritePropertyObject(propertyValue, path);
            //      }
            // }
            
        } // SetProperty

        /// <summary>
        /// Allows the provider to attach additional parameters to the 
        /// set-itemproperty cmdlet.
        /// </summary>
        /// 
        /// <param name="path">
        /// If the path was specified on the command line, this is the path
        /// to the item to get the dynamic parameters for.
        /// </param>
        /// 
        /// <param name="propertyValue">
        /// An PSObject which contains a collection of the name, and value 
        /// of the properties to be set if they were specified on the command
        /// line.  The PSObject could be empty or null if the parameters are
        /// being piped into the command.
        /// </param>
        /// 
        /// <returns>
        /// An object that has properties and fields decorated with parsing
        /// attributes similar to a cmdlet class or a 
        /// <see cref="System.Management.Automation.PseudoParameterDictionary"/>.
        /// Null can be returned if no dynamic parameters are to be added.
        /// </returns>
        /// 
        /// <remarks>
        /// The default implementation returns null.
        /// </remarks>
        public object SetPropertyDynamicParameters( string path,
                            PSObject propertyValue )
        {
            return null;
        } // SetPropertyDynamicParameters

        /// <summary>
        /// Clears a property of the item at the specified path.
        /// </summary>
        /// 
        /// <param name="path">
        /// The path to the item on which to clear the property.
        /// </param>
        /// 
        /// <param name="propertyToClear">
        /// The name of the property to clear.
        /// </param>
        ///
        /// <remarks>
        /// Providers override this method to give the user the ability to clear
        /// the value of provider object properties using the clear-itemproperty 
        /// cmdlet.
        /// 
        /// Providers that declare 
        /// <see cref="System.Management.Automation.Provider.ProviderCapabilities"/>
        /// of ExpandWildcards, Filter, Include, or Exclude should ensure that 
        /// the path passed meets those requirements by accessing the 
        /// appropriate property from the base class.
        /// 
        /// By default, overrides of this method should not clear properties 
        /// from objects that are generally hidden from the user unless the 
        /// Force property is set to true. An error should be sent to the 
        /// <see cref="System.Management.Automation.CmdletProvider.WriteError"/>
        /// method if the path represents an item that is hidden from the user 
        /// and Force is set to false.
        /// 
        /// An <see cref="System.Management.Automation.PSObject"/> can be used
        /// as a property bag for the properties that need to be returned if 
        /// the <paramref name="propertyToClear"/> contains multiple 
        /// properties to write.
        /// 
        /// An instance of <see cref="System.Management.Automation.PSObject"/>
        /// representing the properties that were cleared should be passed 
        /// to the 
        /// <see cref="System.Management.Automation.CmdletProvider.WritePropertyObject"/> 
        /// method. 
        /// 
        /// This method should call 
        /// <see cref="System.Management.Automation.CmdletProvider.ShouldProcess"/> 
        /// and check its return value before making any changes to the store this provider is
        /// working upon.
        /// </remarks>
        public void ClearProperty( string path,
                        Collection<string> propertyToClear )
        {
            // Write code here to clear properties
            // of the item speicified at the path
            // after performing necessary validations

            // Example
            // 
            // if (propertyToClear.Count == 1)
            // {
            //      if (ShouldProcess(path, "clear properties"))
            //      {
            //          // Clear properties and then call WriteItemObject
            //          WritePropertyObject(propertyToClear[0], path);            
            //      }
            // }

        } // ClearProperty

        /// <summary>
        /// Allows the provider to attach additional parameters to the
        /// clear-itemproperty cmdlet.
        /// </summary>
        /// 
        /// <param name="path">
        /// If the path was specified on the command line, this is the path
        /// to the item to get the dynamic parameters for.
        /// </param>
        /// 
        /// <param name="propertyToClear">
        /// The name of the property to clear. This parameter may be null or
        /// empty if the properties to clear are being piped into the command.
        /// </param>
        /// 
        /// <returns>
        /// An object that has properties and fields decorated with parsing
        /// attributes similar to a cmdlet class or a 
        /// <see cref="System.Management.Automation.PseudoParameterDictionary"/>.
        /// Null can be returned if no dynamic parameters are to be added.
        /// </returns>
        /// 
        /// <remarks>
        /// The default implementation returns null.
        /// </remarks>
        public object ClearPropertyDynamicParameters( string path,
                        Collection<string> propertyToClear )
        {
            return null;
        } // ClearPropertyDynamicParameters

        #endregion IPropertyCmdletProvider

        // IDynamicPropertyCmdletProvider is an interface for a provider
        // that hooks into the Windows PowerShell namespace and
        // exposes dymanic manipulation of properties of an item.
        //
        // An IDynamicPropertyCmdletProvider provider implements a set of 
        // methods that allows the use of a set of core commands against 
        // the data store that the provider gives access to. By 
        // implementing this interface users can take advantage the 
        // commands that expose the creation and deletion of properties on an 
        // item.
        //
        //     rename-itemproperty
        //     remove-itemproperty
        //     new-itemproperty
        //     copy-itemproperty
        //     move-itemproperty

        #region IDynamicPropertyCmdletProvider Overrides

        /// <summary>
        /// Copies a property of the item at the specified path to a new
        /// property on the destination item.
        /// </summary>
        /// 
        /// <param name="sourcePath">
        /// The path to the item from which to copy the property.
        /// </param>
        /// 
        /// <param name="sourceProperty">
        /// The name of the property to copy.
        /// </param>
        /// 
        /// <param name="destinationPath">
        /// The path to the item on which to copy the property to.
        /// </param>
        /// 
        /// <param name="destinationProperty">
        /// The destination property to copy to.
        /// </param>
        ///
        /// <remarks>
        /// Providers override this method to give the user the ability to copy
        /// properties of provider objects using the copy-itemproperty cmdlet.
        /// 
        /// Providers that declare 
        /// <see cref="System.Management.Automation.Provider.ProviderCapabilities"/>
        /// of ExpandWildcards, Filter, Include, or Exclude should ensure that 
        /// the path passed meets those requirements by accessing the 
        /// appropriate property from the base class.
        /// 
        /// By default, overrides of this method should not copy properties 
        /// from or to objects that are generally hidden from the user unless 
        /// the Force property is set to true. An error should be sent to the 
        /// <see cref="System.Management.Automation.CmdletProvider.WriteError"/>
        /// method if the path represents an item that is hidden from the user 
        /// and Force is set to false. 
        /// 
        /// This method should call 
        /// <see cref="System.Management.Automation.CmdletProvider.ShouldProcess"/>
        /// and check its return value before making any changes to the store 
        /// this provider is working upon.
        /// </remarks>
        public void CopyProperty( string sourcePath, string sourceProperty,
                        string destinationPath, string destinationProperty )
        {
            // Write code here to modify property of 
            // an item after performing necessary
            // validations
            //
            // WritePropertyObject(destinationProperty, destinationPath );

            // Example 
            // 
            // if (ShouldProcess(destinationPath, "copy property"))
            // {
            //      // Copy property and then call WritePropertyObject
            //      WritePropertyObject(destinationProperty, destinationPath);
            // }

        } // CopyProperty

        /// <summary>
        /// Allows the provider a attach additional parameters to the
        /// copy-itemproperty cmdlet.
        /// </summary>
        /// 
        /// <param name="sourcePath">
        /// If the path was specified on the command line, this is the path
        /// to the item to get the dynamic parameters for.
        /// </param>
        /// 
        /// <param name="sourceProperty">
        /// The name of the property to copy.
        /// </param>
        /// 
        /// <param name="destinationPath">
        /// The path to the item on which to copy the property to.
        /// </param>
        /// 
        /// <param name="destinationProperty">
        /// The destination property to copy to.
        /// </param>
        /// 
        /// <returns>
        /// An object that has properties and fields decorated with parsing
        /// attributes similar to a cmdlet class or
        /// <see cref="System.Management.Automation.PseudoParameterDictionary"/>.
        /// Null can be returned if no
        /// dynamic parameters are to be added.
        /// </returns>
        /// 
        /// <remarks>
        /// The default implementation returns null.
        /// </remarks>
        public object CopyPropertyDynamicParameters( string sourcePath, 
                            string sourceProperty, string destinationPath, 
                                string destinationProperty )
        {
            return null;
        } // CopyPropertyDynamicParameters

        /// <summary>
        /// Moves a property on an item specified by the path.
        /// </summary>
        /// 
        /// <param name="sourcePath">
        /// The path to the item from which to move the property.
        /// </param>
        /// 
        /// <param name="sourceProperty">
        /// The name of the property to move.
        /// </param>
        /// 
        /// <param name="destinationPath">
        /// The path to the item on which to move the property to.
        /// </param>
        /// 
        /// <param name="destinationProperty">
        /// The destination property to move to.
        /// </param>
        ///
        /// <remarks>
        /// Providers override this method to give the user the ability to move
        /// properties from one provider object to another using the move-itemproperty cmdlet.
        /// 
        /// Providers that declare 
        /// <see cref="System.Management.Automation.Provider.ProviderCapabilities"/>
        /// of ExpandWildcards, Filter, Include, or Exclude should ensure that 
        /// the path passed meets those requirements by accessing the 
        /// appropriate property from the base class.
        /// 
        /// By default, overrides of this method should not move properties on 
        /// or to objects that are generally hidden from the user unless the 
        /// Force property is set to true. An error should be sent to the 
        /// <see cref="System.Management.Automation.CmdletProvider.WriteError"/> 
        /// method if the path represents an item that is hidden from the user 
        /// and Force is set to false.
        /// 
        /// This method should call 
        /// <see cref="System.Management.Automation.CmdletProvider.ShouldProcess"/> 
        /// and check its return value before making any changes to the store 
        /// this provider is working upon.
        /// </remarks>
        public void MoveProperty( string sourcePath, string sourceProperty,
                        string destinationPath, string destinationProperty )
        {
            // Write code to move property of an item
            // here after performing necessary validations
            //
            // WritePropertyObject( destinationProperty, destinationPath );

            // Example
            // 
            // if (ShouldProcess(destinationPath, "move property"))
            // {
            //      // Move the properties and then call WritePropertyObject
            //      WritePropertyObject(destinationProperty, destinationPath);
            // }

        } // MoveProperty

        /// <summary>
        /// Allows the provider to attach additional parameters to the
        /// move-itemproperty cmdlet.
        /// </summary>
        /// 
        /// <param name="sourcePath">
        /// If the path was specified on the command line, this is the path
        /// to the item to get the dynamic parameters for.
        /// </param>
        /// 
        /// <param name="sourceProperty">
        /// The name of the property to move.
        /// </param>
        /// 
        /// <param name="destinationPath">
        /// The path to the item on which to move the property to.
        /// </param>
        /// 
        /// <param name="destinationProperty">
        /// The destination property to move to.
        /// </param>
        /// 
        /// <returns>
        /// An object that has properties and fields decorated with parsing
        /// attributes similar to a cmdlet class or a
        /// <see cref="System.Management.Automation.PseudoParameterDictionary"/>.
        /// Null can be returned if no dynamic parameters are to be added.
        /// </returns>
        /// 
        /// <remarks>
        /// The default implementation returns null.
        /// </remarks>
        public object MovePropertyDynamicParameters( string sourcePath,
                            string sourceProperty, string destinationPath,
                                string destinationProperty )
        {
            return null;
        } // MovePropertyDynamicParameters

        /// <summary>
        /// Creates a new property on the specified item
        /// </summary>
        /// 
        /// <param name="path">
        /// The path to the item on which the new property should be created.
        /// </param>
        /// 
        /// <param name="propertyName">
        /// The name of the property that should be created.
        /// </param>
        /// 
        /// <param name="type">
        /// The type of the property that should be created.
        /// </param>
        /// 
        /// <param name="value">
        /// The new value of the property that should be created.
        /// </param>
        ///
        /// <remarks>
        /// Providers override this method to give the user the ability to add
        /// properties to provider objects using the new-itemproperty cmdlet.
        /// 
        /// Providers that declare 
        /// <see cref="System.Management.Automation.Provider.ProviderCapabilities"/>
        /// of ExpandWildcards, Filter, Include, or Exclude should ensure that 
        /// the path passed meets those requirements by accessing the
        /// appropriate property from the base class.
        /// 
        /// By default, overrides of this method should not create new 
        /// properties on objects that are generally hidden from the user unless
        /// the Force property is set to true. An error should be sent to the 
        /// <see cref="Sytem.Management.Automation.CmdletProvider.WriteError"/> 
        /// method if the path represents an item that is hidden from the user
        /// and Force is set to false.
        /// 
        /// This method should call 
        /// <see cref="System.Management.Automation.CmdletProvider.ShouldProcess"/> 
        /// and check its return value before making any changes to the store 
        /// this provider is working upon.
        /// </remarks>
        public void NewProperty( string path, string propertyName, string type,
                        object value )
        {
            // Write code here to create a new property
            // after performing necessary validations
            //
            // WritePropertyObject( propertyValue, path );

            // setting 
            // 
            // if (ShouldProcess(path, "new property"))
            // {
            //      // Set the new property and then call WritePropertyObject
            //      WritePropertyObject(value, path);
            // }
        } // NewProperty
         
        /// <summary>
        /// Allows the provider to attach additional parameters to the
        /// new-itemproperty cmdlet.
        /// </summary>
        /// 
        /// <param name="path">
        /// If the path was specified on the command line, this is the path
        /// to the item to get the dynamic parameters for.
        /// </param>
        /// 
        /// <param name="propertyName">
        /// The name of the property that should be created.
        /// </param>
        /// 
        /// <param name="type">
        /// The type of the property that should be created.
        /// </param>
        /// 
        /// <param name="value">
        /// The new value of the property that should be created.
        /// </param>
        /// 
        /// <returns>
        /// An object that has properties and fields decorated with parsing
        /// attributes similar to a cmdlet class or a
        /// <see cref="System.Management.Automation.PseudoParameterDictionary"/>.
        /// Null can be returned if no dynamic parameters are to be added.
        /// </returns>
        /// 
        /// <remarks>
        /// The default implementation returns null.
        /// </remarks>
        public object NewPropertyDynamicParameters( string path,
                            string propertyName, string type, object value )
        {
            return null;
        } // NewPropertyDynamicParameters

        /// <summary>
        /// Removes a property on the item specified by the path.
        /// </summary>
        /// 
        /// <param name="path">
        /// The path to the item from which the property should be removed.
        /// </param>
        /// 
        /// <param name="propertyName">
        /// The name of the property to be removed.
        /// </param>
        ///
        /// <remarks>
        /// Providers override this method to give the user the ability to
        /// remove properties from provider objects using the remove-itemproperty
        /// cmdlet.
        /// 
        /// Providers that declare
        /// <see cref="System.Management.Automation.Provider.ProviderCapabilities"/>
        /// of ExpandWildcards, Filter, Include, or Exclude should ensure that 
        /// the path passed meets those requirements by accessing the 
        /// appropriate property from the base class.
        /// 
        /// By default, overrides of this method should not remove properties 
        /// on objects that are generally hidden from the user unless the Force 
        /// property is set to true. An error should be sent to the 
        /// <see cref="System.Management.Automation.CmdletProvider.WriteError"/>
        /// method if the path represents an item that is hidden from the user 
        /// and Force is set to false.
        /// 
        /// This method should call 
        /// <see cref="System.Management.Automatin.CmdletProvider.ShouldProcess"/>
        /// and check its return value before making any changes to the store 
        /// this provider is working upon.
        /// </remarks>
        public void RemoveProperty( string path, string propertyName )
        {
            // Write code here to remove property of 
            // of an item after performing necessary
            // operations

            // Example 
            // if (ShouldProcess(propertyName, "delete"))
            //{
            //    // delete the property
            //}
        } // RemoveProperty

        /// <summary>
        /// Allows the provider to attach additional parameters to the
        /// remove-itemproperty cmdlet.
        /// </summary>
        /// 
        /// <param name="path">
        /// If the path was specified on the command line, this is the path
        /// to the item to get the dynamic parameters for.
        /// </param>
        /// 
        /// <param name="propertyName">
        /// The name of the property that should be removed.
        /// </param>
        /// 
        /// <returns>
        /// An object that has properties and fields decorated with parsing
        /// attributes similar to a cmdlet class or a 
        /// <see cref="System.Management.Automation.PseudoParameterDictionary"/>.
        /// Null can be returned if no dynamic parameters are to be added.
        /// </returns>
        /// 
        /// <remarks>
        /// The default implementation returns null.
        /// </remarks>
        public object RemovePropertyDynamicParameters( string path,
            string propertyName )
        {
            return null;
        } // RemovePropertyDynamicParameters

        /// <summary>
        /// Renames a property of the item at the specified path
        /// </summary>
        /// 
        /// <param name="path">
        /// The path to the item on which to rename the property.
        /// </param>
        /// 
        /// <param name="sourceProperty">
        /// The property to rename.
        /// </param>
        /// 
        /// <param name="destinationProperty">
        /// The new name of the property.
        /// </param>
        ///
        /// <remarks>
        /// Providers override this method to give the user the ability to 
        /// rename properties of provider objects using the rename-itemproperty
        /// cmdlet.
        /// 
        /// Providers that declare 
        /// <see cref="System.Management.Automation.Provider.ProviderCapabilities"/>
        /// of ExpandWildcards, Filter, Include, or Exclude should ensure that 
        /// the path passed meets those requirements by accessing the
        /// appropriate property from the base class.
        /// 
        /// By default, overrides of this method should not rename properties 
        /// on objects that are generally hidden from the user unless the Force
        /// property is set to true. An error should be sent to the 
        /// <see cref="System.Management.Automation.CmdletProvider.WriteError"/> 
        /// method if the path represents an item that is hidden from the user 
        /// and Force is set to false.
        /// 
        /// This method should call
        /// <see cref="System.Management.Automation.CmdletProvider.ShouldProcess"/> 
        /// and check its return value before making any changes to the store 
        /// this provider is working upon.
        /// </remarks>
        public void RenameProperty( string path, string sourceProperty,
            string destinationProperty )
        {
            // Write code here to rename a property after
            // performing necessary validaitions
            //
            // WritePropertyObject(destinationProperty, path);

            // Example 
            // if (ShouldProcess(destinationProperty, "delete"))
            //{
            //    // Delete property here
            //}
        } // RenameProperty

        /// <summary>
        /// Allows the provider to attach additional parameters to the
        /// rename-itemproperty cmdlet.
        /// </summary>
        /// 
        /// <param name="path">
        /// If the path was specified on the command line, this is the path
        /// to the item to get the dynamic parameters for.
        /// </param>
        /// 
        /// <param name="sourceProperty">
        /// The property to rename.
        /// </param>
        /// 
        /// <param name="destinationProperty">
        /// The new name of the property.
        /// </param>
        /// 
        /// <returns>
        /// An object that has properties and fields decorated with parsing
        /// attributes similar to a cmdlet class or a
        /// <see cref="System.Management.Automation.PseudoParameterDictionary"/>.
        /// Null can be returned if no dynamic parameters are to be added.
        /// </returns>
        /// 
        /// <remarks>
        /// The default implementation returns null.
        /// </remarks>
        public object RenamePropertyDynamicParameters( string path,
                            string sourceProperty, string destinationProperty )
        {
            return null;
        } // RenamePropertyDynamicParameters

        #endregion IDynamicPropertyCmdletProvider Overrides

        // ISecurityDescriptorCmdletProvider is an interface for a provider 
        // that hooks into the Windows PowerShell namespace and exposes
        // security descriptors of an item.
        //
        // An ISecurityDescriptorCmdletProvider provides an interface that 
        // allows simplified interaction with namespaces that support security 
        // descriptors. The methods on this interface allow a common set of 
        // commands to manage the security on any namespace that supports this 
        //interface.

        #region ISecurityDescriptorCmdletProvider Overrides

        /// <summary>
        /// When implemented, gets the security descriptor for the item
        /// specified by the path.
        /// </summary>
        /// 
        /// <param name="path">
        /// The path to the item to retrieve the security descriptor from.
        /// </param>
        /// 
        /// <param name="sections">
        /// Specifies the parts of a security descriptor to retrieve.
        /// </param>
        /// 
        /// <remarks>
        /// An instance of an object that represents the security descriptor
        /// for the item specifed by the path should be written to the
        /// <see cref="System.Management.Automation.CmdletProvider.WriteSecurityDescriptorObject"/> 
        /// method.
        /// </remarks>
        public void GetSecurityDescriptor( string path,
                        AccessControlSections sections )
        {
            // Write code here to get the
            // security descriptor for the item
            // at the specified path

        } // GetSecurityDescriptor

        /// <summary>
        /// Sets the security descriptor for the item specified by the path.
        /// </summary>
        /// 
        /// <param name="path">
        /// The path to the item to set the new security descriptor on.
        /// </param>
        /// 
        /// <param name="securityDescriptor">
        /// The new security descriptor for the item.
        /// </param>
        /// 
        /// <remarks>
        /// An instance of an object that represents the security descriptor
        /// for the item specifed by the path should be written to the
        /// <see cref="System.Management.Automation.CmdletProvider.WriteSecurityDescriptorObject"/> 
        /// method.
        /// 
        /// This method should call
        /// <see cref="System.Management.Automation.CmdletProvider.ShouldProcess"/>
        /// and check its return value before making any changes to the store 
        /// this provider is working upon.
        /// </remarks>
        public void SetSecurityDescriptor( string path,
                        ObjectSecurity securityDescriptor )
        {
            // Write code for setting the new security 
            // descriptor of the object here
            //
            // WriteSecurityDescriptorObject( path, securityDescriptor );

        } // SetSecurityDescriptor

        /// <summary>
        /// Creates a new empty security descriptor of type specified by the 
        /// path. For example, if path points to a file system directory,
        /// the descriptor returned will be of type DirectorySecurity.
        /// </summary>
        /// 
        /// <param name="path">
        /// The path to the item whose type is to be used when creating a new 
        /// descriptor
        /// </param>
        /// 
        /// <param name="sections">
        /// Specifies the parts of a security descriptor to create.
        /// </param>
        /// 
        /// <returns>
        /// A new ObjectSecurity object of the same type as the item specified 
        /// by the path.
        /// </returns>
        /// 
        public ObjectSecurity NewSecurityDescriptorFromPath( string path,
                                    AccessControlSections sections)
        {
            ObjectSecurity sd = null;

            return sd;
        } // NewSecurityDescriptorFromPath

        /// <summary>
        /// Creates a new empty security descriptor of the specified type.
        /// This method is used as a convenience function for consumers of
        /// your provider.
        /// </summary>
        /// 
        /// <param name="type">
        /// The type of Security Descriptor to create.  Your provider should
        /// understand a string representation for each of the types of 
        /// SecurityDescriptors that it supports.  For example, the File System
        /// provider performs a case-insensitive comparison against "file" for a
        /// FileSecurity descriptor, and "directory" or "container" for a 
        /// DirectorySecurity descriptor.
        /// </param>
        /// 
        /// <param name="sections">
        /// Specifies the parts of a security descriptor to create.
        /// </param>
        /// 
        /// <returns>
        /// A new ObjectSecurity object of the specified type.
        /// </returns>
        /// 
        public ObjectSecurity NewSecurityDescriptorOfType( string type,
                                    AccessControlSections sections)
        {
            ObjectSecurity sd = null;

            return sd;
        } // NewSecurityDescriptorOfType

        #endregion ISecurityDescriptorCmdletProvider Overrides
    }
}

    

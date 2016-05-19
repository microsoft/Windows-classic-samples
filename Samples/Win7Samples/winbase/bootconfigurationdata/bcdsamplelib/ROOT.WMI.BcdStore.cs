// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved


namespace ROOT.WMI
{
    using System;
    using System.ComponentModel;
    using System.Management;
    using System.Collections;
    using System.Globalization;
    using System.ComponentModel.Design.Serialization;
    using System.Reflection;
    
    
    // Functions ShouldSerialize<PropertyName> are functions used by VS property browser to check if a particular property has to be serialized. These functions are added for all ValueType properties ( properties of type Int32, BOOL etc.. which cannot be set to null). These functions use Is<PropertyName>Null function. These functions are also used in the TypeConverter implementation for the properties to check for NULL value of property so that an empty value can be shown in Property browser in case of Drag and Drop in Visual studio.
    // Functions Is<PropertyName>Null() are used to check if a property is NULL.
    // Functions Reset<PropertyName> are added for Nullable Read/Write properties. These functions are used by VS designer in property browser to set a property to NULL.
    // Every property added to the class for WMI property has attributes set to define its behavior in Visual Studio designer and also to define a TypeConverter to be used.
    // An Early Bound class generated for the WMI class.BcdStore
    public class BcdStore : System.ComponentModel.Component {
        
        // Private property to hold the WMI namespace in which the class resides.
        private static string CreatedWmiNamespace = "ROOT\\WMI";
        
        // Private property to hold the name of WMI class which created this class.
        private static string CreatedClassName = "BcdStore";
        
        // Private member variable to hold the ManagementScope which is used by the various methods.
        private static System.Management.ManagementScope statMgmtScope = null;
        
        private ManagementSystemProperties PrivateSystemProperties;
        
        // Underlying lateBound WMI object.
        private System.Management.ManagementObject PrivateLateBoundObject;
        
        // Member variable to store the 'automatic commit' behavior for the class.
        private bool AutoCommitProp;
        
        // Private variable to hold the embedded property representing the instance.
        private System.Management.ManagementBaseObject embeddedObj;
        
        // The current WMI object used
        private System.Management.ManagementBaseObject curObj;
        
        // Flag to indicate if the instance is an embedded object.
        private bool isEmbedded;
        
        // Below are different overloads of constructors to initialize an instance of the class with a WMI object.
        public BcdStore() {
            this.InitializeObject(null, null, null);
        }
        
        public BcdStore(string keyFilePath) {
            this.InitializeObject(null, new System.Management.ManagementPath(BcdStore.ConstructPath(keyFilePath)), null);
        }
        
        public BcdStore(System.Management.ManagementScope mgmtScope, string keyFilePath) {
            this.InitializeObject(((System.Management.ManagementScope)(mgmtScope)), new System.Management.ManagementPath(BcdStore.ConstructPath(keyFilePath)), null);
        }
        
        public BcdStore(System.Management.ManagementPath path, System.Management.ObjectGetOptions getOptions) {
            this.InitializeObject(null, path, getOptions);
        }
        
        public BcdStore(System.Management.ManagementScope mgmtScope, System.Management.ManagementPath path) {
            this.InitializeObject(mgmtScope, path, null);
        }
        
        public BcdStore(System.Management.ManagementPath path) {
            this.InitializeObject(null, path, null);
        }
        
        public BcdStore(System.Management.ManagementScope mgmtScope, System.Management.ManagementPath path, System.Management.ObjectGetOptions getOptions) {
            this.InitializeObject(mgmtScope, path, getOptions);
        }
        
        public BcdStore(System.Management.ManagementObject theObject) {
            Initialize();
            if ((CheckIfProperClass(theObject) == true)) {
                PrivateLateBoundObject = theObject;
                PrivateSystemProperties = new ManagementSystemProperties(PrivateLateBoundObject);
                curObj = PrivateLateBoundObject;
            }
            else {
                throw new System.ArgumentException("Class name does not match.");
            }
        }
        
        public BcdStore(System.Management.ManagementBaseObject theObject) {
            Initialize();
            if ((CheckIfProperClass(theObject) == true)) {
                embeddedObj = theObject;
                PrivateSystemProperties = new ManagementSystemProperties(theObject);
                curObj = embeddedObj;
                isEmbedded = true;
            }
            else {
                throw new System.ArgumentException("Class name does not match.");
            }
        }
        
        // Property returns the namespace of the WMI class.
        [Browsable(true)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        public string OriginatingNamespace {
            get {
                return "ROOT\\WMI";
            }
        }
        
        [Browsable(true)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        public string ManagementClassName {
            get {
                string strRet = CreatedClassName;
                if ((curObj != null)) {
                    if ((curObj.ClassPath != null)) {
                        strRet = ((string)(curObj["__CLASS"]));
                        if (((strRet == null) 
                                    || (strRet == string.Empty))) {
                            strRet = CreatedClassName;
                        }
                    }
                }
                return strRet;
            }
        }
        
        // Property pointing to an embedded object to get System properties of the WMI object.
        [Browsable(true)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        public ManagementSystemProperties SystemProperties {
            get {
                return PrivateSystemProperties;
            }
        }
        
        // Property returning the underlying lateBound object.
        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        public System.Management.ManagementBaseObject LateBoundObject {
            get {
                return curObj;
            }
        }
        
        // ManagementScope of the object.
        [Browsable(true)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        public System.Management.ManagementScope Scope {
            get {
                if ((isEmbedded == false)) {
                    return PrivateLateBoundObject.Scope;
                }
                else {
                    return null;
                }
            }
            set {
                if ((isEmbedded == false)) {
                    PrivateLateBoundObject.Scope = value;
                }
            }
        }
        
        // Property to show the commit behavior for the WMI object. If true, WMI object will be automatically saved after each property modification.(ie. Put() is called after modification of a property).
        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        public bool AutoCommit {
            get {
                return AutoCommitProp;
            }
            set {
                AutoCommitProp = value;
            }
        }
        
        // The ManagementPath of the underlying WMI object.
        [Browsable(true)]
        public System.Management.ManagementPath Path {
            get {
                if ((isEmbedded == false)) {
                    return PrivateLateBoundObject.Path;
                }
                else {
                    return null;
                }
            }
            set {
                if ((isEmbedded == false)) {
                    if ((CheckIfProperClass(null, value, null) != true)) {
                        throw new System.ArgumentException("Class name does not match.");
                    }
                    PrivateLateBoundObject.Path = value;
                }
            }
        }
        
        // Public static scope property which is used by the various methods.
        [Browsable(true)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        public static System.Management.ManagementScope StaticScope {
            get {
                return statMgmtScope;
            }
            set {
                statMgmtScope = value;
            }
        }
        
        [Browsable(true)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        [Description("A BcdStore is uniquely identified by its file path. The system store is denoted v" +
            "ia an empty file path.")]
        public string FilePath {
            get {
                return ((string)(curObj["FilePath"]));
            }
        }
        
        private bool CheckIfProperClass(System.Management.ManagementScope mgmtScope, System.Management.ManagementPath path, System.Management.ObjectGetOptions OptionsParam) {
            if (((path != null) 
                        && (string.Compare(path.ClassName, this.ManagementClassName, true, System.Globalization.CultureInfo.InvariantCulture) == 0))) {
                return true;
            }
            else {
                return CheckIfProperClass(new System.Management.ManagementObject(mgmtScope, path, OptionsParam));
            }
        }
        
        private bool CheckIfProperClass(System.Management.ManagementBaseObject theObj) {
            if (((theObj != null) 
                        && (string.Compare(((string)(theObj["__CLASS"])), this.ManagementClassName, true, System.Globalization.CultureInfo.InvariantCulture) == 0))) {
                return true;
            }
            else {
                System.Array parentClasses = ((System.Array)(theObj["__DERIVATION"]));
                if ((parentClasses != null)) {
                    int count = 0;
                    for (count = 0; (count < parentClasses.Length); count = (count + 1)) {
                        if ((string.Compare(((string)(parentClasses.GetValue(count))), this.ManagementClassName, true, System.Globalization.CultureInfo.InvariantCulture) == 0)) {
                            return true;
                        }
                    }
                }
            }
            return false;
        }
        
        [Browsable(true)]
        public void CommitObject() {
            if ((isEmbedded == false)) {
                PrivateLateBoundObject.Put();
            }
        }
        
        [Browsable(true)]
        public void CommitObject(System.Management.PutOptions putOptions) {
            if ((isEmbedded == false)) {
                PrivateLateBoundObject.Put(putOptions);
            }
        }
        
        private void Initialize() {
            AutoCommitProp = true;
            isEmbedded = false;
        }
        
        private static string ConstructPath(string keyFilePath) {
            string strPath = "ROOT\\WMI:BcdStore";
            strPath = string.Concat(strPath, string.Concat(".FilePath=", string.Concat("\"", string.Concat(keyFilePath, "\""))));
            return strPath;
        }
        
        private void InitializeObject(System.Management.ManagementScope mgmtScope, System.Management.ManagementPath path, System.Management.ObjectGetOptions getOptions) {
            Initialize();
            if ((path != null)) {
                if ((CheckIfProperClass(mgmtScope, path, getOptions) != true)) {
                    throw new System.ArgumentException("Class name does not match.");
                }
            }
            PrivateLateBoundObject = new System.Management.ManagementObject(mgmtScope, path, getOptions);
            PrivateSystemProperties = new ManagementSystemProperties(PrivateLateBoundObject);
            curObj = PrivateLateBoundObject;
        }
        
        // Different overloads of GetInstances() help in enumerating instances of the WMI class.
        public static BcdStoreCollection GetInstances() {
            return GetInstances(null, null, null);
        }
        
        public static BcdStoreCollection GetInstances(string condition) {
            return GetInstances(null, condition, null);
        }
        
        public static BcdStoreCollection GetInstances(System.String [] selectedProperties) {
            return GetInstances(null, null, selectedProperties);
        }
        
        public static BcdStoreCollection GetInstances(string condition, System.String [] selectedProperties) {
            return GetInstances(null, condition, selectedProperties);
        }
        
        public static BcdStoreCollection GetInstances(System.Management.ManagementScope mgmtScope, System.Management.EnumerationOptions enumOptions) {
            if ((mgmtScope == null)) {
                if ((statMgmtScope == null)) {
                    mgmtScope = new System.Management.ManagementScope();
                    mgmtScope.Path.NamespacePath = "root\\WMI";
                }
                else {
                    mgmtScope = statMgmtScope;
                }
            }
            System.Management.ManagementPath pathObj = new System.Management.ManagementPath();
            pathObj.ClassName = "BcdStore";
            pathObj.NamespacePath = "root\\WMI";
            System.Management.ManagementClass clsObject = new System.Management.ManagementClass(mgmtScope, pathObj, null);
            if ((enumOptions == null)) {
                enumOptions = new System.Management.EnumerationOptions();
                enumOptions.EnsureLocatable = true;
            }
            return new BcdStoreCollection(clsObject.GetInstances(enumOptions));
        }
        
        public static BcdStoreCollection GetInstances(System.Management.ManagementScope mgmtScope, string condition) {
            return GetInstances(mgmtScope, condition, null);
        }
        
        public static BcdStoreCollection GetInstances(System.Management.ManagementScope mgmtScope, System.String [] selectedProperties) {
            return GetInstances(mgmtScope, null, selectedProperties);
        }
        
        public static BcdStoreCollection GetInstances(System.Management.ManagementScope mgmtScope, string condition, System.String [] selectedProperties) {
            if ((mgmtScope == null)) {
                if ((statMgmtScope == null)) {
                    mgmtScope = new System.Management.ManagementScope();
                    mgmtScope.Path.NamespacePath = "root\\WMI";
                }
                else {
                    mgmtScope = statMgmtScope;
                }
            }
            System.Management.ManagementObjectSearcher ObjectSearcher = new System.Management.ManagementObjectSearcher(mgmtScope, new SelectQuery("BcdStore", condition, selectedProperties));
            System.Management.EnumerationOptions enumOptions = new System.Management.EnumerationOptions();
            enumOptions.EnsureLocatable = true;
            ObjectSearcher.Options = enumOptions;
            return new BcdStoreCollection(ObjectSearcher.Get());
        }
        
        [Browsable(true)]
        public static BcdStore CreateInstance() {
            System.Management.ManagementScope mgmtScope = null;
            if ((statMgmtScope == null)) {
                mgmtScope = new System.Management.ManagementScope();
                mgmtScope.Path.NamespacePath = CreatedWmiNamespace;
            }
            else {
                mgmtScope = statMgmtScope;
            }
            System.Management.ManagementPath mgmtPath = new System.Management.ManagementPath(CreatedClassName);
            System.Management.ManagementClass tmpMgmtClass = new System.Management.ManagementClass(mgmtScope, mgmtPath, null);
            return new BcdStore(tmpMgmtClass.CreateInstance());
        }
        
        [Browsable(true)]
        public void Delete() {
            PrivateLateBoundObject.Delete();
        }
        
        public bool CopyObject(uint Flags, string SourceId, string SourceStoreFile, out System.Management.ManagementBaseObject Object) {
            if ((isEmbedded == false)) {
                System.Management.ManagementBaseObject inParams = null;
                inParams = PrivateLateBoundObject.GetMethodParameters("CopyObject");
                inParams["Flags"] = ((System.UInt32 )(Flags));
                inParams["SourceId"] = ((System.String )(SourceId));
                inParams["SourceStoreFile"] = ((System.String )(SourceStoreFile));
                System.Management.ManagementBaseObject outParams = PrivateLateBoundObject.InvokeMethod("CopyObject", inParams, null);
                Object = ((System.Management.ManagementBaseObject)(outParams.Properties["Object"].Value));
                return System.Convert.ToBoolean(outParams.Properties["ReturnValue"].Value);
            }
            else {
                Object = null;
                return System.Convert.ToBoolean(0);
            }
        }
        
        public bool CopyObjects(uint Flags, string SourceStoreFile, uint Type) {
            if ((isEmbedded == false)) {
                System.Management.ManagementBaseObject inParams = null;
                inParams = PrivateLateBoundObject.GetMethodParameters("CopyObjects");
                inParams["Flags"] = ((System.UInt32 )(Flags));
                inParams["SourceStoreFile"] = ((System.String )(SourceStoreFile));
                inParams["Type"] = ((System.UInt32 )(Type));
                System.Management.ManagementBaseObject outParams = PrivateLateBoundObject.InvokeMethod("CopyObjects", inParams, null);
                return System.Convert.ToBoolean(outParams.Properties["ReturnValue"].Value);
            }
            else {
                return System.Convert.ToBoolean(0);
            }
        }
        
        public bool CreateObject(string Id, uint Type, out System.Management.ManagementBaseObject Object) {
            if ((isEmbedded == false)) {
                System.Management.ManagementBaseObject inParams = null;
                inParams = PrivateLateBoundObject.GetMethodParameters("CreateObject");
                inParams["Id"] = ((System.String )(Id));
                inParams["Type"] = ((System.UInt32 )(Type));
                System.Management.ManagementBaseObject outParams = PrivateLateBoundObject.InvokeMethod("CreateObject", inParams, null);
                Object = ((System.Management.ManagementBaseObject)(outParams.Properties["Object"].Value));
                return System.Convert.ToBoolean(outParams.Properties["ReturnValue"].Value);
            }
            else {
                Object = null;
                return System.Convert.ToBoolean(0);
            }
        }
        
        public static bool CreateStore(string File, out System.Management.ManagementBaseObject Store) {
            bool IsMethodStatic = true;
            if ((IsMethodStatic == true)) {
                System.Management.ManagementBaseObject inParams = null;
                System.Management.ManagementPath mgmtPath = new System.Management.ManagementPath(CreatedClassName);
                System.Management.ManagementClass classObj = new System.Management.ManagementClass(statMgmtScope, mgmtPath, null);
                inParams = classObj.GetMethodParameters("CreateStore");
                inParams["File"] = ((System.String )(File));
                System.Management.ManagementBaseObject outParams = classObj.InvokeMethod("CreateStore", inParams, null);
                Store = ((System.Management.ManagementBaseObject)(outParams.Properties["Store"].Value));
                return System.Convert.ToBoolean(outParams.Properties["ReturnValue"].Value);
            }
            else {
                Store = null;
                return System.Convert.ToBoolean(0);
            }
        }
        
        public bool DeleteObject(string Id) {
            if ((isEmbedded == false)) {
                System.Management.ManagementBaseObject inParams = null;
                inParams = PrivateLateBoundObject.GetMethodParameters("DeleteObject");
                inParams["Id"] = ((System.String )(Id));
                System.Management.ManagementBaseObject outParams = PrivateLateBoundObject.InvokeMethod("DeleteObject", inParams, null);
                return System.Convert.ToBoolean(outParams.Properties["ReturnValue"].Value);
            }
            else {
                return System.Convert.ToBoolean(0);
            }
        }
        
        public static bool DeleteSystemStore() {
            bool IsMethodStatic = true;
            if ((IsMethodStatic == true)) {
                System.Management.ManagementBaseObject inParams = null;
                System.Management.ManagementPath mgmtPath = new System.Management.ManagementPath(CreatedClassName);
                System.Management.ManagementClass classObj = new System.Management.ManagementClass(statMgmtScope, mgmtPath, null);
                System.Management.ManagementBaseObject outParams = classObj.InvokeMethod("DeleteSystemStore", inParams, null);
                return System.Convert.ToBoolean(outParams.Properties["ReturnValue"].Value);
            }
            else {
                return System.Convert.ToBoolean(0);
            }
        }
        
        public bool EnumerateObjects(uint Type, out System.Management.ManagementBaseObject[] Objects) {
            if ((isEmbedded == false)) {
                System.Management.ManagementBaseObject inParams = null;
                inParams = PrivateLateBoundObject.GetMethodParameters("EnumerateObjects");
                inParams["Type"] = ((System.UInt32 )(Type));
                System.Management.ManagementBaseObject outParams = PrivateLateBoundObject.InvokeMethod("EnumerateObjects", inParams, null);
                Objects = ((System.Management.ManagementBaseObject[])(outParams.Properties["Objects"].Value));
                return System.Convert.ToBoolean(outParams.Properties["ReturnValue"].Value);
            }
            else {
                Objects = null;
                return System.Convert.ToBoolean(0);
            }
        }
        
        public static bool ExportStore(string File) {
            bool IsMethodStatic = true;
            if ((IsMethodStatic == true)) {
                System.Management.ManagementBaseObject inParams = null;
                System.Management.ManagementPath mgmtPath = new System.Management.ManagementPath(CreatedClassName);
                System.Management.ManagementClass classObj = new System.Management.ManagementClass(statMgmtScope, mgmtPath, null);
                inParams = classObj.GetMethodParameters("ExportStore");
                inParams["File"] = ((System.String )(File));
                System.Management.ManagementBaseObject outParams = classObj.InvokeMethod("ExportStore", inParams, null);
                return System.Convert.ToBoolean(outParams.Properties["ReturnValue"].Value);
            }
            else {
                return System.Convert.ToBoolean(0);
            }
        }
        
        public static bool GetSystemDisk(out string Disk) {
            bool IsMethodStatic = true;
            if ((IsMethodStatic == true)) {
                System.Management.ManagementBaseObject inParams = null;
                System.Management.ManagementPath mgmtPath = new System.Management.ManagementPath(CreatedClassName);
                System.Management.ManagementClass classObj = new System.Management.ManagementClass(statMgmtScope, mgmtPath, null);
                System.Management.ManagementBaseObject outParams = classObj.InvokeMethod("GetSystemDisk", inParams, null);
                Disk = System.Convert.ToString(outParams.Properties["Disk"].Value);
                return System.Convert.ToBoolean(outParams.Properties["ReturnValue"].Value);
            }
            else {
                Disk = null;
                return System.Convert.ToBoolean(0);
            }
        }
        
        public static bool GetSystemPartition(out string Partition) {
            bool IsMethodStatic = true;
            if ((IsMethodStatic == true)) {
                System.Management.ManagementBaseObject inParams = null;
                System.Management.ManagementPath mgmtPath = new System.Management.ManagementPath(CreatedClassName);
                System.Management.ManagementClass classObj = new System.Management.ManagementClass(statMgmtScope, mgmtPath, null);
                System.Management.ManagementBaseObject outParams = classObj.InvokeMethod("GetSystemPartition", inParams, null);
                Partition = System.Convert.ToString(outParams.Properties["Partition"].Value);
                return System.Convert.ToBoolean(outParams.Properties["ReturnValue"].Value);
            }
            else {
                Partition = null;
                return System.Convert.ToBoolean(0);
            }
        }
        
        public static bool ImportStore(string File) {
            bool IsMethodStatic = true;
            if ((IsMethodStatic == true)) {
                System.Management.ManagementBaseObject inParams = null;
                System.Management.ManagementPath mgmtPath = new System.Management.ManagementPath(CreatedClassName);
                System.Management.ManagementClass classObj = new System.Management.ManagementClass(statMgmtScope, mgmtPath, null);
                inParams = classObj.GetMethodParameters("ImportStore");
                inParams["File"] = ((System.String )(File));
                System.Management.ManagementBaseObject outParams = classObj.InvokeMethod("ImportStore", inParams, null);
                return System.Convert.ToBoolean(outParams.Properties["ReturnValue"].Value);
            }
            else {
                return System.Convert.ToBoolean(0);
            }
        }
        
        public bool OpenObject(string Id, out System.Management.ManagementBaseObject Object) {
            if ((isEmbedded == false)) {
                System.Management.ManagementBaseObject inParams = null;
                inParams = PrivateLateBoundObject.GetMethodParameters("OpenObject");
                inParams["Id"] = ((System.String )(Id));
                System.Management.ManagementBaseObject outParams = PrivateLateBoundObject.InvokeMethod("OpenObject", inParams, null);
                Object = ((System.Management.ManagementBaseObject)(outParams.Properties["Object"].Value));
                return System.Convert.ToBoolean(outParams.Properties["ReturnValue"].Value);
            }
            else {
                Object = null;
                return System.Convert.ToBoolean(0);
            }
        }
        
        public static bool OpenStore(string File, out System.Management.ManagementBaseObject Store) {
            bool IsMethodStatic = true;
            if ((IsMethodStatic == true)) {
                System.Management.ManagementBaseObject inParams = null;
                System.Management.ManagementPath mgmtPath = new System.Management.ManagementPath(CreatedClassName);
                System.Management.ManagementClass classObj = new System.Management.ManagementClass(statMgmtScope, mgmtPath, null);
                inParams = classObj.GetMethodParameters("OpenStore");
                inParams["File"] = ((System.String )(File));
                System.Management.ManagementBaseObject outParams = classObj.InvokeMethod("OpenStore", inParams, null);
                Store = ((System.Management.ManagementBaseObject)(outParams.Properties["Store"].Value));
                return System.Convert.ToBoolean(outParams.Properties["ReturnValue"].Value);
            }
            else {
                Store = null;
                return System.Convert.ToBoolean(0);
            }
        }
        
        // Enumerator implementation for enumerating instances of the class.
        public class BcdStoreCollection : object, ICollection {
            
            private ManagementObjectCollection privColObj;
            
            public BcdStoreCollection(ManagementObjectCollection objCollection) {
                privColObj = objCollection;
            }
            
            public virtual int Count {
                get {
                    return privColObj.Count;
                }
            }
            
            public virtual bool IsSynchronized {
                get {
                    return privColObj.IsSynchronized;
                }
            }
            
            public virtual object SyncRoot {
                get {
                    return this;
                }
            }
            
            public virtual void CopyTo(System.Array array, int index) {
                privColObj.CopyTo(array, index);
                int nCtr;
                for (nCtr = 0; (nCtr < array.Length); nCtr = (nCtr + 1)) {
                    array.SetValue(new BcdStore(((System.Management.ManagementObject)(array.GetValue(nCtr)))), nCtr);
                }
            }
            
            public virtual System.Collections.IEnumerator GetEnumerator() {
                return new BcdStoreEnumerator(privColObj.GetEnumerator());
            }
            
            public class BcdStoreEnumerator : object, System.Collections.IEnumerator {
                
                private ManagementObjectCollection.ManagementObjectEnumerator privObjEnum;
                
                public BcdStoreEnumerator(ManagementObjectCollection.ManagementObjectEnumerator objEnum) {
                    privObjEnum = objEnum;
                }
                
                public virtual object Current {
                    get {
                        return new BcdStore(((System.Management.ManagementObject)(privObjEnum.Current)));
                    }
                }
                
                public virtual bool MoveNext() {
                    return privObjEnum.MoveNext();
                }
                
                public virtual void Reset() {
                    privObjEnum.Reset();
                }
            }
        }
        
        // TypeConverter to handle null values for ValueType properties
        public class WMIValueTypeConverter : TypeConverter {
            
            private TypeConverter baseConverter;
            
            private System.Type baseType;
            
            public WMIValueTypeConverter(System.Type inBaseType) {
                baseConverter = TypeDescriptor.GetConverter(inBaseType);
                baseType = inBaseType;
            }
            
            public override bool CanConvertFrom(System.ComponentModel.ITypeDescriptorContext context, System.Type srcType) {
                return baseConverter.CanConvertFrom(context, srcType);
            }
            
            public override bool CanConvertTo(System.ComponentModel.ITypeDescriptorContext context, System.Type destinationType) {
                return baseConverter.CanConvertTo(context, destinationType);
            }
            
            public override object ConvertFrom(System.ComponentModel.ITypeDescriptorContext context, System.Globalization.CultureInfo culture, object value) {
                return baseConverter.ConvertFrom(context, culture, value);
            }
            
            public override object CreateInstance(System.ComponentModel.ITypeDescriptorContext context, System.Collections.IDictionary dictionary) {
                return baseConverter.CreateInstance(context, dictionary);
            }
            
            public override bool GetCreateInstanceSupported(System.ComponentModel.ITypeDescriptorContext context) {
                return baseConverter.GetCreateInstanceSupported(context);
            }
            
            public override PropertyDescriptorCollection GetProperties(System.ComponentModel.ITypeDescriptorContext context, object value, System.Attribute[] attributeVar) {
                return baseConverter.GetProperties(context, value, attributeVar);
            }
            
            public override bool GetPropertiesSupported(System.ComponentModel.ITypeDescriptorContext context) {
                return baseConverter.GetPropertiesSupported(context);
            }
            
            public override System.ComponentModel.TypeConverter.StandardValuesCollection GetStandardValues(System.ComponentModel.ITypeDescriptorContext context) {
                return baseConverter.GetStandardValues(context);
            }
            
            public override bool GetStandardValuesExclusive(System.ComponentModel.ITypeDescriptorContext context) {
                return baseConverter.GetStandardValuesExclusive(context);
            }
            
            public override bool GetStandardValuesSupported(System.ComponentModel.ITypeDescriptorContext context) {
                return baseConverter.GetStandardValuesSupported(context);
            }
            
            public override object ConvertTo(System.ComponentModel.ITypeDescriptorContext context, System.Globalization.CultureInfo culture, object value, System.Type destinationType) {
                if ((baseType.BaseType == typeof(System.Enum))) {
                    if ((value.GetType() == destinationType)) {
                        return value;
                    }
                    if ((((value == null) 
                                && (context != null)) 
                                && (context.PropertyDescriptor.ShouldSerializeValue(context.Instance) == false))) {
                        return  "NULL_ENUM_VALUE" ;
                    }
                    return baseConverter.ConvertTo(context, culture, value, destinationType);
                }
                if (((baseType == typeof(bool)) 
                            && (baseType.BaseType == typeof(System.ValueType)))) {
                    if ((((value == null) 
                                && (context != null)) 
                                && (context.PropertyDescriptor.ShouldSerializeValue(context.Instance) == false))) {
                        return "";
                    }
                    return baseConverter.ConvertTo(context, culture, value, destinationType);
                }
                if (((context != null) 
                            && (context.PropertyDescriptor.ShouldSerializeValue(context.Instance) == false))) {
                    return "";
                }
                return baseConverter.ConvertTo(context, culture, value, destinationType);
            }
        }
        
        // Embedded class to represent WMI system Properties.
        [TypeConverter(typeof(System.ComponentModel.ExpandableObjectConverter))]
        public class ManagementSystemProperties {
            
            private System.Management.ManagementBaseObject PrivateLateBoundObject;
            
            public ManagementSystemProperties(System.Management.ManagementBaseObject ManagedObject) {
                PrivateLateBoundObject = ManagedObject;
            }
            
            [Browsable(true)]
            public int GENUS {
                get {
                    return ((int)(PrivateLateBoundObject["__GENUS"]));
                }
            }
            
            [Browsable(true)]
            public string CLASS {
                get {
                    return ((string)(PrivateLateBoundObject["__CLASS"]));
                }
            }
            
            [Browsable(true)]
            public string SUPERCLASS {
                get {
                    return ((string)(PrivateLateBoundObject["__SUPERCLASS"]));
                }
            }
            
            [Browsable(true)]
            public string DYNASTY {
                get {
                    return ((string)(PrivateLateBoundObject["__DYNASTY"]));
                }
            }
            
            [Browsable(true)]
            public string RELPATH {
                get {
                    return ((string)(PrivateLateBoundObject["__RELPATH"]));
                }
            }
            
            [Browsable(true)]
            public int PROPERTY_COUNT {
                get {
                    return ((int)(PrivateLateBoundObject["__PROPERTY_COUNT"]));
                }
            }
            
            [Browsable(true)]
            public string[] DERIVATION {
                get {
                    return ((string[])(PrivateLateBoundObject["__DERIVATION"]));
                }
            }
            
            [Browsable(true)]
            public string SERVER {
                get {
                    return ((string)(PrivateLateBoundObject["__SERVER"]));
                }
            }
            
            [Browsable(true)]
            public string NAMESPACE {
                get {
                    return ((string)(PrivateLateBoundObject["__NAMESPACE"]));
                }
            }
            
            [Browsable(true)]
            public string PATH {
                get {
                    return ((string)(PrivateLateBoundObject["__PATH"]));
                }
            }
        }
    }
}

// -----------------------------------------------------------------------
// <copyright file="Program.cs" company="Microsoft">
//     Copyright (C) 2012 Microsoft Corporation
// </copyright>
// -----------------------------------------------------------------------

namespace Microsoft.Samples.Management.OData.AssociationClient
{
    using System;
    using System.Collections.Generic;
    using System.Net;

    /// <summary>
    /// Program startup
    /// </summary>
    public class Program
    {
        /// <summary>
        /// Constant for HTTP GET verb
        /// </summary>
        private const string HttpVerbGet = "GET";

        /// <summary>
        /// Constant for HTTP POST verb
        /// </summary>
        private const string HttpVerbPost = "POST";

        /// <summary>
        /// Constant for HTTP PUT verb
        /// </summary>
        private const string HttpVerbPut = "PUT";

        /// <summary>
        /// Constant for HTTP DELETE verb
        /// </summary>
        private const string HttpVerbDelete = "DELETE";

        /// <summary>
        /// Default URI value for the server where association sever is setup. This is the URI created from RoleBasedPlugin sample
        /// </summary>
        private const string DefaultUri = "http://localhost:7000/MODataSvc/Microsoft.Management.Odata.svc";

        /// <summary>
        /// Default authentication type
        /// </summary>
        private const string DefaultAuthType = "Basic";

        /// <summary>
        /// Uri of the server where Association is setup
        /// </summary>
        private static Uri uri = new Uri(DefaultUri);

        /// <summary>
        /// User name to be used for connecting to the server
        /// </summary>
        private static string userName;

        /// <summary>
        /// Password to be used for connecting to server
        /// </summary>
        private static string password;

        /// <summary>
        /// Authentication type used
        /// </summary>
        private static string authType = DefaultAuthType;

        /// <summary>
        /// Domain name for the user
        /// </summary>
        private static string domainName;

        /// <summary>
        /// Main routine
        /// </summary>
        /// <param name="args">Command line arguments</param>
        public static void Main(string[] args)
        {
            if (ParseInputArgs(args) == false)
            {
                return;
            }

            // Get all Physical machines
            OdataJsonRequest request = new OdataJsonRequest(HttpVerbGet, uri.ToString() + "/PhysicalMachines", GetCredentials(uri));
            request.SendReceive();
            if (request.ResponseStatus == HttpStatusCode.OK)
            {
                Console.WriteLine("\nReceived all physical machines");
            }
            else
            {
                Console.WriteLine("\nFailed to get physical machines. Http Status Code " + request.ResponseStatus + "\n Response message: " + request.ResponseBody);
                return;
            }

            List<PhysicalMachineResource> physicalMachines = new List<PhysicalMachineResource>();

            foreach (dynamic machine in request.Response.d.results)
            {
                PhysicalMachineResource physicalMachine = new PhysicalMachineResource();
                physicalMachine.Name = machine.Name;

                Console.WriteLine("Physical machine name: " + physicalMachine.Name);

                physicalMachines.Add(physicalMachine);
            }

            // Get Physical machines with associated VMs
            request = new OdataJsonRequest(HttpVerbGet, uri.ToString() + "/PhysicalMachines?$expand=VMs", GetCredentials(uri));
            request.SendReceive();
            if (request.ResponseStatus == HttpStatusCode.OK)
            {
                Console.WriteLine("\nReceived all physical machine details with all VMs");
            }
            else
            {
                Console.WriteLine("\nFailed to get physical machine with all VMs details. Http Status Code " + request.ResponseStatus + "\n Response message: " + request.ResponseBody);
                return;
            }

            physicalMachines = new List<PhysicalMachineResource>();

            foreach (dynamic machine in request.Response.d.results)
            {
                PhysicalMachineResource physicalMachine = new PhysicalMachineResource();
                physicalMachine.Name = machine.Name;
                foreach (dynamic vm in machine.VMs.results)
                {
                    VmResource resource = new VmResource();
                    resource.Id = Guid.Parse(vm.Id);
                    resource.MachineName = vm.MachineName;
                    resource.OS = vm.OS;

                    physicalMachine.VMs.Add(resource);
                }
                
                Console.WriteLine("Physical machine name: " + physicalMachine.Name);
                foreach (VmResource vm in physicalMachine.VMs)
                {
                    Console.WriteLine("Associated virtual machine: Id " + vm.Id + " Machine Name " + vm.MachineName);
                }

                physicalMachines.Add(physicalMachine);
            }
            
            // Get list of Systems
            request = new OdataJsonRequest(HttpVerbGet, uri.ToString() + "/Systems?$expand=VMs", GetCredentials(uri));
            request.SendReceive();
            if (request.ResponseStatus == HttpStatusCode.OK)
            {
                Console.WriteLine("\nReceived all systems details");
            }
            else
            {
                Console.WriteLine("\nFailed to get systems details. Http Status Code " + request.ResponseStatus + "\n Response message: " + request.ResponseBody);
                return;
            }

            List<SystemResource> systems = new List<SystemResource>();
            foreach (dynamic sys in request.Response.d.results)
            {
                SystemResource system = new SystemResource();
                system.Id = Guid.Parse(sys.Id);
                system.Name = sys.Name;

                foreach (dynamic vm in sys.VMs.results)
                {
                    VmResource resource = new VmResource();
                    resource.Id = Guid.Parse(vm.Id);
                    resource.MachineName = vm.MachineName;
                    resource.OS = vm.OS;

                    system.VMs.Add(resource);
                }

                Console.WriteLine("System Id: " + system.Name + " Name: " + system.Name);
                foreach (VmResource vm in system.VMs)
                {
                    Console.WriteLine("Associated virtual machine: Id " + vm.Id + " Machine Name " + vm.MachineName);
                }

                systems.Add(system);
            }

            string firstSystemUri = request.Response.d.results[0].__metadata.uri;

            // Create a new Virtual machine
            request = new OdataJsonRequest(HttpVerbPost, uri.ToString() + "/VirtualMachines", GetCredentials(uri));
            request.RequestBody += "{";
            request.RequestBody += OdataJsonRequest.EncodeJsonElement("MachineName", physicalMachines[0].Name);
            request.RequestBody += ",";
            request.RequestBody += OdataJsonRequest.EncodeJsonElement("OS", "win8");
            request.RequestBody += "}";

            request.SendReceive();
            if (request.ResponseStatus == HttpStatusCode.Created)
            {
                Console.WriteLine("\nCreated a new VM " + request.Response.d.__metadata.uri);
            }
            else
            {
                Console.WriteLine("\nFailed to create a new VM. Http Status Code " + request.ResponseStatus + "\n Response message: " + request.ResponseBody);
                return;
            }

            string newVMUri = request.Response.d.__metadata.uri;

            // Get new VM resource
            request = new OdataJsonRequest(HttpVerbGet, newVMUri, GetCredentials(uri));
            request.SendReceive();
            if (request.ResponseStatus == HttpStatusCode.OK)
            {
                Console.WriteLine("\nnReceived a new VM instance " + request.Response.d.__metadata.uri);
            }
            else
            {
                Console.WriteLine("\nFailed to get new VM " + request.Response.d.__metadata.uri + "instance. Http Status Code " + request.ResponseStatus + "\n Response message: " + request.ResponseBody);
                return;
            }

            VmResource newVM = new VmResource();
            dynamic tempVm = request.Response.d;
            newVM.Id = Guid.Parse(tempVm.Id);
            newVM.MachineName = tempVm.MachineName;
            newVM.OS = tempVm.OS;
            
            // Add Virtual Machine in first system 
            request = new OdataJsonRequest(HttpVerbPost, firstSystemUri + "/$links/VMs", GetCredentials(uri));
            request.RequestBody += "{";
            request.RequestBody += OdataJsonRequest.EncodeJsonElement("uri", newVMUri);
            request.RequestBody += "}";
            request.SendReceive();
            if (request.ResponseStatus == HttpStatusCode.NoContent)
            {
                Console.WriteLine("\nAdded VM " + newVMUri + " to the System " + firstSystemUri);
            }
            else
            {
                Console.WriteLine("\nFailed to add VM " + newVMUri + " to the system " + firstSystemUri + ". Http Status Code " + request.ResponseStatus + "\n Response message: " + request.ResponseBody);
                return;
            }

            // Get list of Virtual Machines for the System
            request = new OdataJsonRequest(HttpVerbGet, firstSystemUri + "/$links/VMs", GetCredentials(uri));
            request.SendReceive();
            if (request.ResponseStatus == HttpStatusCode.OK)
            {
                Console.WriteLine("\nReceived all VMs for the system " + firstSystemUri);
            }
            else
            {
                Console.WriteLine("\nFailed to get all VMs for the system " + firstSystemUri + ". Http Status Code " + request.ResponseStatus + "\n Response message: " + request.ResponseBody);
                return;
            }

            List<string> links = new List<string>();
            foreach (dynamic d in request.Response.d.results)
            {
                links.Add(d.uri);
            }

            if (links.Contains(newVMUri))
            {
                Console.WriteLine("VM " + newVMUri + " is present in the list of VMs associated with the system " + firstSystemUri);
            }
            else
            {
                Console.WriteLine("Error: VM " + newVMUri + " is not present in the list of VMs associated with the system " + firstSystemUri);
                return;
            }

            // Remove Virtual Machine from System
            request = new OdataJsonRequest(HttpVerbDelete, firstSystemUri + "/$links/VMs(Id=guid'" + newVM.Id + "',MachineName='" + newVM.MachineName + "')", GetCredentials(uri));
            request.SendReceive();
            if (request.ResponseStatus == HttpStatusCode.NoContent)
            {
                Console.WriteLine("\nRemoved VM " + newVMUri + " from the system " + firstSystemUri);
            }
            else
            {
                Console.WriteLine("\nFailed to remove VM " + newVMUri + " from the system " + firstSystemUri + ". Http Status Code " + request.ResponseStatus + "\n Response message: " + request.ResponseBody);
                return;
            }

            // Get list of Virtual Machines for the System
            request = new OdataJsonRequest(HttpVerbGet, firstSystemUri + "/$links/VMs", GetCredentials(uri));
            request.SendReceive();
            if (request.ResponseStatus == HttpStatusCode.OK)
            {
                Console.WriteLine("\nReceived all VMs for the system " + firstSystemUri);
            }
            else
            {
                Console.WriteLine("\nFailed to get all VMs for the system " + firstSystemUri + ". Http Status Code " + request.ResponseStatus + "\n Response message: " + request.ResponseBody);
                return;
            }

            links = new List<string>();
            foreach (dynamic d in request.Response.d.results)
            {
                links.Add(d.uri);
            }

            if (links.Contains(newVMUri))
            {
                Console.WriteLine("Error: VM " + newVMUri + " is associated with the system " + firstSystemUri);
            }
            else
            {
                Console.WriteLine("VM " + newVMUri + " is not associated with the system " + firstSystemUri);
            }

            // Delete the VM
            request = new OdataJsonRequest(HttpVerbDelete, newVMUri, GetCredentials(uri));
            request.SendReceive();
            if (request.ResponseStatus == HttpStatusCode.NoContent)
            {
                Console.WriteLine("\nRemoved VM " + newVMUri);
            }
            else
            {
                Console.WriteLine("\nFailed to removed VM " + newVMUri + ". Http Status Code " + request.ResponseStatus + "\n Response message: " + request.ResponseBody);
                return;
            }

            // Get all VMs
            request = new OdataJsonRequest(HttpVerbGet, uri.ToString() + "/VirtualMachines", GetCredentials(uri));
            request.SendReceive();
            if (request.ResponseStatus == HttpStatusCode.OK)
            {
                Console.WriteLine("\nReceived all VMs ");
            }
            else
            {
                Console.WriteLine("\nFailed to receive all VMs. Http Status Code " + request.ResponseStatus + "\n Response message: " + request.ResponseBody);
                return;
            }

            List<string> allVmLinks = new List<string>();

            foreach (dynamic d in request.Response.d.Results)
            {
                allVmLinks.Add(d.__metadata.uri);
            }

            if (allVmLinks.Contains(newVMUri))
            {
                Console.WriteLine("Error: VM " + newVMUri + " is present");
            }
            else
            {
                Console.WriteLine("VM " + newVMUri + " is not present");
            }
        }

        /// <summary>
        /// Parses command line arguments passed to the sample
        /// </summary>
        /// <param name="args">Command line arguments</param>
        /// <returns>true, if found arguments correctly else false</returns>
        private static bool ParseInputArgs(string[] args)
        {
            bool invalidInput = false;
            for (int i = 0; i < args.Length; i += 2)
            {
                string key = args[i];
                string value = ((i + 1) < args.Length) ? args[i + 1] : string.Empty;

                switch (key.ToLower())
                {
                    case "-username":
                        userName = value;
                        break;
                    case "-password":
                        password = value;
                        break;
                    case "-authtype":
                        authType = value;
                        break;
                    case "-domain":
                        domainName = value;
                        break;
                    case "-uri":
                        try
                        {
                            uri = new Uri(value);
                        }
                        catch (UriFormatException ex)
                        {
                            Console.WriteLine("Error: Invalid Uri passed- " + ex.Message);
                            invalidInput = true;
                        }

                        break;
                    default:
                        invalidInput = true;
                        break;
                }
            }

            if (invalidInput || string.IsNullOrEmpty(userName) || string.IsNullOrEmpty(password))
            {
                Console.WriteLine("Incorrect input was passed");
                ShowUsage();
                return false;
            }

            return true;
        }

        /// <summary>
        /// Shows usage for the sample
        /// </summary>
        private static void ShowUsage()
        {
            Console.WriteLine("Usage: Microsoft.Samples.Management.OData.AssociationClient.exe -UserName <User name> -Password <Password> [-AuthType <Authentication type>] [-Domain <Domain name>] [-Uri <uri>]");
            Console.WriteLine("-UserName: User name");
            Console.WriteLine("-Passowrd: Password");
            Console.WriteLine("-Domain: [Optional] Domain name ");
            Console.WriteLine("-AuthType: [Optional] Authentication type. Default value " + DefaultAuthType);
            Console.WriteLine("-Uri: [Optional] Management OData endpoint service document uri. Default value " + DefaultUri);
        }

        /// <summary>
        /// Creates credentials
        /// </summary>
        /// <param name="uri">Uri of the server</param>
        /// <returns>Credentials cache</returns>
        private static CredentialCache GetCredentials(Uri uri)
        {
            NetworkCredential serviceCreds = null;
            if (string.IsNullOrEmpty(domainName))
            {
                serviceCreds = new NetworkCredential(userName, password);
            }
            else
            {
                serviceCreds = new NetworkCredential(userName, password, domainName);
            }

            CredentialCache cache = new CredentialCache();

            // If server is setup to use some authentication mechanism other than "Basic", change the authentication mechanism below
            cache.Add(uri, authType, serviceCreds);
            
            return cache;
        }
    }

    /// <summary>
    /// Physical machine resource
    /// </summary>
    internal class PhysicalMachineResource
    {
        /// <summary>
        /// Initializes a new instance of the PhysicalMachineResource class
        /// </summary>
        public PhysicalMachineResource()
        {
            this.VMs = new List<VmResource>();
        }

        /// <summary>
        /// Initializes a new instance of the PhysicalMachineResource class
        /// </summary>
        /// <param name="name">Name of the machine</param>
        public PhysicalMachineResource(string name)
            : this()
        {
            this.Name = name;
        }

        /// <summary>
        /// Gets or sets Name of the machine
        /// </summary>
        public string Name { get; set; }

        /// <summary>
        /// Gets list of Virtual machines associated with the machine
        /// </summary>
        public List<VmResource> VMs { get; private set; }
    }

    /// <summary>
    /// Virtual machine resource
    /// </summary>
    internal class VmResource
    {
        /// <summary>
        /// Initializes a new instance of the VmResource class
        /// </summary>
        public VmResource()
        {
        }

        /// <summary>
        /// Initializes a new instance of the VmResource class
        /// </summary>
        /// <param name="physicalMachine">Physical machine name</param>
        /// <param name="os">Operating system</param>
        public VmResource(PhysicalMachineResource physicalMachine, string os)
            : this()
        {
            this.OS = os;
            this.PhysicalMachine = physicalMachine;

            this.MachineName = physicalMachine.Name;
            this.Id = Guid.NewGuid();
        }

        /// <summary>
        /// Gets or sets machine name
        /// </summary>
        public string MachineName { get; set; }

        /// <summary>
        /// Gets or sets virtual machine id
        /// </summary>
        public Guid Id { get; set; }

        /// <summary>
        /// Gets or sets operating system value
        /// </summary>
        public string OS { get; set; }

        /// <summary>
        /// Gets or sets physical machine
        /// </summary>
        public PhysicalMachineResource PhysicalMachine { get; set; }

        /// <summary>
        /// Gets or sets system resource
        /// </summary>
        public SystemResource System { get; set; }
    }

    /// <summary>
    /// System resource
    /// </summary>
    internal class SystemResource
    {
        /// <summary>
        /// Initializes a new instance of the SystemResource class.
        /// </summary>
        public SystemResource()
        {
            this.VMs = new List<VmResource>();
        }

        /// <summary>
        /// Initializes a new instance of the SystemResource class.
        /// </summary>
        /// <param name="name">Name of the system</param>
        public SystemResource(string name)
            : this()
        {
            this.Id = Guid.NewGuid();
            this.Name = name;
        }

        /// <summary>
        /// Gets or sets name of the system
        /// </summary>
        public string Name { get; set; }

        /// <summary>
        /// Gets or sets id of the system
        /// </summary>
        public Guid Id { get; set; }

        /// <summary>
        /// Gets collection of virtual machines associated with the system
        /// </summary>
        public List<VmResource> VMs { get; private set; }
    }
}
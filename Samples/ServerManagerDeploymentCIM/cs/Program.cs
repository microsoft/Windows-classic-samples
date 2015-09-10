
namespace Microsoft.Samples.ServerManagerDeployment.SMDSampleApp
{
    using System;
    using System.Collections.Generic;
    using Microsoft.Management.Infrastructure;

    // ---------------------------------------------------------------------------------------------------
    // The sample class to test the GetRole, AddRole and RemoveRole methods
    // ----------------------------------------------------------------------------------------------------
    public class Program
    {
        private static string vhdPath = "C:\\SMDeployment\\TestVHD\\Win8Server.vhd";
        
        // Tests the GetRole method and displays the list of retrieved server components
        static void TestGetRole()
        {
            GetRoleSample getSample = new GetRoleSample();
            List<CimInstance> getRoleResult = getSample.GetRole();
            //List<CimInstance> getRoleResult = getSample.GetRoleVhd(vhdPath);
            ServerComponentsPrinter.DisplayResult(getRoleResult);
        }

        // Tests the AddRole method and displays the list of components that are installed
        static void TestAddRole()
        {
            AddRoleSample addSample = new AddRoleSample();
            List<string> componentsToAdd = new List<string>();
            componentsToAdd.Add("Print-Services");
            componentsToAdd.Add("Print-Server");
            List<CimInstance> addRoleResult = addSample.AddRole(componentsToAdd);
            if (addRoleResult.Count > 0)
            {
                Console.WriteLine("The following components are installed:");
                ServerComponentsPrinter.DisplayResult(addRoleResult);
            }
        }

        // Tests the RemoveRoleVhd method and displays the list of components that are removed
        static void TestRemoveRoleVhd()
        {
            List<string> componentsToRemove = new List<string>();
            componentsToRemove.Add("Test-Feature");
            RemoveRoleSample removeSample = new RemoveRoleSample();
            List<CimInstance> removeRoleResult = removeSample.RemoveRoleVhd(componentsToRemove, vhdPath);
            if (removeRoleResult.Count > 0)
            {
                Console.WriteLine("The following components are uninstalled:");
                ServerComponentsPrinter.DisplayResult(removeRoleResult);
            }
        }
        
        static void Main(string[] args)
        {
            TestGetRole();
            Console.ReadKey();
        }
    }
}

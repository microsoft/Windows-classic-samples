
Setup: 
    1. Setup Management OData endpoint using RoleBasedPlugin sample.
    2. Copy VMSystem.mof, VMSystem.xml, VMSystem.psm1, HostVMSystem.xml into virtual directory.
    3. Replace web.config in the virtual directory with the web.config file from this sample.
    4. Add a Module element with the path to the VMSystem.psm1 module as it's text nested within the Modules tag for the AdminGroup in the  RbacConfiguration.xml file.
    5. Make sure that the value of the $SystemFileName variable in the VMSystem.psm1 setup script points to the path of the HostVMSystem.xml file on the web server.

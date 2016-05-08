#
# This sample demonstrates how to bind to DS Object
#



use OLE;             # Include OLE and Win32 extensions
use Win32;

# Create the Global Providers object
$adsiNameSpaces = CreateObject OLE 'ADsNameSpaces'
                  or warn "Couldn't create new instance of the ADSI Name Spaces Check your registry for ADsNameSpaces key under classes!!";

$errNum = Win32::OLELastError;
if ($errNum != 0)
{
   print "OLE Error0: ", Win32::FormatMessage($errNum);
   exit(1);
}
else
{
    print "Successfully created the NameSpaces object\n";
}

# Now get the LDAP Provider object 
$ldapNameSpace = $adsiNameSpaces->getobject("", "LDAP:");

$errNum = Win32::OLELastError;
if ($errNum != 0)
{
   print "OLE Error1: ", Win32::FormatMessage($errNum);
   exit(1);
}
else
{
    print "Successfully got the LDAP Provider object\n";
}
#######################
# Let us bind to a DS #
#######################
$myDSObject = $ldapNameSpace->OpenDSObject("LDAP://myServer/DC=MyDomain,DC=Com", "cn=MyUser,cn=users,dc=MyDomain,dc=com", "UserPassword", 1);

$errNum = Win32::OLELastError;
if ($errNum != 0)
{
   print "OLE Error2: ", Win32::FormatMessage($errNum);
   exit(1);
}
else
{
    print "Successfully bound to an object\n";
}


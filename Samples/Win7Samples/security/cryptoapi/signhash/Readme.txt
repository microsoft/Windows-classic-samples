File Signing/Verification sample that hashes the data in a file and signs or 
verifies the signature with the private or public key respectively. 

Usage
---------

The SignHash sample is run from the command line as follows:

SignHash [md5|sha1] [</s>|</v>] <DataFile> <SigFile> [</cert>|</key>]
/s to Sign.
/v to Verify.
/cert <CertName> <StoreName> [<u>|<m>] - use a certificate.
/key <ContainerName> [<u>|<m>] [<x>|<s>] use container with exchange or signature key.


Signing
---------------

/s for Signing

When Signing, the <DataFile> is Hashed and signed with the private key.  
The private key can be a certificate or Crypto key container.  Read the 
/cert or /key options for more information.  After the file is signed, the
signature is saved to <SigFile>.


Verifying
---------------

/v for Verifying

When Verifying, the <DataFile> is Hashed and the <SigFile> is used along with
the public key to verify the signature.  The public key can be obtained from 
a certificate or key container.


Using Certificates
------------------

/cert <CertName> <StoreName> [<u>|<m>]

A certificate can be used to sign or verify a file.  The sample retrieves the
certificate from a certificate store.  To find a certificate to use you must
supply the certificate name, certificate store name and whether you want to 
look in the user's store or the local machine store for the certificate.  The 
most common request is to use the user's personal certificate store to find the
certificate.  In this case the store name is "MY" and the <u> option would be used.

Using Key Containers
--------------------

/key <ContainerName> [<u>|<m>] [<x>|<s>]

A key container can be used to sign or verify a file.  A key container represents
a CryptAPI container that holds the private/public key.  The <u> option selects the
user store and <m> selects the machine store.  <x> selects the exchange key in the
container and <s> selects the signature key.
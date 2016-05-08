File Encryption/Decryption sample using a random session key. The RSA
key associated with the specified certificate is used to encrypt or
decrypt the session key.

Usage
---------

The RSACert sample is run from the command line as follows:

RSACert [</e>|</d>] <CertName> <StoreName> [</u>|</m>] <InputFile> <OutputFile>

Encryption
---------------

/e for Encryption

The <CertName> argument specifies the name of the certificate that is
present in the certificate store named by <StoreName> argument.

A random session key is generated and used to encrypt the data contained
in InputFile. The session key is encrypted using the RSA public key
associated with the specified certificate and the encrypted session key
SIMPLEBLOB is exported and written in the beginning of the output file.

/u for current user certificate store or /m for local machine certificate store

The <InputFile> argument specifies the filename of the plaintext file
to be encrypted, and the <OutputFile> argument specifies the filename of
the ciphertext file to be created.

Decryption
---------------

/d for Decryption

The <CertName> argument specifies the name of the certificate that is
present in the certificate store named by <StoreName> argument. The encrypted
session key SIMPLEBLOB is read from the beginning of the input file and is
then imported. The RSA private key to decrypt this blob is acquired using
CryptAcquireCertificatePrivateKey() API with the certifcate context of the
supplied certificate. CryptImportKey () API will decrypt the SIMPLEBLOB
using the RSA private key and return a handle to session key which is then
used to decrypt the data contained in input file.

/u for current user certificate store or /m for local machine certificate store

The <InputFile> argument specifies the filename of the ciphertext file
to be decrypted, and the <OutputFile> argument specifies the filename of
the destination file to be created.

File Encryption/Decryption sample using a random session key. The RSA
key is used to encrypt or decrypt the session key.

Usage
---------

The RSAKey sample is run from the command line as follows:

RSAKey [</e>|</d>] <KeyContainerName> [</u>|</m>] <InputFile> <OutputFile>

Encryption
---------------

/e for Encryption

The <KeyContainerName> argument specifies the name of the current user
key container that is created or opened. A random session key is generated
and used to encrypt the data contained in InputFile. The session
key is encrypted using the RSA public key and the encrypted session key
SIMPLEBLOB is exported and written in the beginning of the output file.

/u for current user key container or /m for local machine key container

The <InputFile> argument specifies the filename of the plaintext file
to be encrypted, and the <OutputFile> argument specifies the filename of
the ciphertext file to be created.

Decryption
---------------

/d for Decryption

The <KeyContainerName> argument specifies the name of the current user
key container that is created or opened. The encrypted session key
SIMPLEBLOB is read from the beginning of the input file and is then imported.
CryptImportKey will decrypt the SIMPLEBLOB using the RSA private key and
return a handle to session key which is then used to decrypt the data contained
in input file.

/u for current user key container or /m for local machine key container

The <InputFile> argument specifies the filename of the ciphertext file
to be decrypted, and the <OutputFile> argument specifies the filename of
the destination file to be created.

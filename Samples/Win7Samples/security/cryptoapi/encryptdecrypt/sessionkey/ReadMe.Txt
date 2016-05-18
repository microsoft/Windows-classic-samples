File Encryption/Decryption sample using session key derived from a password hash

Usage
---------

The SessionKey sample is run from the command line as follows:

SessionKey <password> [</e>|</d>] <InputFile> <OutputFile>

Encryption
---------------

The <password> argument specifies a password whose hash is used
to derive a session key. The derived session key is used to encrypt
the data contained in InputFile

/e for Encryption

The <InputFile> argument specifies the filename of the plaintext file
to be encrypted, and the <OutputFile> argument specifies the filename of
the ciphertext file to be created.

Decryption
---------------

The <password> argument specifies a password whose hash is used
to derive a session key. The derived session key is used to decrypt
the data contained in InputFile

/d for Decryption

The <InputFile> argument specifies the filename of the ciphertext file
to be decrypted, and the <OutputFile> argument specifies the filename of
the destination file to be created.

If an incorrect password is supplied during decrypt, the file will not be
decrypted correctly.

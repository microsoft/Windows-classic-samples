File Encryption


The ENCRYPT sample is a console application that encrypts files. Files
encrypted with this sample can be later decrypted with the DECRYPT sample.

Note that the INITUSER sample (or equivalent) must be run prior to running
these samples, to create a key container for the default user.

Usage
-----

The ENCRYPT sample is run from the command line as follows:

    encrypt <source file> <dest file> [ <password> ]

The <source file> argument specifies the filename of the plaintext file
to be encrypted, and the <dest file> argument specifies the filename of
the ciphertext file to be created. The optional <password> argument specifies
a password with which to encrypt the file.

If no password is specified, then a random session key is used to encrypt
the file. This session key is then encrypted with the key exchange public
key of the default user and stored with the encrypted file. In this case,
the corresponding key exchange private key is later used (by DECRYPT) to
decrypt the session key, which is used in turn to decrypt the file itself.

The DECRYPT sample is run from the command line as follows:

    decrypt <source file> <dest file> [ <password> ]

The <source file> argument specifies the filename of the ciphertext file
to be decrypted, and the <dest file> argument specifies the filename of
the plaintext file to be created. The optional <password> argument specifies
a password with which to decrypt the file.

If a bogus password is supplied to DECRYPT, no error is typically generated.
Of course, the file isn't decrypted properly either.

Exercises for the Reader
------------------------

1. By default, these samples use the RC4 stream cipher to perform the
   encryption and decryption operations. If the USE_BLOCK_CIPHER constant is
   defined at the top of each file, however, the RC2 block cipher is used
   instead.

2. For the sake of simplicity, these samples do not use salt values or (in the
   case of a block cipher) initialization vectors (IVs). This greatly
   diminishes their effective security. It would be a small matter to modify
   these programs such that salt values and IVs are used (see CryptSetKeyParam
   in the documentation). These values should be generated using the
   CryptGenRandom function, and need to be stored (in a non-encrypted form)
   along with the ciphertext file.

3. Another command line argument could be added, which would specify a file
   containing the public key to use when encrypting the session key. This file
   would ideally be in the form of a certificate. If this is done, then it
   becomes possible to encrypt a file such that only the owner of the
   corresponding private key would be able to decrypt the file. This is useful
   if you want to send the file to someone else. Note that this is only
   applicable in the case when a password is not specified.

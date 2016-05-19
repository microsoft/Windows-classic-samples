Enumerate Algorithms


The ENUMALGS sample is a console application that lists the algorithms
supported by the default PROV_RSA_FULL provider. By default, this will
be the Microsoft RSA Base Provider, which is included along with the
operating system.

Note that the INITUSER sample (or equivalent) must be run prior to running
these samples, in order to create a key container for the default user.

In addition to listing the name of each supported algorithm, this sample
also lists:

  - The type of algorithm (encryption, hash, key exchange, or signature).

  - The key length used by the algorithm (or number of bits in the
    hash value for hash algorithms).

  - The algorithm identifier for the algorithm. This value can be passed
    to the appropriate CryptoAPI function in order to create a key or
    hash object that makes use of the particular algorithm.

The guts of this sample can be used by applications that want to display
a list of available applications to the user.

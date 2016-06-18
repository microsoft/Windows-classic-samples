OfflinePublishing

This sample demonstrates offline publishing.  It acquires the appropriate licenses and certificates, initializes the environment, and signs the publishing license offline.  See the comments at the beginning of wmain() for a more detailed description.

The usage of this sample is as follows:

Usage:
  OfflinePublishing -U UserID -M ManifestFile [-A ActivationSvr] [-L LicensingSvr]
    -U: specifies the UserID.
        example: user@yourdomain.com
    -M: specifies the manifest file to use.
        example: manifest.xml
    -A: specifies the Activation Server. (optional)
        example: http://localhost/_wmcs/certification
    -L: specifies the Licensing Server. (optional)
        example: http://localhost/_wmcs/licensing

Note: The userID specified should be the user's primary SMTP proxy address.

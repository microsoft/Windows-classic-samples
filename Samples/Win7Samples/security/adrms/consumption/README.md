---
page_type: sample
languages:
- cpp
products:
- windows-api-win32
name: Consumption sample
urlFragment: consumption-sample
description: Demonstrates consumption. 
extendedZipContent:
- path: LICENSE
  target: LICENSE
---

# Consumption

This sample demonstrates consumption.  It acquires the appropriate licenses and certificates, initializes the environment, signs the publishing license offline, encrypts content, and then decrypts it.  See the comments at the beginning of wmain() for a more detailed description.

## Usage

The usage of this sample is as follows:

     Consumption -U UserID -M ManifestFile [-A ActivationSvr] [-L LicensingSvr]
        -U: specifies the UserID.
          example: user@yourdomain.com
        -M: specifies the manifest file to use.
          example: manifest.xml
        -A: specifies the Activation Server. (optional)
          example: http://localhost/_wmcs/certification
        -L: specifies the Licensing Server. (optional)
          example: http://localhost/_wmcs/licensing

Note: The userID specified should be the user's primary SMTP proxy address.
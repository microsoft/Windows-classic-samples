##
##  <copyright file="readme.txt" company="Microsoft">
##    Copyright (C) Microsoft. All rights reserved.
##  </copyright>
##

Copy Sample.proplink to %programfiles%\Windows Server\Bin\Addins\Users

How to check that the addin is working:

1) Open the Dashboard and select the "Users" tab

2) Select one of the users in the list (ie, Guest)

3) Click on the link "View Account Properties"

4) There will be 3 tabs on the "Properties for <user name>" dialog: General, Shared Folders, and Addi-ins. Without this proplink addin, there would be only the first 2 tabs.

5) Select the Addins tab. There will be 2 buttons on the tab corresponding to the 2 tasks defined in the proplink XML
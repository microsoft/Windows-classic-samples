Using the Win32 API to Access the Registry


SUMMARY
=======

The REGISTRY sample is a simple utility which demonstrates the functions 
needed to access the registry. The sample goes up and down the branches of 
the registry tree, displaying the individual key's data values.

MORE INFORMATION
================

Usage
-----

Start the sample. A dialog box appears with several edit fields, list boxes,
and buttons. The list box at the center of the dialog box (labeled CHILD 
KEYS at the bottom) always holds the child keys of the current key. 
Initially, it has four entries, representing the predefined key handles of 
the registry: 

    HKEY_LOCAL_MACHINE 
    HKEY_CURRENT_USER 
    HKEY_USERS
    HKEY_CLASSES_ROOT

If you double-click on any of these entries, or highlight one and press the 
Next/Down button; the key that you selected appears in the Key Name edit 
field, and the children of that selected key replace the entries in the 
CHILD KEYS list box. To proceed deeper into the tree, double-click another 
child. To back out of the registry, double-click on the ".." at the top of 
the list box, or press the Back/Up button.

If the current key has values associated with it, the name of the values is 
listed in the right list box (labeled VALUES). If it has no values, VALUES 
is followed by a "0". Once you come to a key that does have values 
associated with it, you can double-click on any of the values in this list 
box. At the bottom of the dialog box are two edit fields: Value: Data Type 
and Value: Data Entry. By double-clicking a value entry, these edit fields 
are filled with the data's type and the data's value. 

If the current key has a class type associated with it, it appears in the 
Class edit field. The ACL edit field is not implemented at this time.

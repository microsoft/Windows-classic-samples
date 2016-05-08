// Used by EdgeTraversalOptions.rc and main.cpp

// Resource identifiers for firewall rule
#define RULE_NAME_STRING_ID             101
#define RULE_GROUP_STRING_ID            102
#define RULE_DESCRIPTION_STRING_ID      103

// For multiple firewall rules belonging to the same rule group:
// Following resource must be located at 10000 higher than the rule group resource identifier. 
// This resource string will get displayed as the description text for the entire rule group in the Windows Firewall control panel.
#define GROUP_DESCRIPTION_STRING_ID     RULE_GROUP_STRING_ID+10000



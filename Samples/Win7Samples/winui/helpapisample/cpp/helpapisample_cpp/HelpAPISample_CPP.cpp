#include "stdafx.h"
#include "stdio.h"
#include <iostream>

#import "C:\Program Files\Microsoft SDKs\Windows\v1.0\Lib\hxhelppaneproxy.tlb" named_guids no_namespace exclude("GUID") exclude("IUnknown")
char strSrch[128];
char strTopicDisp[128];
char strTopicToc[128];

int _tmain(int argc, _TCHAR* argv[])
{
	std::cout << "Please enter a value (1-4)\n";
    std::cout << " 1: DisplaySearchResults\n";
    std::cout << " 2: DisplayTask\n";
    std::cout << " 3: DisplayContents (TOC root)\n";
    std::cout << " 4: DisplayContents (specific task)\n";
    std::cout << ">";

    std::cin >> argc;

	CoInitialize(NULL);
    IHxHelpPane* pHelpPane = NULL;

    HRESULT hr = ::CoCreateInstance(CLSID_HxHelpPane, NULL, CLSCTX_ALL, IID_IHxHelpPane, reinterpret_cast<void**>(&pHelpPane));
    if(FAILED(hr))
    {
        std::cout << "Can't create HelpPaneProxy object. HR=0x%X\n", hr;
        return -1;
    }

	if (argc == 1)
		try
	        {
                // (1) Function: Display search results
                // Parameter: any word or words that exist in registered help contents
				std::cout << "Please enter a search keyword: ";
				std:: cin >> strSrch;
				hr = pHelpPane->DisplaySearchResults(strSrch);
	        }
		catch(_com_error &err)
			{
				std::cout << "COM Error Code = " << err.Error();
				std::cout << "COM Error Desc = " << err.ErrorMessage() << "\n";
			}

	else if (argc == 2)
		try
	        {
                // (2) Function: Display a registered topic under Windows namespace
                // Parameter: url with valid help protocol and registered topic id
                // such as: mshelp://Windows/?id=004630d0-9241-4842-9d3f-2a0c5825ef14
				std::cout << "Please enter a topic ID: ";
				std:: cin >> strTopicDisp;
				hr = pHelpPane->DisplayTask(strTopicDisp);
	        }
		catch(_com_error &err)
			{
				std::cout << "COM Error Code = " << err.Error();
				std::cout << "COM Error Desc = " << err.ErrorMessage() << "\n";
				std::cout << "Please enter a valid URI.";
			}

	else if (argc == 3)
		try
	        {
                // (3) Function: Display the root TOC (Table of content)
                // Parameter: NULL or empty string
				std::cout << "Displaying the TOC root.";
                hr = pHelpPane->DisplayContents("");
	        }
		catch(_com_error &err)
			{
				std::cout << "COM Error Code = " << err.Error();
				std::cout << "COM Error Desc = " << err.ErrorMessage() << "\n";
			}

	else if (argc == 4)
		try
	        {
                // (4) Function: Display a TOC (Table of content) page
                // Parameter: url with valid help protocol and authoried toc id
                // such as mshelp://Windows/?id=004630d0-9241-4842-9d3f-2a0c5825ef14
				std::cout << "Please enter a toc ID: ";
				std:: cin >> strTopicToc;
				hr = pHelpPane->DisplayContents(strTopicToc);
	        }
		catch(_com_error &err)
			{
				std::cout << "COM Error Code = " << err.Error();
				std::cout << "COM Error Desc = " << err.ErrorMessage() << "\n";
				std::cout << "Please enter a valid URI.";
			}

	else
		    {
				std::cout << "Please enter a valid value (1-4).";
				return -1;
            }

    CoUninitialize();


	return 0;
}
